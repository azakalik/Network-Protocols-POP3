CC = gcc
CFLAGS = -Wall -Iinclude -g 
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

TARGET = $(BINDIR)/project

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJDIR)
$(TARGET): | $(BINDIR)

$(OBJDIR) $(BINDIR):
	@mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(OBJDIR) $(BINDIR)

all: $(TARGET)

.DEFAULT_GOAL := all
