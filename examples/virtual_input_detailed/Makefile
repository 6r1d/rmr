CFLAGS=-Wall -O2 -g $(shell pkg-config --cflags alsa) $(shell pkg-config --cflags glib-2.0) -I../../include -I../include -lm
LIBS=-pthread $(shell pkg-config --libs glib-2.0) $(shell pkg-config --libs alsa)

all:
	$(CC) -o virtual_input virtual_input.c $(CFLAGS) $(LIBS)

clean:
	rm -f main
