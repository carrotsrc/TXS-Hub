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

hubdesc_t *init_hub()
{
	hubdesc_t *h = malloc(sizeof(hubdesc_t));
	h->head = h->cli = WIRE_UNDEF;
	h->state = STATE_RUN;
	h->flags = 0;
	int *dbp = db_connect();
	if(dbp == NULL)
		return NULL;

	h->dbp = dbp;
	return h;
}

void free_hub(hubdesc_t *hub)
{
	db_close(hub->dbp);
	free_null(hub);
}

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

int process_hub(int sfd, hubdesc_t *hub)
{
	char *out = malloc(512);
	size_t sz = 512;

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
		hub_parse(hub, sfd, out);

	free_null(out);
	return  0;
}

void hub_state(hubdesc_t *hub, short state)
{
	hub->state = state;
}

void hub_regclient(hubdesc_t *hub, int fd, short type)
{
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

void hub_parse(hubdesc_t *hub, int sfd, char *str)
{
	short i = hub_strpos(str, ' ');
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

void parse_cmdDb(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("add", var) == 0) {
		parse_cmdDbArt(hub, (char*)var+(i+1));
	}
	else
	if(strcmp("get", var) == 0) {
		if(i > 0)
			parse_cmdDbGet(hub, (char*)var+(i+1));
		else
			parse_cmdDbGet(hub, "");
	}
	if(strcmp("lineup", var) == 0) {
		parse_cmdDbLineup(hub, (char*)var);
	}
}

void parse_cmdDbArt(hubdesc_t *hub, char *artist)
{
	char *sql = malloc(sizeof(char)<<7);
	
	sprintf(sql, "INSERT INTO `artist_list` (`name`) VALUE ('%s')", artist);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	r = NULL;
	mysql_free_result(r);
	free_null(sql);
}

void parse_cmdDbGet(hubdesc_t *hub, char *srch)
{
	char *sql = malloc(sizeof(char)<<8);
	unsigned long *len, size;
	unsigned int nf, i, nr;
	sprintf(sql, "SELECT DISTINCT `id`, `name` FROM `artist_list` WHERE `name` LIKE '%s%%'", srch);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	if(r == NULL) {
		free_null(sql);
		write(hub->cli, HEADER_ERROR(), HDR_SIZE); 
		return;
	}

	nr = mysql_num_rows(r);

	if(nr == 0) {
		free_null(sql);
		mysql_free_result(r);
		r = NULL;
		write(hub->cli, HEADER_NOTFOUND(), HDR_SIZE); 
		return;
	}

	hdispatch_t dispatch;

	char *xml = db_sqltoxml(r, (int*)&size);
	dispatch.payload = xml;
	dispatch.fd = hub->cli;
	dispatch.size = size;
	hub->dispatch = &dispatch;
	write(hub->cli, HEADER_DISPATCH(size, nr), sizeof(hph_t));

	if(r != NULL) {
		mysql_free_result(r);
		r = NULL;
	}

	free_null(sql);
}

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
	if(nr == 0) {
		write(hub->cli, HEADER_NOTFOUND(), HDR_SIZE); 
		mysql_free_result(r);
		r = NULL;
		return;
	}
	hdispatch_t dispatch;

	char *xml = db_sqltoxml(r, (int*)&size);
	dispatch.payload = xml;
	dispatch.fd = hub->cli;
	dispatch.size = size;
	hub->dispatch = &dispatch;
	write(hub->cli, HEADER_DISPATCH(size, nr), sizeof(hph_t));


	mysql_free_result(r);
	r = NULL;
}

void parse_cmdMessage(hubdesc_t *hub, char *var)
{
	char *sql = malloc(sizeof(char)<<7);

	sprintf(sql, "INSERT INTO `head_msg` (`msg`) VALUE ('%s')", var);
	MYSQL_RES *r = db_query(hub->dbp, sql);

	mysql_free_result(r);

	if(hub->head != WIRE_UNDEF)
		write(hub->head, HEADER_REFRESH(), HDR_SIZE); 

}

void parse_cmdPlaylist(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("add", var) == 0) {
		parse_cmdPlaylistAdd(hub, (char*)var+(i+1));
	}
}

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

void parse_cmdLineup(hubdesc_t *hub, char *var)
{
	short i = hub_strpos(var, ' ');
	if(i > 0)
		var[i] = '\0';

	if(strcmp("current", var) == 0) {
		parse_cmdLineupCurrent(hub, (char*)var+(i+1));
	}
}

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

void parse_cmdLineupNone(hubdesc_t *hub, char *var)
{
	char *sql = malloc(sizeof(char)<<7);

	sprintf(sql, "UPDATE `lineup` SET `playing`='0' WHERE `playing`='1';");
	MYSQL_RES *r = db_query(hub->dbp, sql);
	mysql_free_result(r);
	r = NULL;
	free_null(sql);
}

void hub_parseOk(hubdesc_t *hub, int sock)
{
	if(hub->state = STATE_WAITING_DISPATCH) {
		if(sock == hub->dispatch->fd) {
			write(hub->cli, hub->dispatch->payload, hub->dispatch->size);
			hub->state = STATE_READY;
			hub->dispatch = NULL;
		}
	}
}

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
