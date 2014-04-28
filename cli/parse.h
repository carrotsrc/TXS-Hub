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
#ifndef PARSE_H
#define PARSE_H
#include <glib.h>

void parser_start_element(GMarkupParseContext *context, const gchar *ename, const gchar **anames, const gchar avalues, gpointer data, GError **e);
void parser_end_element(GMarkupParseContext *context, const gchar *ename, gpointer data, GError **e);
void parser_text(GMarkupParseContext *context, const gchar *text, gsize len, gpointer data, GError **e);
void parser_error(GMarkupParseContext *context, GError *e, gpointer data);
void parser_destroy(gpointer data);

#endif
