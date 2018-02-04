CC = gcc
CFLAGS = -g -O3 -I./ \
	 -Wall \
	 -Wextra \
	 -Wno-unused-parameter \
	 $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0)

PROG = dtreetrawl
HDRS = dtreetrawl.h
SRCS = dtreetrawl.c

OBJS = $(SRCS:.c=.o)

all : $(PROG)

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(PROG)

dtreetrawl.o : dtreetrawl.c dtreetrawl.h

.PHONY : clean install uninstall

clean :
	rm -f core $(PROG) $(OBJS)

install : uninstall
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 dtreetrawl $(DESTDIR)/usr/bin/dtreetrawl

uninstall :
	rm -f $(DESTDIR)/usr/bin/dtreetrawl
