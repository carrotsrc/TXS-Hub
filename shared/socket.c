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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#define SOCK_ADDR(c) ( (struct sockaddr*) c)
int net_create_socket()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
		return -1;

	return fd;
}

int net_bind(int sock, int port)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);
	if(bind(sock, (const struct sockaddr *)&saddr, sizeof(saddr)) < 0)
		return -1;

	return 0;
}

int net_connect(int sock, char *addr, int port)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	if(inet_pton(AF_INET, addr, &saddr.sin_addr)<0)
		return -1;

	if(connect(sock, SOCK_ADDR(&saddr), sizeof(saddr)) <0)
		return -1;

	return 0;

}

int net_listen(int sock)
{
	if(listen(sock, 5) < 0)
		return -1;
	return 0;
}

int net_sock_nonblocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if(flags < 0)
		return -1;
	
	flags |= O_NONBLOCK;
	if(fcntl(sock, F_SETFL, flags) < 0)
		return -1;

	return 0;
}

int net_accept(int sock)
{
	struct sockaddr_in caddr;
	int size = sizeof(caddr);
	return accept(sock, SOCK_ADDR(&caddr), &size);
}
