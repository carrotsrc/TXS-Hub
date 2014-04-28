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
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define STATE_BLOCK 0
#define STATE_ELEMENT_NAME 1
#define STATE_ATTRIBUTE 2
#define STATE_VALUE 3
#define STATE_STRING 5

#define end() state[cstate-1]
#define push_state(s) state[cstate]=s; cstate++;
#define pop_state() state[cstate-1]=0; cstate--;
#define append(c) str[cp]=c; cp++; str[cp] = '\0';
#define reset() cp = 0;

parser_t *parser_init(char *xml, int len)
{
	parser_t *p = malloc(sizeof(parser_t));
	p->xml = xml;
	p->len = len;
	p->loc = 0;
	p->pstate = PARSER_RUNNING;

	return p;
}


int allocateAttributes(parser_t *parser, element_t *element)
{
	int opos = parser->loc;
	int total = 0;
	while(parser->loc < parser->len) {
		if(parser->xml[parser->loc] == '=')
			total++;
		else
		if(parser->xml[parser->loc] == '/' || parser->xml[parser->loc] == '>')
			break;
		parser->loc++;
	}

	element->nattr = total;
	if(total > 0)
		element->attributes = malloc(sizeof(attribute_t)*total);
	else
		element->attributes = NULL;


	parser->loc = opos;
	return total;
}

element_t *parser_nextElement(parser_t *parser)
{
	short state[32];
	short cstate = 0;
	short estate = 0;
	short cattr = 0;

	element_t *e = malloc(sizeof(element_t));
	e->name = NULL;
	e->nattr = 0;
	e->attributes = NULL;

	attribute_t *a = malloc(sizeof(attribute_t));

	char ch;
	int bsize = sizeof(char)<<8;
	char *str = malloc(bsize);
	short cp = 0;
	push_state(STATE_BLOCK);
	while(parser->loc < parser->len) {
		ch = parser->xml[parser->loc];
		estate = end();

		switch(ch) {
		case '<':
			if(cp > 0) {
				e->name = "#text";
				e->nattr = 1;
				a->name="#text";
				a->value = malloc(sizeof(char)*cp);
				a->vlen = cp;
				strcpy(a->value, str);
				e->attributes = malloc(sizeof(attribute_t));
				memcpy(e->attributes, a, sizeof(attribute_t));
				return e;
			}

			push_state(STATE_ELEMENT_NAME);
		break;

		case '>':
			if(estate == STATE_ELEMENT_NAME) {
				if(e->name == NULL ){
					e->name = malloc(sizeof(char)*cp);
					strcpy(e->name, str);
				}
				parser->loc++;
				return e;

			}
			if(estate == STATE_VALUE) {
				a->value = malloc(sizeof(char)*cp);
				a->vlen = cp;
				strcpy(a->value, str);
				reset();

				memcpy(&e->attributes[cattr], a, sizeof(attribute_t));
				cattr++;
				parser->loc++;
				return e;
			}
			else
			if(estate == STATE_BLOCK) {
				if(cp == bsize-1) {
					bsize = ((bsize>>8)+1)<<8;
					str = realloc(str, bsize);
				}
				append(ch);
			}
		break;

		case '/':
			if(estate == STATE_ELEMENT_NAME) {
				append(ch);
			}
			else
			if(estate == STATE_VALUE) {
				a->value = malloc(sizeof(char)*cp);
				a->vlen = cp;
				strcpy(a->value, str);
				reset();

				memcpy(&e->attributes[cattr], a, sizeof(attribute_t));
				cattr++;
			}
			else
			if(estate == STATE_BLOCK) {
				if(cp == bsize-1) {
					bsize = ((bsize>>8)+1)<<8;
					str = realloc(str, bsize);
				}
				append(ch);
			}
		break;

		case '=':
			if(estate == STATE_ATTRIBUTE) {
				a->name = malloc(sizeof(char)*cp);
				strcpy(a->name, str);
				reset();
				push_state(STATE_VALUE);
			}
			else
			if(estate == STATE_BLOCK) {
				if(cp == bsize-1) {
					bsize = ((bsize>>8)+1)<<8;
					str = realloc(str, bsize);
				}
				append(ch);
			}

		break;

		case ' ':
			if(estate == STATE_ELEMENT_NAME) { // set current element name
				e->name = malloc(sizeof(char)*cp);
				strcpy(e->name, str);
				if(allocateAttributes(parser, e) == 0) // we have a full element already
					break; // we don't return it yet to keep flow cleaner

				reset(); // reset char point
				push_state(STATE_ATTRIBUTE);
			}
			else
			if(estate == STATE_STRING || estate == STATE_BLOCK) {
				if(cp == bsize-1) {
					bsize = ((bsize>>8)+1)<<8;
					str = realloc(str, bsize);
				}
				append(ch);
			}
			else
			if(estate == STATE_VALUE) {
				a->value = malloc(sizeof(char)*cp);
				a->vlen = cp;
				strcpy(a->value, str);
				reset();
				pop_state();

				memcpy(&e->attributes[cattr], a, sizeof(attribute_t));
				cattr++;
			}
		break;

		case '"':
			if(estate == STATE_BLOCK) {
				append(ch);
			}
			else
			if(estate == STATE_STRING) {
				pop_state();
			}
			else {
				push_state(STATE_STRING);
			}
		break;

		default:
			if(cp == bsize-1) {
				bsize = ((bsize>>8)+1)<<8;
				str = realloc(str, bsize);
			}
			append(ch);
		break;
		}

		parser->loc++;
	}

	if(cp > 0) {
		e->name = "#text";
		e->nattr = 1;
		a->name="#text";
		a->value = malloc(sizeof(char)*cp);
		a->vlen = cp;
		strcpy(a->value, str);
		e->attributes = malloc(sizeof(attribute_t));
		memcpy(e->attributes, a, sizeof(attribute_t));
		parser->pstate = PARSER_EOF;
		return e;
	}

	parser->pstate = PARSER_EOF;
	return NULL;
}
