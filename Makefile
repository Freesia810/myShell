OBJS=myshell.o console.o Command.o parser.o joblist.o handler.o
CC=g++
CFLAGS=-c -Wall -g

myshell:$(OBJS)
	$(CC) $^ -o myshell

myshell.o:myshell.cpp
	$(CC) $^ $(CFLAGS) -o $@

console.o:console.cpp
	$(CC) $^ $(CFLAGS) -o $@

Command.o:Command.cpp
	$(CC) $^ $(CFLAGS) -o $@

parser.o:parser.cpp
	$(CC) $^ $(CFLAGS) -o $@

joblist.o:joblist.cpp
	$(CC) $^ $(CFLAGS) -o $@

handler.o:handler.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	$(RM) *.o myshell -r