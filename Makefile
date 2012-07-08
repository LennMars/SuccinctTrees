CC=g++
CFLAGS=-Isrc
INCLUDES=

.PHONY: all clean

all: suc_tree_test.cc src/suc_tree.o
	$(CC) $(CFLAGS) $(INCLUDES) -o suc_tree_test.out $^

src/%.o: src/%.cc
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	rm -rf *~ *.out src/*.o