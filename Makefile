#variables
CC = gcc
CFLAGS = -Wall -g -Wextra -lmpdclient -O2 -shared -fpic

all: mpd.so l51mpd.so

mpd.so: luampd.c
	$(CC) $(CFLAGS) -llua luampd.c -o mpd.so

l51mpd.so: luampd.c
	$(CC) $(CFLAGS) -llua5.1 luampd.c -o l51mpd.so -I/usr/include/lua5.1

install: all
	install -D -m644 mpd.so $(DESTDIR)$(PREFIX)/usr/lib/lua/5.2/mpd.so
	install -D -m644 l51mpd.so $(DESTDIR)$(PREFIX)/usr/lib/lua/5.1/mpd.so

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/usr/lib/lua/5.2/mpd.so
	$(RM) $(DESTDIR)$(PREFIX)/usr/lib/lua/5.1/mpd.so

clean:
	$(RM) *.so
