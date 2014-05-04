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
#include "socket.h"
#include "hubcomm.h"
#include "head.h"

gboolean read_chan (GIOChannel *gio, GIOCondition condition, gpointer data);

int main(int argc, char *argv[])
{
	// create the head descriptor
	head_t head;
	head.state = HEAD_UNDEF; // undefined state

	// create a new network socket
	int sock = net_create_socket();

	// connect to the hub
	if(net_connect(sock, "127.0.0.1", 9904)<0) {
		printf("Problem connecting");
		close(sock);
		return 1;
	}

	// set the socket to nonblocking since we are multiplexing
	net_sock_nonblocking(sock);

	// we are using GTK+ channels. This means that we can
	// put the socket into the GTK event loop instead of
	// having a separate poll. Assign the socket file
	// descriptor to the channel
	GIOChannel *gsock = g_io_channel_unix_new(sock);
	head.sock = gsock;

	g_io_channel_set_encoding(gsock, NULL, NULL);
	// watch for read events on the socket in the channel
	g_io_add_watch(gsock, G_IO_IN, read_chan, (gpointer)&head);

	GtkWidget *win = NULL;

	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init (&argc, &argv);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize (win);
	gtk_window_fullscreen(GTK_WINDOW(win));


	g_signal_connect (win, "destroy", gtk_main_quit, NULL);

	// create a new webkit widget
	WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

	// set the webkit widget in the head descriptor
	head.view = (void*)webView;
	gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(webView));

	// direct webkit widget to local webpage
	webkit_web_view_load_uri(webView, "http://localhost/txs");

	gtk_widget_show_all (win);
	gtk_main ();
}

// this is for reading data off the channel. This is callback for
// when a read event is triggered in the event loop.
gboolean read_chan (GIOChannel *gio, GIOCondition condition, gpointer data)
{
	hph_t *hdr = malloc(sizeof(hph_t)); // allocate a new header
	gchar *buf;
	gsize l;
	head_t *cli = (head_t*)data; // cast the data pointer to a head descriptor

	// get size of data
	if(cli->state == HEAD_WAITING_DISPATCH) {
		// we have a payload, allocate buffer
		// and set size to that specified by
		// dispatch header
		l = cli->pl_size;
		buf = malloc(sizeof(char)*l);
	}
	else
		l = sizeof(hph_t); // size is header size

	gsize br = 0;
	GIOStatus r;
	GError *e = NULL;
	if(cli->state == HEAD_WAITING_DISPATCH)
		r = g_io_channel_read_chars(gio, buf, l, &br, &e); // read into the payload buffer
	else
		r = g_io_channel_read_chars(gio, (gchar*)hdr, l, &br, &e); // read in commands

	switch(r) {
	case G_IO_STATUS_ERROR: // come across an error
		g_error("Error reading %s\n", e->message);
	break;

	case G_IO_STATUS_EOF:
		g_io_channel_shutdown(gio, FALSE, &e);
	break;

	case G_IO_STATUS_NORMAL:
		if(cli->state == HEAD_WAITING_DISPATCH)
			process_dispatch(gio, cli, buf, br); // process a payload
		else
			process_head(gio, cli, hdr, br); // process a command

	break;
	}

	return TRUE;
}
