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
	clang-format -i */*.c
	./bin/scc test/test.c -o ./bin/tmp.s
	gcc -static -o ./bin/tmp ./bin/tmp.s
	./bin/tmp

clean:
	-rm -f $(OBJECTS) $(TARDIR)/*

format:
	clang-format -i */*.c

.PHONY: test clean format all
