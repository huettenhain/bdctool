CC=gcc
BINARY = bdctool
TARGET = permanent
CFLAGS = -std=c99 -pedantic -Wall -Wextra
DFLAGS = -g -O0 -DDEBUG -DPRINT_PROGRESS
RFLAGS = -O2

SOURCES = *.c ./targets/$(TARGET).c
HEADERS = *.h
OBJECTS = bdctool.o finitefield.o binmatrix.o target.o

debug: $(OBJECTS)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(BINARY) $(OBJECTS)

openmp: RFLAGS += -fopenmp
openmp: release
release: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(BINARY) $(SOURCES) $(RFLAGS) 

clean:
	-rm $(OBJECTS)
	-rm *.stackdump

.PHONY: go debug openmp release clean

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) -c -o $@ $< 

target.o: ./targets/$(TARGET).c
	$(CC) $(CFLAGS) $(DFLAGS) -c -o $@ $< 

