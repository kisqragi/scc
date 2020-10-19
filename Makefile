CFLAGS=-std=c11 -g -fno-common

scc: main.o
	$(CC) -o scc main.o $(LDFLAGS)

test: scc 
	./test.sh

clean:
	rm -f scc *.o *~ tmp*

.PHONY: test clean
