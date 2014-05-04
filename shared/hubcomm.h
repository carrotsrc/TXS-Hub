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
#ifndef HUBCOMM_H
#define HUBCOMM_H
#include <stdlib.h>

// the header code
#define HUB_HAIL 101
#define HUB_OK 100
#define HUB_ERROR 900

#define HUB_DISPATCH 400
#define HUB_NOT_FOUND 404

#define HUB_REFRESH 500

/* this is a header structure for the hub
*  to send to clients.
*  make sure it is contiguous
*/
typedef struct hub_packetheader_struct {
	unsigned short code; // code of the header
	unsigned int pl_size; // size if bytes of the dispatch
	unsigned int nelements; // number of elements in the dispatch
	char end;
}__attribute__((packed)) hph_t;

hph_t *hub_header(unsigned short code, unsigned int size, unsigned nelements);


// macros for generating the correct header
#define HDR_SIZE sizeof(hph_t)
#define HEADER_HAIL() hub_header(HUB_HAIL, 0, 0)
#define HEADER_OK() hub_header(HUB_OK, 0, 0)
#define HEADER_ERROR() hub_header(HUB_ERROR, 0, 0)
#define HEADER_DISPATCH(s,n) hub_header(HUB_DISPATCH, s, n)
#define HEADER_NOTFOUND() hub_header(HUB_NOT_FOUND, 0, 0)
#define HEADER_REFRESH() hub_header(HUB_REFRESH, 0, 0)
#endif
