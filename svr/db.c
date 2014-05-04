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
#include "db.h"
#define VPDB_HOST "127.0.0.1"
#define VPDB_DATABASE "txs"
#define VPDB_USER ""
#define VPDB_PASS ""

typedef struct db_row_struct {
	char *name;
	int len;
} db_row_t;

int db_getFieldSize(MYSQL_RES *res, db_row_t *row)
{
	MYSQL_FIELD *f = NULL;
	unsigned int size = 0;
	int i = 0;
	while((f = mysql_fetch_field(res)) != NULL) {
		row[i].len = strlen(f->name);
		row[i].name = malloc(sizeof(char)*row[i].len+1);
		strcpy(row[i].name, f->name);
		size += row[i].len;
		i++;
	}

	return size;
}

unsigned int db_getColumnSize(MYSQL_RES *res, int nf, int *clen)
{
	MYSQL_ROW row;
	unsigned long size = 0;
	unsigned long *len;
	int i;
	int j = 0;
	while((row = mysql_fetch_row(res)) != NULL) {
		len = mysql_fetch_lengths(res);
		for(i = 0; i < nf; i++) {
			size += len[i];
			clen[j] = len[i];
			j++;
		}
	}
	mysql_data_seek(res, 0);

	return size;
}
int *db_connect()
{
	MYSQL *dbc = NULL;

	dbc = mysql_init(NULL);
	if(dbc == NULL)
		return NULL;

	dbc = mysql_real_connect(dbc, VPDB_HOST, VPDB_USER, VPDB_PASS, \
				VPDB_DATABASE, 0, NULL, 0);

	if(dbc == NULL)
		return NULL;

	return (int*)dbc;
}

void db_close(int *dbc)
{
	mysql_close((MYSQL*) dbc);
}

MYSQL_RES *db_query(int *dbc, char *q)
{
	if(mysql_real_query((MYSQL*)dbc, q, strlen(q)) != 0)
		return NULL;

	MYSQL_RES *r = mysql_store_result((MYSQL*)dbc);
	return r;
}

char *db_sqltoxml(MYSQL_RES *res, int *size)
{
	// get the field sizes
	int sz = 0;
	int nf, nr;
	nf = mysql_num_fields(res);
	nr = mysql_num_rows(res);
	db_row_t rows[nf];
	int clen[nr*nf]; // column length index

	sz += db_getFieldSize(res, (db_row_t*)&rows);
	sz = (sz*nr)+2;
	sz += db_getColumnSize(res, nf, (int*)&clen);


	sz += 11*nr; // number of row bytes
	sz += (19*nf)*nr; // number of col bytes for all rows
	char *xml = calloc(sz, sizeof(char));
	char *tmp = NULL;

	*size = sz;
	MYSQL_ROW row;
	int f = 0;
	int r = 0;
	int rsize = 0;

	while((row = mysql_fetch_row(res)) != NULL) {
		strcat(xml, "<row>");

		for(f = 0; f < nf; f++) {
			rsize = clen[r*nf+f]+rows[f].len+19;
			tmp = malloc(sizeof(char)*rsize+1);
			sprintf(tmp, "<col name=\"%s\">%s</col>", rows[f].name, row[f]);
			strcat(xml, tmp);
			free(tmp);
		}
		strcat(xml, "</row>");

		r++;
	}
	tmp = NULL;
	strcat(xml,"\n\0");
	return xml;
}
