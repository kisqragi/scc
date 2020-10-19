CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

scc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): scc.h

test: scc 
	./test.sh

clean:
	rm -f scc *.o *~ tmp*

.PHONY: test clean
