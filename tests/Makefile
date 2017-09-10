#compiler
CC=gcc

CFLAGS=-c -Wall

all: selectTest slaveTest

selectTest: selectTest.o
	$(CC) selectTest.o -o selectTest

selectTest.o:
	$(CC) $(CFLAGS) selectTest.c

slaveTest: slaveTest.o
	$(CC) slaveTest.o -o slaveTest

slaveTest.o:
	$(CC) $(CFLAGS) slaveTest.c

clean:
	rm -rf *.o selectTest slaveTest
