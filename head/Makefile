GTK_PKG=`pkg-config --libs --cflags gtk+-3.0 webkitgtk-3.0`
GTK_C=`pkg-config --cflags gtk+-3.0`
OBJ_DIR=obj
GFLAG=$(CFLAG) -I$(SHARED) -I/usr/include/libsoup-2.4/
full:
	gcc $(GFLAG) $(GTK_C) -c head.c -o $(OBJ_DIR)/head.o $(GTK_PKG)
	gcc $(GFLAG) $(GTK_C) -c entry.c -o $(OBJ_DIR)/entry.o $(GTK_PKG)
	gcc -o $(BIN)txsh \
			$(SHARED)/obj/hubcomm.o \
			$(SHARED)/obj/socket.o \
			$(OBJ_DIR)/head.o \
			$(OBJ_DIR)/entry.o \
			$(GTK_PKG)
