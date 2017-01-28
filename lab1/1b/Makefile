CC=gcc
CFLAGS=-Wall
OUT=simpsh
OBJS=simpsh.o

$(OUT): simpsh.o
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS)
simpsh.o: simpsh.c
	$(CC) $(CFLAGS) -c simpsh.c -o simpsh.o

.PHONY: clean dist check

clean:
	rm -rf *.o $(OUT)

dist:
	tar -cvf lab1-ZHUOQILI.gz.tar Makefile simpsh.c test.sh README

check:
	bash test.sh 
