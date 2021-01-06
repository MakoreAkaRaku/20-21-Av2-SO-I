CC=gcc
CFLAGS=-g -Wall -std=c99
#LDFLAGS=
 
SOURCES=nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c my_shell.c
INCLUDES=nivel1.h nivel2.h nivel3.h nivel4.h nivel5.h nivel6.h my_shell.h
PROGRAMS=nivel1 nivel2 nivel3 nivel4 nivel5 nivel6  my_shell

OBJS=$(SOURCES:.c=.o)

all: $(PROGRAMS)

%: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c %.h 
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o $(PROGRAMS)