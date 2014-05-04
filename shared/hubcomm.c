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
#include "hubcomm.h"


// setup a hub header for sending to the client
hph_t *hub_header(unsigned short code, unsigned int size, unsigned int nelements)
{
	hph_t *h = malloc(sizeof(hph_t));
	h->code = code;
	h->pl_size = size;
	h->nelements = nelements;
	h->end = '\n';
	return h;
}
