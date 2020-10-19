CFLAGS=-std=c11 -g -fno-common
TARGET    = ./bin/scc
TARDIR    = ./bin
SRCDIR    = ./src
SOURCES   = $(wildcard $(SRCDIR)/*.c)
OBJDIR    = ./obj
OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/scc.h
	-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

all: clean $(TARGET)

test: $(TARGET)
	./test.sh

clean:
	-rm -f $(OBJECTS) $(TARDIR)/*

.PHONY: test clean all
