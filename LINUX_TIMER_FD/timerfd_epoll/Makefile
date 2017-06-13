# Makefile for periodic threads example programs
# Chris Simmonds
# chris@2net.co.uk

CFLAGS = -Wall -g -lrt
PROGS = timerfd

all: $(PROGS)

timerfd: timerfd.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

clean:
	rm -f *.o
	rm -f $(PROGS)

install:
	cp $(PROGS) $(TARGETDIR)/bin
