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
#include "head.h"
#include "hubcomm.h"

// this function is for processing the header sent from the hub
void process_head(GIOChannel *channel, head_t *head, hph_t *hdr, int len)
{
	GError *e = NULL;
	GIOStatus r;
	switch(hdr->code) {
	case HUB_HAIL:
		// we need to register this as the HEAD interface
		head->state = HEAD_HAIL;
		gsize bwr = 0;
		r = g_io_channel_write_chars(channel, "rhead\n\0",7 , &bwr, &e);
		g_io_channel_flush(channel, NULL);
	break;

	case HUB_OK:
		if(head->state == HEAD_HAIL) {
			printf("Hailed successfully\n");
			head->state = HEAD_READY; // head has hailed and is ready
		}
	break;

	case HUB_ERROR:
		// this is pretty unlikely
		if(head->state == HEAD_HAIL) {
			printf("Hail unsuccessful\n");
		}
	break;

	case HUB_DISPATCH:
		// this could be needed at some point
		// but at the moment all the head does
		// is refresh
		if(head->state == HEAD_READY) {
			head->state = HEAD_WAITING_DISPATCH;
			head->pl_size = hdr->pl_size;
			head->pl_nelements = hdr->nelements;

		/*	r = g_io_channel_write_chars(channel, "ok\n\0",4 , &bwr, &e);
			g_io_channel_flush(channel, NULL);
		*/

		}
	break;

	case HUB_REFRESH:
		// hub state has changed so has requested head make a 
		// refresh to display newest information
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(head->view), "http://localhost/txs");
	break;
	}
}

void process_dispatch(GIOChannel *channel, head_t *head, gchar *buf, int len)
{

}
