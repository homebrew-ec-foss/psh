CC = gcc
SRCDIR = src
BINDIR = bin
EXECUTABLE = $(BINDIR)/psh

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

.PHONY: all clean run

all: $(BINDIR) $(EXECUTABLE)

$(BINDIR):
	mkdir -p $(BINDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(OBJECTS): $(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $<

run: all
	./$(EXECUTABLE)

clean:
	rm -rf $(BINDIR)/*
