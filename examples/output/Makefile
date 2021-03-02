CFLAGS=-Wall -O2 -g $(shell pkg-config --cflags alsa) $(shell pkg-config --cflags glib-2.0) -I../../include -I../include
LIBS=-pthread $(shell pkg-config --libs glib-2.0) $(shell pkg-config --libs alsa) -lm

all:
	$(CC) -o output output.c $(CFLAGS) $(LIBS)

clean:
	rm -f main
