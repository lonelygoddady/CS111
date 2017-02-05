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
	tar -cvf lab1-ZHUOQILI.tar.gz Makefile simpsh.c README a b c d Benchmark_dash.sh Benchmark_bash.sh 

check:
	bash Benchmark_bash.sh
	bash Benchmark_dash.sh
