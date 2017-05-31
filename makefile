CC=gcc
TARGET=permanent
CFLAGS=-std=c99 -Wall -pedantic -O3 -DPRINT_PROGRESS
DEPS=target.h finitefield.h binmatrix.h myassert.h
OBJECTS=bdctool.o finitefield.o binmatrix.o target.o
# -o bdctool  bdctool.c finitefield.c binmatrix.c

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

target.o: ./targets/$(TARGET).c
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: bdctool

bdctool: $(OBJECTS)
	gcc $(CFLAGS) -o $@ $^ 