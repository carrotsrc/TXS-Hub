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
#ifndef SYS_H
#define SYS_H


// headers for the whole program
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h> // multiplexing
#include <sys/types.h>
#include <sys/socket.h> // sockets
#include <fcntl.h>
#include <unistd.h>

void die(const char *error);
#define free_null(m) free(m); m = NULL;
#endif
