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

	case HUB_DISPATCH:
		if(cli->state == CLI_READY) {
			cli->state = CLI_WAITING_DISPATCH;
			cli->pl_size = hdr->pl_size;
			cli->pl_nelements = hdr->nelements;

			r = g_io_channel_write_chars(channel, "ok\n\0",4 , &bwr, &e);
			g_io_channel_flush(channel, NULL);

		}
	break;

	case HUB_NOT_FOUND:
	break;
	}
}

void process_dispatch(GIOChannel *channel, client_t *cli, gchar *buf, int len)
{
	if(cli->pl_type == PL_ARTISTS)
		dispatch_artists(cli, buf, len);
	else
	if(cli->pl_type == PL_LINEUP)
		dispatch_lineup(cli, buf, len);

	cli->state = CLI_READY;
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

	int cel = 0;

	while(parser->pstate == PARSER_RUNNING) {
		element_t *e = parser_nextElement(parser);
		if(strcmp(e->name, "/row") == 0)
			cel++;
		else
		if(strcmp(e->name, "col") == 0) {
			if(strcmp(e->attributes[0].value, "id") == 0) {
				e = parser_nextElement(parser);
				if(strcmp(e->name, "#text") == 0)
					list->artists[cel].id = atoi(e->attributes[0].value);
			}
			else
			if(strcmp(e->attributes[0].value, "name") == 0) {
				e = parser_nextElement(parser);
				if(strcmp(e->name, "#text") == 0) {
					list->artists[cel].name = malloc(sizeof(char)*e->attributes[0].vlen);
					strcpy(list->artists[cel].name, e->attributes[0].value);
				}
			}
		}
	}

	populate_artistList(cli, list);
}
/*
 * Process an lineup dispatch
 */
void dispatch_lineup(client_t *cli, gchar *buf, int len)
{
	parser_t *parser = parser_init(buf, len);
	alist_t *list = allocate_artistList(cli->pl_nelements);

	int cel = 0;

	while(parser->pstate == PARSER_RUNNING) {
		element_t *e = parser_nextElement(parser);
		if(strcmp(e->name, "/row") == 0)
			cel++;
		else
		if(strcmp(e->name, "col") == 0) {
			if(strcmp(e->attributes[0].value, "id") == 0) {
				e = parser_nextElement(parser);
				if(strcmp(e->name, "#text") == 0)
					list->artists[cel].id = atoi(e->attributes[0].value);
			}
			else
			if(strcmp(e->attributes[0].value, "name") == 0) {
				e = parser_nextElement(parser);
				if(strcmp(e->name, "#text") == 0) {
					list->artists[cel].name = malloc(sizeof(char)*e->attributes[0].vlen);
					strcpy(list->artists[cel].name, e->attributes[0].value);
				}
			}
		}
	}

	populate_lineupList(cli, list);
}


alist_t *allocate_artistList(int num)
{
	alist_t *list = malloc(sizeof(alist_t));
	list->nelements = num;
	list->artists = malloc(sizeof(arow_t)*list->nelements);
}

void populate_artistList(client_t *cli, alist_t *list)
{
	GtkListStore *str = GTK_LIST_STORE(cli->artist_list);
	gtk_list_store_clear(str);

	GtkTreeIter it;
	for(int i = 0; i < list->nelements; i++) {
		gtk_list_store_append(str, &it);
		char *id = malloc(sizeof(char)<<2);
		sprintf(id, "%d", list->artists[i].id);
		gtk_list_store_set(str, &it, 0, id, -1);
		gtk_list_store_set(str, &it, 1, list->artists[i].name, -1);
	}
}

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
}
