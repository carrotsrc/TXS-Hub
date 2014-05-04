/*
* Copyright 2014, carrotsrc.org
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "hub.h"
#include "db.h"
#include "common.h"

/*
 * this is where all the server processing occurs
 *
 * the MACROS() for server response are defined in
 * shared/hubbcomm.h
 */


void hub_state(hubdesc_t *hub, short state);
void hub_regclient(hubdesc_t *hub, int fd, short type);
void hub_parse(hubdesc_t *hub, int sfd, char *str);
void hub_parseOk(hubdesc_t *hub, int sock);

void parse_cmdDb(hubdesc_t *hub, char *var);
void parse_cmdDbArt(hubdesc_t *hub, char *artist);
void parse_cmdDbGet(hubdesc_t *hub, char *srch);
void parse_cmdDbLineup(hubdesc_t *hub, char *var);

void parse_cmdPlaylist(hubdesc_t *hub, char *var);
void parse_cmdPlaylistAdd(hubdesc_t *hub, char *id);

void parse_cmdHead(hubdesc_t *hub, char *var);

void parse_cmdLineup(hubdesc_t *hub, char *var);
void parse_cmdLineupCurrent(hubdesc_t *hub, char *id);

void parse_cmdMessage(hubdesc_t *hub, char *var);

void hub_refreshVis(hubdesc_t *hub, char *id);


/* setup a hub descriptor and set it to running
 * set all wire file descriptors to undefined
 */
hubdesc_t *init_hub()
{
	hubdesc_t *h = malloc(sizeof(hubdesc_t));
	h->vis = h->head = h->cli = WIRE_UNDEF;
	h->state = STATE_RUN;
	h->flags = 0;
	int *dbp = db_connect();
	if(dbp == NULL)
		return NULL;

	h->dbp = dbp;
	return h;
}

// free up a hub descriptor
void free_hub(hubdesc_t *hub)
{
	db_close(hub->dbp);
	free_null(hub);
}

// find a character
int hub_strpos(char *str, char c)
{
	short i = 0;
	char ch;
	while((ch = str[i]) != '\0') {
		if(ch == c)
			return i;

		i++;
	}

	return -1;
}

// process the data from the socket
int process_hub(int sfd, hubdesc_t *hub)
{
	char *out = malloc(512);
	size_t sz = 512;

	// read data from the socket to the buffer (512 bytes)
	int r = read(sfd, out, sz);
	if(r == -1) {
		free_null(out);
		return -1;
	}

	if(hub->flags&HFLAG_VERBOSE > 0) {
		printf("read %d bytes from %d\n", r, sfd);
		printf("OUT: %s\n", out);
	}

	// hub commands
	out[r-2] = '\0';
	if(strcmp(out, "ok") == 0)
		hub_parseOk(hub, sfd);
	else
	if(strcmp(out, "kill") == 0) // kill the connection
		hub_state(hub, STATE_KILL);
	else
	if(strcmp(out, "rcli") == 0) // register a cli
		hub_regclient(hub, sfd, WIRE_CLI);
	else
	if(strcmp(out, "rhead") == 0) // register a head
		hub_regclient(hub, sfd, WIRE_HEAD);
	else
	if(strcmp(out, "rvis") == 0) // register vis
	      hub_regclient(hub, sfd, WIRE_VIS);
	else
		hub_parse(hub, sfd, out); // not a single word command

	free_null(out);
	return  0;
}

// set the hub state
void hub_state(hubdesc_t *hub, short state)
{
	hub->state = state;
}

/* register a client descriptor as a wire descriptor on the hub
 *
 * Different clients provide different functions:-
 * 	- the head is the graphical heads up of the playlist and messages
 * 	- the cli is the client for controlling different aspects
 * 	- the vis is the visuals rig which mixes in artist names
 */
void hub_regclient(hubdesc_t *hub, int fd, short type)
{
	// register the head client
	if(type == WIRE_HEAD && hub->head == WIRE_UNDEF) {
		write(fd, HEADER_OK(), sizeof(hph_t));

		if(hub->flags&HFLAG_VERBOSE > 0)
			printf("Descriptor %d ", fd);
		printf("HEAD hailed\n");
		hub->head = fd;
	}
	else
	if(type == WIRE_CLI && hub->cli == WIRE_UNDEF) {

		write(fd, HEADER_OK(), HDR_SIZE);

		if(hub->flags&HFLAG_VERBOSE > 0)
			printf("Descriptor %d ", fd);
		printf("CLI hailed\n");
		hub->cli = fd;
	}
	else
	if(type == WIRE_VIS && hub->vis == WIRE_UNDEF) {
		write(fd, HEADER_OK(), HDR_SIZE);

		if(hub->flags&HFLAG_VERBOSE > 0)
			printf("Descriptor %d ", fd);
		printf("VIS hailed\n");
		hub->vis = fd;
	}
	else
		write(fd, HEADER_ERROR(), HDR_SIZE);
}

// initial branch for command processing
void hub_parse(hubdesc_t *hub, int sfd, char *str)
{
	short i = hub_strpos(str, ' '); // find the space
	str[i] = '\0';
	if(strcmp("db", str) == 0 && hub->cli == sfd) {
		parse_cmdDb(hub, (char*)str+(i+1));
	}
	else
	if(strcmp("playlist", str) == 0 && hub->cli == sfd) {
		parse_cmdPlaylist(hub, (char*)str+(i+1));
	}
	else
	if(strcmp("lineup", str) == 0 && hub->cli == sfd) {
		parse_cmdLineup(hub, (char*)str+(i+1));
	}
	else
	if(strcmp("message", str) == 0 && hub->cli == sfd) {
		parse_cmdMessage(hub, (char*)str+(i+1));
	}

}

// the db branch for command processing
void parse_cmdDb(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("add", var) == 0) { // add an artist to the database
		parse_cmdDbArt(hub, (char*)var+(i+1));
	}
	else
	if(strcmp("get", var) == 0) { // search for artists
		if(i > 0)
			parse_cmdDbGet(hub, (char*)var+(i+1)); // search term provided
		else
			parse_cmdDbGet(hub, ""); // no search term
	}
	if(strcmp("lineup", var) == 0) { // get the line up
		parse_cmdDbLineup(hub, (char*)var);
	}
}

// add an artist to the database
void parse_cmdDbArt(hubdesc_t *hub, char *artist)
{
	char *sql = malloc(sizeof(char)<<7);
	
	sprintf(sql, "INSERT INTO `artist_list` (`name`) VALUE ('%s')", artist);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	r = NULL;
	mysql_free_result(r);
	free_null(sql);
}


/* run a search for an artist name.
 * the search term can be blank to search for
 * all the artists in the database.
 */
void parse_cmdDbGet(hubdesc_t *hub, char *srch)
{
	char *sql = malloc(sizeof(char)<<8); // 128 bytes should be enough space
	unsigned long *len, size;
	unsigned int nf, i, nr;
	sprintf(sql, "SELECT DISTINCT `id`, `name` FROM `artist_list` WHERE `name` LIKE '%s%%'", srch);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	if(r == NULL) {
		free_null(sql);
		write(hub->cli, HEADER_ERROR(), HDR_SIZE); 
		return;
	}

	// get the number of rows
	nr = mysql_num_rows(r);

	if(nr == 0) { // there are no results
		free_null(sql);
		mysql_free_result(r);
		r = NULL;
		write(hub->cli, HEADER_NOTFOUND(), HDR_SIZE); 
		return;
	}

	// here we setup the dispatch payload for the client
	hdispatch_t dispatch;

	char *xml = db_sqltoxml(r, (int*)&size);
	dispatch.payload = xml; // the xml data
	dispatch.fd = hub->cli; // the file descriptor to send dispatch
	dispatch.size = size; // the size of the dispatch
	hub->dispatch = &dispatch; // set it on the hub descriptor

	// tell the client there is data waiting for dispatch
	write(hub->cli, HEADER_DISPATCH(size, nr), sizeof(hph_t));

	if(r != NULL) {
		mysql_free_result(r);
		r = NULL;
	}

	free_null(sql);
}

// get the current lineup from the database
void parse_cmdDbLineup(hubdesc_t *hub, char *var)
{
	char *sql = malloc(sizeof(char)<<7);
	unsigned long *len, size;
	unsigned int nf, i, nr;
	
	sql = "SELECT DISTINCT `id`, `name` FROM `lineup`";
	db_query(hub->dbp, sql);
	MYSQL_RES *r = (MYSQL_RES*)db_query(hub->dbp, sql);
	if(r == NULL) {
		write(hub->cli, HEADER_ERROR(), HDR_SIZE); 
		return;
	}

	nr = mysql_num_rows(r);
	if(nr == 0) { // there is no line up
		write(hub->cli, HEADER_NOTFOUND(), HDR_SIZE); 
		mysql_free_result(r);
		r = NULL;
		return;
	}

	// setup the dispatch payload
	hdispatch_t dispatch;

	char *xml = db_sqltoxml(r, (int*)&size);
	dispatch.payload = xml; // set the xml data in the dispatch
	dispatch.fd = hub->cli; // set the file descriptor
	dispatch.size = size; // set the size of the payload
	hub->dispatch = &dispatch; // set the dispatch in the hub descriptor

	// tell the client there is data waiting for dispatch
	write(hub->cli, HEADER_DISPATCH(size, nr), sizeof(hph_t));


	mysql_free_result(r);
	r = NULL;
}


// add a message to display on the head message area
void parse_cmdMessage(hubdesc_t *hub, char *var)
{
	char *sql = malloc(sizeof(char)<<7); // 64 byte should be adequate

	sprintf(sql, "INSERT INTO `head_msg` (`msg`) VALUE ('%s')", var);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	mysql_free_result(r);

	if(hub->head != WIRE_UNDEF)
		write(hub->head, HEADER_REFRESH(), HDR_SIZE); 

}


// branching for the playlist command 
void parse_cmdPlaylist(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("add", var) == 0) {
		parse_cmdPlaylistAdd(hub, (char*)var+(i+1));
	}
}

/* add an artist to the current playlist and
 * increment the artist's playcount
 */
void parse_cmdPlaylistAdd(hubdesc_t *hub, char *id)
{
	char *sql = malloc(sizeof(char)<<7);

	sprintf(sql, "INSERT INTO `playlist` (`artist_id`) VALUE ('%s')", id);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	mysql_free_result(r);

	sprintf(sql, "UPDATE `artist_list` SET count=count+1 WHERE `id`='%s';", id);
	r = db_query(hub->dbp, sql);

	mysql_free_result(r);
	r = NULL;
	free_null(sql);

	if(hub->head != WIRE_UNDEF)
		write(hub->head, HEADER_REFRESH(), HDR_SIZE); 

	if(hub->vis != WIRE_UNDEF)
		hub_refreshVis(hub, id); 
}

// branching for the lineup command
void parse_cmdLineup(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("current", var) == 0) {
		parse_cmdLineupCurrent(hub, (char*)var+(i+1));
	}
}


// set the currently playing DJ
void parse_cmdLineupCurrent(hubdesc_t *hub, char *id)
{
	char *sql = malloc(sizeof(char)<<7);

	sprintf(sql, "UPDATE `lineup` SET `playing`='0' WHERE `playing`='1';");
	MYSQL_RES *r = db_query(hub->dbp, sql);

	sprintf(sql, "UPDATE `lineup` SET `playing`='1' WHERE `id`='%s';", id);
	r = db_query(hub->dbp, sql);

	mysql_free_result(r);
	r = NULL;
	free_null(sql);

	if(hub->head != WIRE_UNDEF)
		write(hub->head, HEADER_REFRESH(), HDR_SIZE); 
}

// set no one playing
void parse_cmdLineupNone(hubdesc_t *hub, char *var)
{
	char *sql = malloc(sizeof(char)<<7);

	sprintf(sql, "UPDATE `lineup` SET `playing`='0' WHERE `playing`='1';");
	MYSQL_RES *r = db_query(hub->dbp, sql);
	mysql_free_result(r);
	r = NULL;
	free_null(sql);
}

// accept request for waiting payload and dispatch it to the client
void hub_parseOk(hubdesc_t *hub, int sock)
{
	if(hub->state = STATE_WAITING_DISPATCH) {
		if(sock == hub->dispatch->fd && hub->dispatch != NULL) {
			write(hub->cli, hub->dispatch->payload, hub->dispatch->size);
			hub->state = STATE_READY;
			free_null(hub->dispatch->payload);
			hub->dispatch->size = 0;
			hub->dispatch = NULL;
		}
	}
}

// send the visuals rig the currently playing artist in XML format
void hub_refreshVis(hubdesc_t *hub, char *id)
{
	char *sql = malloc(sizeof(char)<<7);
	unsigned long *len, size;
	unsigned int nf, i, nr;
	
	sprintf(sql, "SELECT `name` FROM `artist_list` WHERE `id`='%s' ORDER BY `id` DESC LIMIT 1;", id);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	nr = mysql_num_rows(r);
	if(nr == 0) {
		write(hub->cli, HEADER_NOTFOUND(), HDR_SIZE); 
		return;
	}

	char *xml = db_sqltoxml(r, (int*)&size);
	write(hub->vis, xml, size);
	r = NULL;
	mysql_free_result(r);
	free_null(sql);
}
