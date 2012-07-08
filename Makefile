CC=g++
CFLAGS=-Isrc
INCLUDES=

ifdef debug
 CFLAGS += -DDEBUG -g -O0
else
 CFLAGS += -O2
endif

.PHONY: all clean

all: suc_tree_test.cc src/suc_tree.o
	$(CC) $(CFLAGS) $(INCLUDES) -o suc_tree_test.out $^

src/%.o: src/%.cc
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	rm -rf *~ *.out src/*.o