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
#ifndef UI_H
#define UI_H

// These functions are used for dealing with interactions with
// the user interface


GtkWidget *txs_create_window(); // create a window
GtkWidget *txs_gen_tree(GtkListStore *store); // generate a treelist from list store


// these are functions for performing requests with the server via the socket
gboolean ui_perform_lookup(GtkWidget *widget, GdkEvent *event, gpointer data);
void ui_perform_add(GtkWidget *widget, gpointer data);
void ui_perform_playlist_add(GtkWidget *widget, gpointer data);

void ui_perform_db_lineup(GtkWidget *widget, gpointer data);
void ui_perform_lineup_current(GtkWidget *widget, gpointer data);

void ui_perform_message(GtkWidget *widget, gpointer data);
#endif
