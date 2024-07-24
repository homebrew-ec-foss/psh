DIR := .files

CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic
SRCDIR = src
BINDIR = bin
INCDIR = src
EXECUTABLE = $(BINDIR)/psh
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

# Targets
.PHONY: all clean run valgrind debug create_dir

# Default target
all: create_dir $(BINDIR) $(EXECUTABLE)

# Ensure the .files directory exists
create_dir: 
	@echo "Creating directory $(DIR)"
	mkdir -p $(DIR)

# Ensure the bin directory exists
$(BINDIR):
	@echo "Creating directory $(BINDIR)"
	mkdir -p $(BINDIR)

# Build the executable from object files
$(EXECUTABLE): $(OBJECTS)
	@echo "Linking executable $@"
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Compile source files into object files
$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	@echo "Compiling $< into $@"
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

# Clean up object files and executable
clean:
	rm -rf $(BINDIR)/*

# Run the executable
run: all
	./$(EXECUTABLE)

# Run Valgrind on the executable
valgrind: all
	valgrind --tool=memcheck --show-leak-kinds=all --leak-check=full -s ./$(EXECUTABLE)

# Debug with GDB
debug: all
	gdb ./$(EXECUTABLE)
