CC=gcc
CFLAGS=-Wall -pthread
OUT=lab2_add
OBJS=lab2_add.o

$(OUT): lab2_add.o
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS)
simpsh.o: lab2_add.c
	$(CC) $(CFLAGS) -c lab2_add.c -o lab2_add.o

.PHONY: clean dist check

clean:
	rm -rf *.o $(OUT)

dist:

check:
