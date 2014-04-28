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
#ifndef HEAD_H
#define HEAD_H
#include "common.h"

#define HEAD_UNDEF 0
#define HEAD_HAIL 1
#define HEAD_READY 10
#define HEAD_WAITING_DISPATCH 40

typedef struct head_struct {
	short state;
	short pl_type;
	int pl_size;
	int pl_nelements;

	void *view;

	GIOChannel *sock;
} head_t;

void process_head(GIOChannel *channel, head_t *head, hph_t *hdr, int len);
void process_dispatch(GIOChannel *channel, head_t *head, gchar *buf, int len);

#endif
