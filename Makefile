# create .files directory before executing
DIR := .files
create_dir : 
	mkdir -p $(DIR)
all: create_dir
	@echo "$(DIR) created or already exists"

CC = gcc
#CFLAGS = -g -Wall -Wextra -pedantic -Werror
CFLAGS = -g -Wall -Wextra -pedantic
SRCDIR = src
BINDIR = bin
INCDIR = include
EXECUTABLE = $(BINDIR)/psh

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

.PHONY: all clean run valgrind

all: $(BINDIR) $(EXECUTABLE)

$(BINDIR):
	mkdir -p $(BINDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

clean:
	rm -rf $(BINDIR)/*

run: all
	./$(EXECUTABLE)

valgrind: all
	valgrind --tool=memcheck --leak-check=yes --leak-check=full ./$(EXECUTABLE) -s

debug: all
	gdb ./$(EXECUTABLE)
