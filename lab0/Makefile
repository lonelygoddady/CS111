CC=gcc
CFLAGS=-Wall
OUT=lab0
OBJS=lab0.o

$(OUT): lab0.o
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS)
lab0.o: lab0.c
	$(CC) $(CFLAGS) -c lab0.c -o lab0.o

.PHONY: clean dist check

clean:
	rm -rf *.o $(OUT)

dist:
	tar -cvf lab0.gz.tar lab0 lab0.c test.txt

check:
	#./lab0 --input=test.txt --output=out.txt
	#./lab0 < test.txt > output.txt
	#./lab0 --input=test.txt > output.txt
	#./lab0 --output=output.txt < test.txt
	#./lab0 --input=test.txt --output=output.txt
	./lab0 --segfault --catch
