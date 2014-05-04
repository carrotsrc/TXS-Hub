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

#define TXS_WIN_WIDTH 800
#define TXS_WIN_HEIGHT 400
#include "common.h"
#include "client.h"

// a method for quickly creating a window
GtkWidget *txs_create_window()
{
	GtkWidget *window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), TXS_WIN_WIDTH, TXS_WIN_HEIGHT);
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	return window;
}

// generate a tree widget from a list store
GtkWidget *txs_gen_tree(GtkListStore *store)
{
	GtkWidget *tree;
	gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
}

/* The following functions send data to the server via the socket. They
 * are callback functions tied to widget events on the client window.
 */

// perform an artist lookup query with the server
gboolean ui_perform_lookup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// get the current search value from the text box
	const gchar *buf = gtk_entry_get_text(GTK_ENTRY(widget));
	gsize bw = 0;
	GError *e = NULL;

	guint16 len = gtk_entry_get_text_length(GTK_ENTRY(widget));

	const gchar sch[len+9];
	client_t *client = (client_t*)data; // cast the pointer data to the client descriptor
	sprintf((char * restrict)&sch, "db get %s\n\0", buf);
	g_io_channel_write_chars(client->sock, sch, len+9, &bw, &e); // write the query to the socket

	// CRITICAL- must flush the socket to send the data. Otherwise you can spend a few hours
	// wondering why nothing is coming through.
	g_io_channel_flush(client->sock, NULL);

	// set the client's expectant payload to artists
	client->pl_type = PL_ARTISTS;
	
	return TRUE;
}

// add the current artist in the search box to the database
void ui_perform_add(GtkWidget *widget, gpointer data)
{
	client_t *client = (client_t*)data; // cast the pointer data to the client descriptor

	// get the value in the search box
	const gchar *buf = gtk_entry_get_text(GTK_ENTRY(client->dbsearch));
	gsize bw = 0;
	GError *e = NULL;

	guint16 len = gtk_entry_get_text_length(GTK_ENTRY(client->dbsearch));

	const gchar sch[len+9];
	sprintf((char * restrict)&sch, "db add %s\n\0", buf);
	g_io_channel_write_chars(client->sock, sch, len+9, &bw, &e);
	g_io_channel_flush(client->sock, NULL); // flush to send the data
}

// add the currently selected artist in the artist list box to the playlist
// on the server
void ui_perform_playlist_add(GtkWidget *widget, gpointer data)
{
	client_t *client = (client_t*)data; // cast the pointer data to the client descriptor
	GtkTreeModel *model;
	GtkTreeIter iter;

	// create a new tree selector for finding out who is selected on the artist list
	GtkTreeView *selector = client->selector; // set the selector to the artist selector
	GtkTreeSelection *selection = gtk_tree_view_get_selection(selector); // get the current selection
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *name;

		// get the column index 0 value and place it in name
		gtk_tree_model_get(model, &iter, 0, &name, -1);

		gsize bw = 0;
		GError *e = NULL;
		char *sch = malloc(sizeof(char)*(14+strlen(name)));

		// add by the artist name
		sprintf(sch, "playlist add %s\n\0", name);
		g_io_channel_write_chars(client->sock, sch, strlen(sch)+1, &bw, &e);
		g_io_channel_flush(client->sock, NULL); // flush to send
	}
	
}

// request the current line up from the server
void ui_perform_db_lineup(GtkWidget *widget, gpointer data)
{
	char *buf  = malloc(sizeof(char)*11);
	buf = "db lineup\n\0";
	gsize bw = 0;
	GError *e = NULL;

	client_t *client = (client_t*)data;
	g_io_channel_write_chars(client->sock, buf, 11, &bw, &e);
	g_io_channel_flush(client->sock, NULL);

	// set the client's expectant payload from dispatch to be a lineup list
	client->pl_type = PL_LINEUP;
}

// set the currently playing DJ on the line up
void ui_perform_lineup_current(GtkWidget *widget, gpointer data)
{
	client_t *client = (client_t*)data;
	GtkTreeModel *model;
	GtkTreeIter iter;

	GtkTreeView *selector = client->lineup_selector; // set the selector to the lineup selector
	GtkTreeSelection *selection = gtk_tree_view_get_selection(selector); // get the currently selected DJ
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar *name;
		// place column index 0 value from the selection into name
		// it is only one thing, so terminate the list with -1
		gtk_tree_model_get(model, &iter, 0, &name, -1); 

		gsize bw = 0;
		GError *e = NULL;
		char *sch = malloc(sizeof(char)*(14+strlen(name)));
		sprintf(sch, "lineup current %s\n\0", name);
		g_io_channel_write_chars(client->sock, sch, strlen(sch)+1, &bw, &e);
		g_io_channel_flush(client->sock, NULL);
	}

}

// add a message to the head
void ui_perform_message(GtkWidget *widget, gpointer data)
{
	client_t *client = (client_t*)data;

	// get the message from the text box
	const gchar *buf = gtk_entry_get_text(GTK_ENTRY(client->message));
	gsize bw = 0;
	GError *e = NULL;

	guint16 len = gtk_entry_get_text_length(GTK_ENTRY(client->message));

	const gchar sch[len+10];
	sprintf((char * restrict)&sch, "message %s\n\0", buf);
	g_io_channel_write_chars(client->sock, sch, len+10, &bw, &e);
	g_io_channel_flush(client->sock, NULL);
}
