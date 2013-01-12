#variables
CC = gcc
CFLAGS = -Wall -g -Wextra -llua -lmpdclient -O2 -shared -fpic

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: mpd.so

mpd.so: luampd.c
	$(CC) $(CFLAGS) luampd.c -o mpd.so

clean:
	rm *.so
