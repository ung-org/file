.POSIX:

.SUFFIXES: .cat .msg

default: all

CFLAGS=-g -Wall -Wextra -Wpedantic -Werror
UTILITY=file
SOURCES=file.c magic.c
HEADERS=file.h
OBJECTS=file.o magic.o
L10N=
all: $(UTILITY) $(L10N)

$(UTILITY): $(OBJECTS) $(HEADERS)

.msg.cat:
	gencat $@ $<

.c.cat:
	sed -ne '/^\/\*\*cat/,/cat\*\*\//p;' $< | grep -v ^/ | grep -v ^\* | gencat $@ -

clean:
	rm -f *.o $(L10N) $(UTILITY)
