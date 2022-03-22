#all to have multiple executables
all: Poke
CC=gcc
CFLAGS=-I.
LIBS=-lm
#DEPS = 

%.o: %.c
	$(CC) -g -c -o $@ $< $(CFLAGS)

Poke: main.o -lncurses heap.c heap.h
	$(CC) -g -o $@ $^ $(CFLAGS)
	rm -f *.o

clean:
	rm -f *.o output Poke
	rm -f *.tar.gz
