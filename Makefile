CC = gcc
#CFLAGS = -g -Wall -Wextra -pedantic -Werror
CFLAGS = -g -Wall -Wextra -pedantic
SRCDIR = src
BINDIR = bin
EXECUTABLE = $(BINDIR)/psh

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

.PHONY: all clean run valgrind

all: $(BINDIR) $(EXECUTABLE)

$(BINDIR):
	mkdir -p $(BINDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): $(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(EXECUTABLE)

valgrind: all
	valgrind --leak-check=full ./$(EXECUTABLE) -s

clean:
	rm -rf $(BINDIR)/*

debug:
	gdb ./$(EXECUTABLE)
