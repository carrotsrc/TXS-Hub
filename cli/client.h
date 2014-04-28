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

#ifndef CLIENT_H
#define CLIENT_H

#define CLI_UNDEF 0
#define CLI_HAIL 1
#define CLI_READY 10
#define CLI_WAITING_DBADD 20
#define CLI_WAITING_DISPATCH 40

#define PL_ARTISTS 1
#define PL_LINEUP 2

typedef struct client_struct {
	short state;
	short pl_type;
	int pl_size;
	int pl_nelements;

	GIOChannel *sock;
	
	void *artist_list;
	void *selector;
	void *dbsearch;

	void *lineup_list;
	void *lineup_selector;

	void *message;

} client_t;

typedef struct artist_row {
	int id;
	char *name;
} arow_t;

typedef struct artist_list {
	int nelements;
	arow_t *artists;
} alist_t;

void process_client(GIOChannel *channel, client_t *cli, hph_t *hdr, int len);
void process_dispatch(GIOChannel *channel, client_t *cli, gchar *buf, int len);

#endif
