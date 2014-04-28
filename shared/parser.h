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
#ifndef PARSER_H
#define PARSER_H
#define PARSER_RUNNING 1
#define PARSER_EOF 2
struct element_attribute_struct {
	char *name;
	char *value;
	int vlen; 
};
typedef struct element_attribute_struct attribute_t;

struct element_struct {
	char *name;
	attribute_t *attributes;
	short nattr;
};
typedef struct element_struct element_t;

struct xml_parser_struct {
	int loc;
	int len;
	char *xml;

	short pstate;
};
typedef struct xml_parser_struct parser_t;

parser_t *parser_init(char *xml, int len);
element_t *parser_nextElement(parser_t *parser);

#endif
