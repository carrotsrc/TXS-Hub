#
# MAKE SVR
#
GTK_PKG=`pkg-config --libs --cflags gtk+-2.0`
OBJ_DIR=obj
GFLAG=$(CFLAG) -I$(SHARED)
full:
	gcc $(GFLAG) -c db.c -o $(OBJ_DIR)/db.o `mysql_config --cflags` $(GTK_PKG)
	gcc $(GFLAG) -c hub.c -o $(OBJ_DIR)/hub.o `mysql_config --cflags` $(GTK_PKG)
	gcc $(GFLAG) -c entry.c -o $(OBJ_DIR)/entry.o `mysql_config --cflags` $(GTK_PKG)
	gcc -o $(BIN)txss \
			$(SHARED)/obj/socket.o \
			$(SHARED)/obj/hubcomm.o \
			$(OBJ_DIR)/db.o \
			$(OBJ_DIR)/hub.o \
			$(OBJ_DIR)/entry.o \
			$(GTK_PKG) `mysql_config --libs`
