CFLAGS = -Wall -std=gnu11 -O2 -g $(shell pkg-config --cflags alsa) $(shell pkg-config --cflags glib-2.0) -I../../include
LDFLAGS = -lm -lsoundio
LIBS=-pthread $(shell pkg-config --libs glib-2.0) $(shell pkg-config --libs alsa)

all:
	$(CC) $(CFLAGS) *.c -o main $(LDFLAGS) $(LIBS)

clean:
	rm -f main
