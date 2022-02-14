#all to have multiple executables
all: Poke
CC=gcc
CFLAGS=-I.
LIBS=-lm
#DEPS = 

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

Poke: main.o
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f *.o
