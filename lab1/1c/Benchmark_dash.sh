#!/bin/sh

#Benchmark 1:

# strace -c ./simpsh --rdonly a --pipe --trunc --wronly c --wronly d --command 0 2 4 cat --command 1 3 4 sort --wait 

time (cat < a | sort > c 2>d)

#Benchmark 2:

#strace -c ./simpsh --profile --rdonly a --pipe --pipe --creat --trunc --wronly c --creat --append --wronly d\
#	 --command 0 2 6 sort --command 1 4 6 cat b - --command 3 5 6 tr a-z A-Z --wait

time (sort < a | cat b - | tr a-z A-Z > c 2>>d)

#Benchmark 3:

#strace -c ./simpsh --profile --rdonly a --pipe --pipe --pipe --creat --trunc --wronly c --creat --append --wronly d\
#	 --profile --command 0 2 8 sort --profile --command 1 4 8 cat - a --profile --command 3 6 8 tr '[:space:]' 'z' --profile --command 5 7 8 uniq - c --wait

time (sort a | cat - a | tr '[:space:]' 'z' | uniq - c  > c 2>>d)
