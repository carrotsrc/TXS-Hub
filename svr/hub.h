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
#ifndef HUB_H
#define HUB_H
#include "hubcomm.h" // a bunch of shared hub commmunications


// wire types
#define WIRE_HEAD 1
#define WIRE_CLI 2
#define WIRE_VIS 3
#define WIRE_UNDEF 0

// hub states
#define STATE_RUN 0
#define STATE_READY 1
#define STATE_WAITING_DISPATCH 40
#define STATE_KILL 90

// CLI params

// -v for verbose
#define HFLAG_VERBOSE 1


// structure data waiting dispatch
typedef struct hub_dispatch_struct {
	int fd;
	void *payload;
	int size;
}__attribute__((packed)) hdispatch_t;

/* the hub descriptor
 * defines wire descriptors for the 
 * different client descriptors
 */
typedef struct hubdesc_struct {
	int head; // the head wire
	int cli; // the client wire
	int vis; // the visuals rig wire
	int state; // current state of hub
	int flags; // CLI params
	int *dbp; // the database connection

	// payload waiting dispatch
	hdispatch_t *dispatch;
} hubdesc_t;


int process_hub(int sfd, hubdesc_t *hub);
hubdesc_t *init_hub();
void free_hub(hubdesc_t *hub);
#endif
