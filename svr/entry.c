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
#include <errno.h>
#include <arpa/inet.h>
#include "common.h"
#include "db.h"
#include "hub.h"
#include "hubcomm.h"
#include "socket.h"

#define MAX_EV 10
int setup_epoll(int sfd);
int setup_socket();
int make_nonblocking(int sfd);

int main(int argc, char *argv[])
{
	printf("TXS head server\n");
	hubdesc_t *hub = init_hub();
	
	if(hub == NULL)
		die("Failed to initialize hub. No database.\n");

	for(int v = 1; v < argc; v++) {
		if(strcmp("-v", argv[v]) == 0)
			hub->flags |= HFLAG_VERBOSE;
	}

	int epfd, sfd, csock, nfds;
	struct epoll_event ev, events[MAX_EV];

	sfd = net_create_socket();
	if(sfd < 0)
		die("Failed to create socket");

	if(net_bind(sfd, 9904) < 0)
		die("Failed to bind socket");

	net_listen(sfd);

	epfd = setup_epoll(sfd);
	if(epfd == -1) 
		die("Failed to initialize epoll");

	printf("Ready\n");
	for(;;) {
		if(hub->state == STATE_KILL) {
			close(csock);
			close(epfd);
			close(sfd);
			break;
		}
		nfds = epoll_wait(epfd, events, MAX_EV, -1);
		if(nfds == -1) {
			printf("Error waiting\n");
			break;
		}

		for(int i = 0; i < nfds; i++) {
			if(events[i].data.fd == sfd) {
				csock = net_accept(sfd);
				if(csock < 0) {
					printf("Failed to accept %d\n", errno);
					close(csock);
					close(epfd);
					close(sfd);
					break;
				}

				if(net_sock_nonblocking(csock) == -1) {
					printf("Error changing flags\n");
					close(csock);
					close(epfd);
					close(sfd);
					break;
				}
				ev.events = EPOLLIN|EPOLLET;
				ev.data.fd = csock;
				if(epoll_ctl(epfd, EPOLL_CTL_ADD, csock, &ev) == -1) {
					printf("Error registering fd\n");
					close(csock);
					close(epfd);
					close(sfd);
					break;
				}
				if(hub->flags&HFLAG_VERBOSE > 0)
					printf("Registered socket %d\n", csock);

				write(csock, HEADER_HAIL(), HDR_SIZE);
			} else {
				if(process_hub(events[i].data.fd, hub) == -1) {
					printf("Error reading from socket\n");
					close(csock);
					close(epfd);
					close(sfd);
					break;
				}
			}
		}
	}
	free_hub(hub);
	printf("Closing down\n");
	return 0;
}

int setup_epoll(int sfd)
{
	int epfd;
	struct epoll_event ev;

	epfd = epoll_create(MAX_EV);
	if(epfd == -1)
		return -1;

	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
		perror("Failed to register fd\n");
		return -1;
	}

	return epfd;
}

void die(const char *error)
{
	printf("%s\n", error);
	exit(EXIT_FAILURE);
}
