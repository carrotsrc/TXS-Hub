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
#include "ui.h"
#include "socket.h"
#include "hubcomm.h"

void populate_tree_model(GtkListStore *store);
void clear_tree_model(GtkWidget *widget, gpointer data);
gboolean read_chan (GIOChannel *gio, GIOCondition condition, gpointer data);
gboolean perform_lookup(GtkWidget *widget, GdkEvent *event, gpointer user_data);

int main(int argc, char *argv[])
{

	// setup the client descriptor
	client_t cli;
	cli.state = CLI_UNDEF;
	cli.pl_size = 0;
	cli.pl_type = 0;

	// setup the socket for connection to the server
	int sock = net_create_socket();
	if(net_connect(sock, "127.0.0.1", 9904)<0) {
		printf("Problem connecting");
		close(sock);
		return 1;
	}

	// set it to nonblocking since it is being used
	// in multiplexing
	net_sock_nonblocking(sock);

	/* set the socket to a GTK+ channel.
	 * we are not going to handle the socket file descriptor directly, but deal with
	 * the GTK+ channel. This means we can watch for socket events through GTK+'s 
	 * multiplex loop. No need for multiple poll loops to confuse the situation.
	 */
	GIOChannel *gsock = g_io_channel_unix_new(sock);
	cli.sock = gsock; // the client will deal with the channel, not the fd

	g_io_channel_set_encoding(gsock, NULL, NULL);
	g_io_add_watch(gsock, G_IO_IN, read_chan, (gpointer)&cli); // watch out for input on the socket channel


	// a bunch of widgets
	GtkWidget *window;
	GtkWidget *bt;
	GtkWidget *lb;
	GtkWidget *tx;
	GtkWidget *frame;
	GtkWidget *scroll;
	GtkWidget *bin;
	GtkWidget *box;



	gtk_init(&argc, &argv);

	window = txs_create_window();

	/*
	 * The following init code is setting up the layout
	 * of the client window. This is not very elegent code
	 * since in the process of learning GTK.
	 *
	 * the client window is fugly.
	 */
	frame = gtk_grid_new ();
	gtk_grid_set_column_spacing(GTK_GRID(frame), 7);
	g_object_set (frame, "expand", TRUE, NULL);
	gtk_container_add(GTK_CONTAINER(window), frame);

	bin = gtk_frame_new("Playlist");
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

/*
 * Add to playlist
 */
	bt = gtk_button_new_with_label("Add to Playlist");
	g_signal_connect(bt, "clicked", G_CALLBACK(ui_perform_playlist_add), (gpointer)&cli);
	gtk_box_pack_end(GTK_BOX(box), bt, FALSE, FALSE, 0);
	//gtk_grid_attach(GTK_GRID(frame), bt, 0,1,2,1);
	//g_object_set (bt, "expand", TRUE, NULL);

/*
 * Artist list
 */

	GtkListStore *st;
	GtkWidget *tree;
	GtkTreeViewColumn *col;
	GtkCellRenderer *rnd;

	st = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	cli.artist_list = st;

	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(st));

	rnd = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(rnd), "foreground", "green", NULL);

	col = gtk_tree_view_column_new_with_attributes("id", rnd, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);

	col = gtk_tree_view_column_new_with_attributes("Artist", rnd, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);

	// here we are setting the artist list to be scrollable
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300); // make the list scrollable
	gtk_container_add(GTK_CONTAINER(scroll), tree);
	
	//gtk_grid_attach(GTK_GRID(frame),
	//scroll, 0,2,2,1);
	gtk_box_pack_end(GTK_BOX(box), scroll, TRUE, TRUE, 0);
	cli.selector = (void*)tree;
//	g_object_set (scroll, "expand", TRUE, NULL);

/*
 * Artist search/add
 */
	tx = gtk_entry_new();

	// set a callback on the text input box so when a key is released it will perform
	// a query with the server
	g_signal_connect(tx, "key-release-event", G_CALLBACK(ui_perform_lookup), (gpointer)&cli);
	gtk_box_pack_end(GTK_BOX(box), tx, TRUE, TRUE, 0);
	cli.dbsearch = (void*)tx;
	g_object_set (tx, "expand", TRUE, NULL);

/*
 * Add to database
 */
	bt = gtk_button_new_with_label("Add to Database");

	// callback for button when clicked will add an artist to the database
	g_signal_connect(bt, "clicked", G_CALLBACK(ui_perform_add), (gpointer)&cli);
	gtk_box_pack_end(GTK_BOX(box), bt, FALSE, FALSE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(box), 5);
	gtk_container_add(GTK_CONTAINER(bin), box);
	gtk_grid_attach(GTK_GRID(frame), bin, 0,0,1,1);


// COL 2

	bin = gtk_frame_new("Line-Up");
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
// lineup  set current
	bt = gtk_button_new_with_label("Set as playing");
	g_signal_connect(bt, "clicked", G_CALLBACK(ui_perform_lineup_current), (gpointer)&cli);
	gtk_box_pack_end(GTK_BOX(box), bt, FALSE, FALSE, 0);

// lineup list
	st = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	cli.lineup_list = st;

	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(st));

	rnd = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(rnd), "foreground", "green", NULL);

	col = gtk_tree_view_column_new_with_attributes("id", rnd, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);

	col = gtk_tree_view_column_new_with_attributes("DJ", rnd, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
	gtk_container_add(GTK_CONTAINER(scroll), tree);

	gtk_box_pack_end(GTK_BOX(box), scroll, TRUE, TRUE, 0);
	
	cli.lineup_selector = (void*)tree;
	g_object_set (scroll, "expand", TRUE, NULL);

// lineup  refresh
	bt = gtk_button_new_with_label("Refresh Lineup");
	g_signal_connect(bt, "clicked", G_CALLBACK(ui_perform_db_lineup), (gpointer)&cli);
	gtk_box_pack_end(GTK_BOX(box), bt, FALSE, FALSE, 0);
	g_object_set (bt, "expand", TRUE, NULL);


	gtk_container_set_border_width(GTK_CONTAINER(box), 5);
	gtk_container_add(GTK_CONTAINER(bin), box);
	gtk_grid_attach(GTK_GRID(frame), bin, 1,0,1,1);


// messages

	bin = gtk_frame_new("Messages");
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

// message

	tx = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(box), tx, TRUE, TRUE, 0);
	cli.message = (void*)tx;
// add message
	bt = gtk_button_new_with_label("Add Message");
	g_signal_connect(bt, "clicked", G_CALLBACK(ui_perform_message), (gpointer)&cli);
	gtk_box_pack_start(GTK_BOX(box), bt, FALSE, FALSE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(box), 5);
	gtk_container_add(GTK_CONTAINER(bin), box);
	gtk_grid_attach(GTK_GRID(frame), bin, 0,2,2,1);

// label
	lb = gtk_label_new("TXS Head Client");
	gtk_widget_set_name(lb, "gtk_txsheader");
	gtk_grid_attach(GTK_GRID(frame), lb, 0,3,1,1);



	gtk_widget_show_all(window);
	gtk_window_set_title(GTK_WINDOW(window), "TXS Client");
	gtk_main();
}

// debug function to add artists to the list
void populate_tree_model(GtkListStore *store)
{
	GtkTreeIter i;

	gtk_list_store_append(store, &i);
	gtk_list_store_set(store, &i, 0, "1", -1);
	gtk_list_store_set(store, &i, 1, "Akara", -1);

	gtk_list_store_append(store, &i);
	gtk_list_store_set(store, &i, 0, "2", -1);
	gtk_list_store_set(store, &i, 1, "Ott", -1);

	gtk_list_store_append(store, &i);
	gtk_list_store_set(store, &i, 0, "3", -1);
	gtk_list_store_set(store, &i, 1, "Shpongle", -1);

	gtk_list_store_append(store, &i);
	gtk_list_store_set(store, &i, 0, "4", -1);
	gtk_list_store_set(store, &i, 1, "Kaya project", -1);

	gtk_list_store_append(store, &i);
	gtk_list_store_set(store, &i, 0, "5", -1);
	gtk_list_store_set(store, &i, 1, "Zen Baboon", -1);
}

// clear the provided list
void clear_tree_model(GtkWidget *widget, gpointer data)
{
	gtk_list_store_clear(GTK_LIST_STORE(data));
}

/* read data from the channel
 * so here an event has triggered on the socket. Time to do sum'int.
 */
gboolean read_chan (GIOChannel *gio, GIOCondition condition, gpointer data)
{
	hph_t *hdr = malloc(sizeof(hph_t)); // allocate a header
	gchar *buf;
	gsize l;
	client_t *cli = (client_t*)data;

	// set the size of the data
	if(cli->state == CLI_WAITING_DISPATCH) {
		// the client is waiting for a payload
		l = cli->pl_size;
		buf = malloc(sizeof(char)*l); // allocate the buffer
	}
	else
		l = sizeof(hph_t); // client is only reading a standard hub header

	gsize br = 0;
	GIOStatus r;
	GError *e = NULL;
	if(cli->state == CLI_WAITING_DISPATCH)
		r = g_io_channel_read_chars(gio, buf, l, &br, &e); // read the payload off the channel
	else
		r = g_io_channel_read_chars(gio, (gchar*)hdr, l, &br, &e); // read the header off the channel

	switch(r) {
	case G_IO_STATUS_ERROR: // something borked
		g_error("Error reading %s\n", e->message);
	break;

	case G_IO_STATUS_EOF:
		g_io_channel_shutdown(gio, FALSE, &e);
	break;

	case G_IO_STATUS_NORMAL:
		if(cli->state == CLI_WAITING_DISPATCH)
			process_dispatch(gio, cli, buf, br); // process the payload
		else
			process_client(gio, cli, hdr, br); // go to client command branching

	break;
	}

	return TRUE;
}
