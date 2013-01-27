#variables
CC = gcc
CFLAGS = -Wall -g -Wextra -lmpdclient -O2 -shared -fpic

all: mpd.so l51mpd.so

mpd.so: luampd.c
	$(CC) $(CFLAGS) -llua luampd.c -o mpd.so

l51mpd.so: luampd.c
	$(CC) $(CFLAGS) -llua5.1 luampd.c -o l51mpd.so -I/usr/include/lua5.1

clean:
	rm *.so
