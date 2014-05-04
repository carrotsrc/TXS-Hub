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
#include "common.h"
#include "client.h"
#include "hubcomm.h"
#include "parser.h"
void dispatch_artists(client_t *cli, gchar *buf, int len);
alist_t *allocate_artistList(int num);
void populate_artistList(client_t *cli, alist_t *list);

void dispatch_lineup(client_t *cli, gchar *buf, int len);
void populate_lineupList(client_t *cli, alist_t *list);

void process_client(GIOChannel *channel, client_t *cli, hph_t *hdr, int len)
{
	GError *e = NULL;
	GIOStatus r;
	switch(hdr->code) {
	case HUB_HAIL:
		// we need to register this as the CLI interface
		cli->state = CLI_HAIL;
		gsize bwr = 0;
		r = g_io_channel_write_chars(channel, "rcli\n\0",6 , &bwr, &e);
		g_io_channel_flush(channel, NULL);
	break;

	case HUB_OK:
		if(cli->state == CLI_HAIL) {
			// we have hailed successfully
			printf("Hailed successfully\n");
			cli->state = CLI_READY;
			//r = g_io_channel_write_chars(channel, "db get\n\0",8 , &bwr, &e);
			//g_io_channel_flush(channel, NULL);
		}
	break;

	case HUB_ERROR:
		if(cli->state == CLI_HAIL) {
			printf("Hail unsuccessful\n");
		}
	break;

	case HUB_DISPATCH: // hub has payload waiting for dispatch
		if(cli->state == CLI_READY) {
			cli->state = CLI_WAITING_DISPATCH;
			cli->pl_size = hdr->pl_size;
			cli->pl_nelements = hdr->nelements;

			// tell hub we are ready to receive the dispatch
			r = g_io_channel_write_chars(channel, "ok\n\0",4 , &bwr, &e);
			g_io_channel_flush(channel, NULL); // flush to send

		}
	break;

	case HUB_NOT_FOUND:
	break;
	}
}

// process the dispatch from the server
void process_dispatch(GIOChannel *channel, client_t *cli, gchar *buf, int len)
{
	// process for type if payload
	if(cli->pl_type == PL_ARTISTS)
		dispatch_artists(cli, buf, len);
	else
	if(cli->pl_type == PL_LINEUP)
		dispatch_lineup(cli, buf, len);

	cli->state = CLI_READY; // reset the client to ready state
	cli->pl_size = 0;
	cli->pl_type = 0;
	cli->pl_nelements = 0;
}

/*
 * Process an artist dispatch
 */
void dispatch_artists(client_t *cli, gchar *buf, int len)
{
	parser_t *parser = parser_init(buf, len);
	alist_t *list = allocate_artistList(cli->pl_nelements);
	element_t *e;

	int cel = 0;

	/* parse the xml in the payload
	 * the form of the xml is
	 * <row><col id="id">%id%</col><col id="name">%artist%</col></row>
	 */
	while(parser->pstate == PARSER_RUNNING) {
		e = parser_nextElement(parser);
		if(strcmp(e->name, "/row") == 0)
			cel++;
		else
		if(strcmp(e->name, "col") == 0) {
			if(strcmp(e->attributes[0].value, "id") == 0) {
				free(e);
				e = parser_nextElement(parser); // we need the #text element for the value
				if(strcmp(e->name, "#text") == 0)
					list->artists[cel].id = atoi(e->attributes[0].value);
			}
			else
			if(strcmp(e->attributes[0].value, "name") == 0) {
				free(e);
				e = parser_nextElement(parser); // we need the #text element for the value
				if(strcmp(e->name, "#text") == 0) {
					list->artists[cel].name = malloc(sizeof(char)*e->attributes[0].vlen);
					strcpy(list->artists[cel].name, e->attributes[0].value);
				}
			}
		}
		free(e);
		e = NULL;
	}
	populate_artistList(cli, list);
}
/*
 * Process an lineup dispatch
 */
void dispatch_lineup(client_t *cli, gchar *buf, int len)
{
	parser_t *parser = parser_init(buf, len);
	element_t *e = NULL;

	/* allocate a list based on the number of elements
	 * specified in the original dispatch header
	 * which was assigned to the client descriptor
	 */
	alist_t *list = allocate_artistList(cli->pl_nelements);

	int cel = 0;

	/* parse the xml in the payload
	 * the form of the xml is
	 * <row><col id="id">%id%</col><col id="name">%DJ%</col></row>
	 */
	while(parser->pstate == PARSER_RUNNING) {
		e = parser_nextElement(parser); // get the next xml element
		if(strcmp(e->name, "/row") == 0)
			cel++; // next list element
		else
		if(strcmp(e->name, "col") == 0) {
			if(strcmp(e->attributes[0].value, "id") == 0) {
				free(e);
				e = parser_nextElement(parser); // we need to get the #text element
				if(strcmp(e->name, "#text") == 0)
					list->artists[cel].id = atoi(e->attributes[0].value); // set the current list item id
			}
			else
			if(strcmp(e->attributes[0].value, "name") == 0) {
				free(e);
				e = parser_nextElement(parser); // we need to get the #text element
				if(strcmp(e->name, "#text") == 0) {
					list->artists[cel].name = malloc(sizeof(char)*e->attributes[0].vlen);
					strcpy(list->artists[cel].name, e->attributes[0].value); // set the current list item name
				}
			}
		}
		free(e);
		e = NULL;
	}


	populate_lineupList(cli, list);
}

// take the number of artists and allocate enough space in the list
alist_t *allocate_artistList(int num)
{
	alist_t *list = malloc(sizeof(alist_t));
	list->nelements = num;
	list->artists = malloc(sizeof(arow_t)*list->nelements);
}


// TODO: Consolidate these two functions

// populate the artist list store with the artists
void populate_artistList(client_t *cli, alist_t *list)
{
	GtkListStore *str = GTK_LIST_STORE(cli->artist_list);
	gtk_list_store_clear(str);

	GtkTreeIter it;
	for(int i = 0; i < list->nelements; i++) {
		gtk_list_store_append(str, &it);
		char *id = malloc(sizeof(char)<<2);
		sprintf(id, "%d", list->artists[i].id);

		gtk_list_store_set(str, &it, 0, id, -1); // set the first column (should this be -1?)
		gtk_list_store_set(str, &it, 1, list->artists[i].name, -1); // set the second column
	}
	free(list);
	list = NULL;
}

// populate the line up list store with the DJs
void populate_lineupList(client_t *cli, alist_t *list)
{
	GtkListStore *str = GTK_LIST_STORE(cli->lineup_list);
	gtk_list_store_clear(str);

	GtkTreeIter it;
	for(int i = 0; i < list->nelements; i++) {
		gtk_list_store_append(str, &it);
		char *id = malloc(sizeof(char)<<2);
		sprintf(id, "%d", list->artists[i].id);
		gtk_list_store_set(str, &it, 0, id, -1);
		gtk_list_store_set(str, &it, 1, list->artists[i].name, -1);
	}

	free(list);
	list = NULL;
}
