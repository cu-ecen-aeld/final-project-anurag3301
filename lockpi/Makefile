# Uncomment for parser DEBUG
#DEFS=-DDEBUG

OBJS=main.o 

all: main

main: $(OBJS)
	gcc -o $@ $(OBJS)

main.o: main.c
	gcc -o $@ -c main.c


clean:
	rm -f *.o main
