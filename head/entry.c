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
	head_t head;
	head.state = HEAD_UNDEF;


	int sock = net_create_socket();
	if(net_connect(sock, "127.0.0.1", 9904)<0) {
		printf("Problem connecting");
		close(sock);
		return 1;
	}
	net_sock_nonblocking(sock);
	GIOChannel *gsock = g_io_channel_unix_new(sock);
	head.sock = gsock;

	g_io_channel_set_encoding(gsock, NULL, NULL);
	g_io_add_watch(gsock, G_IO_IN, read_chan, (gpointer)&head);

	GtkWidget *win = NULL;

	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init (&argc, &argv);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize (win);
	gtk_window_fullscreen(GTK_WINDOW(win));


	g_signal_connect (win, "destroy", gtk_main_quit, NULL);
	WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
	head.view = (void*)webView;
	gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(webView));
	webkit_web_view_load_uri(webView, "http://localhost/txs");

	gtk_widget_show_all (win);
	gtk_main ();
}

gboolean read_chan (GIOChannel *gio, GIOCondition condition, gpointer data)
{
	hph_t *hdr = malloc(sizeof(hph_t));
	gchar *buf;
	gsize l;
	head_t *cli = (head_t*)data;

	if(cli->state == HEAD_WAITING_DISPATCH) {
		l = cli->pl_size;
		buf = malloc(sizeof(char)*l);
	}
	else
		l = sizeof(hph_t);

	gsize br = 0;
	GIOStatus r;
	GError *e = NULL;
	if(cli->state == HEAD_WAITING_DISPATCH)
		r = g_io_channel_read_chars(gio, buf, l, &br, &e);
	else
		r = g_io_channel_read_chars(gio, (gchar*)hdr, l, &br, &e);

	switch(r) {
	case G_IO_STATUS_ERROR:
		g_error("Error reading %s\n", e->message);
	break;

	case G_IO_STATUS_EOF:
		g_io_channel_shutdown(gio, FALSE, &e);
	break;

	case G_IO_STATUS_NORMAL:
		if(cli->state == HEAD_WAITING_DISPATCH)
			process_dispatch(gio, cli, buf, br);
		else
			process_head(gio, cli, hdr, br);

	break;
	}

	return TRUE;
}
