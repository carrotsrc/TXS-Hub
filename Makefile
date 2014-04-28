#
# MAKE PARENT
#
CFLAG=-std=c99 -ggdb
BIN=../
socket.o:
	gcc $(CFLAG) -c -o shared/obj/socket.o shared/socket.c

hubcomm.o:
	gcc $(CFLAG) -c -o shared/obj/hubcomm.o shared/hubcomm.c

parser.o:
	gcc $(CFLAG) -c -o shared/obj/parser.o shared/parser.c

server: socket.o hubcomm.o
	@echo -e "*** Making server..."
	$(MAKE) -C svr SHARED=../shared CFLAG="$(CFLAG)" BIN=$(BIN)

client: socket.o hubcomm.o parser.o
	@echo -e "*** Making client..."
	$(MAKE) -C cli SHARED=../shared CFLAG="$(CFLAG)" BIN=$(BIN)

head: socket.o hubcomm.o
	@echo -e "*** Making head unit..."
	$(MAKE) -C head SHARED=../shared CFLAG="$(CFLAG)" BIN=$(BIN)

all: socket.o
	@echo -e "*** Making everything..."
	$(MAKE) server
	$(MAKE) client
	$(MAKE) head
