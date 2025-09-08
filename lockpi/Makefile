# Uncomment for parser DEBUG
#DEFS=-DDEBUG

OBJS=main.o socket.o person.o

all: main

main: $(OBJS)
	gcc -o $@ $(OBJS)

main.o: main.c
	gcc -o $@ -c main.c

socket.o: socket.c socket.h
	gcc -o $@ -c socket.c

person.o: person.c person.h
	gcc -o $@ -c person.c

clean:
	rm -f *.o main
