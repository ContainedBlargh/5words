CC=gcc
CFLAGS=-std=gnu99 -ggdb -Wall -pedantic -O3

main: 5words.o
	$(CC) $(CFLAGS) -o 5words fail.c rbtree/rbtree.c 5words.o