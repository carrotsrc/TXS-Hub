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
#include <stdio.h>
#include "parse.h"
#include "hubcomm.h"
#include "client.h"



void parser_start_element(GMarkupParseContext *context, const gchar *ename, const gchar **anames, const gchar avalues, gpointer data, GError **e)
{
	printf("<%s>\n", ename);
}

void parser_end_element(GMarkupParseContext *context, const gchar *ename, gpointer data, GError **e)
{
	if(strcmp(ename, "row") == 0) {
		alist_t *list = (alist_t*) data;
		list->celement++;
	}

}

void parser_text(GMarkupParseContext *context, const gchar *text, gsize len, gpointer data, GError **e)
{
	

}

void parser_error(GMarkupParseContext *context, GError *e, gpointer data)
{
	printf("Error\n");
}

void parser_destroy(gpointer data)
{
	g_free(data);
}
