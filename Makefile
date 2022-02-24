#all to have multiple executables
all: Poke
CC=gcc
CFLAGS=-I.
LIBS=-lm
#DEPS = 

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

Poke: main.o heap.c heap.h
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f *.o

clean:
	rm -f *.o output Poke
	rm -f *.tar.gz
