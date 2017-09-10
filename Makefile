#compiler
CC=gcc

CFLAGS=-c -Wall

all: hash slaveProcess hashOutput

hash: hash.o queue.o
	$(CC) hash.o queue.o -o hash

hash.o:
	$(CC) $(CFLAGS) hash.c

slaveProcess: slaveProcess.o queue.o
	$(CC) slaveProcess.o queue.o -o slaveProcess

slaveProcess.o:
	$(CC) $(CFLAGS) slaveProcess.c

queue.o:
	$(CC) $(CFLAGS) queue.c

hashOutput: hashOutput.o
	$(CC) hashOutput.o -o hashOutput

hashOutput.o:
	$(CC) $(CFLAGS) hashOutput.c

clean:
	rm -rf *.o hash slaveProcess hashOutput hash.txt
