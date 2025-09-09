# Allow overriding compiler and flags
CC ?= gcc
CFLAGS ?= -Wall -Wextra $(DEFS)
LDFLAGS ?=

OBJS = main.o socket.o person.o
TARGET = lockpi

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
