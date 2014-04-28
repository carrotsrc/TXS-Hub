#
# MAKE CLI
#
GTK_PKG=`pkg-config --libs --cflags gtk+-3.0`
GTK_C=`pkg-config --cflags gtk+-3.0`
OBJ_DIR=obj
GFLAG=$(CFLAG) -I$(SHARED)

full:
	gcc $(GFLAG) -c client.c -o $(OBJ_DIR)/client.o $(GTK_PKG)
	gcc $(GFLAG) -c ui.c -o $(OBJ_DIR)/ui.o $(GTK_PKG)
	gcc $(GFLAG) -c entry.c -o $(OBJ_DIR)/entry.o $(GTK_PKG)
	gcc -o $(BIN)txsc \
			$(SHARED)/obj/hubcomm.o \
			$(SHARED)/obj/socket.o \
			$(SHARED)/obj/parser.o \
			$(OBJ_DIR)/client.o \
			$(OBJ_DIR)/ui.o \
			$(OBJ_DIR)/entry.o \
			$(GTK_PKG)
