#!/bin/bash

echo "lab1 test" > Input.txt        
touch Output.txt        
touch error.txt        
touch message.txt

echo "test 1"        
echo "should output nothing"
./simpsh --rdonly Input.txt > Output.txt        
cat Output.txt

echo
echo "test 2"        
echo "should output no such file/directory"        
./simpsh --rdonly non_exist.txt > Output.txt
cat Output.txt
        
echo
echo "test 3"        
echo "should output incorrect file descriptor"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --verbose --command 0 1 5 wc > message.txt

echo "should not output anything"               
echo "error.txt is "        
cat error.txt

echo
echo "message.txt should be 'wc'"
cat message.txt

echo
echo "test 4"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --verbose --command 0 1 2 cat Input.txt > message.txt

echo "input.txt should has the same content as output.txt"
echo "Input.txt is "
cat Input.txt

echo
echo "Output.txt is "
cat Output.txt
echo

echo "should output nothing"
echo "error.txt is "
cat error.txt

echo
echo "should output the sub command 'cat Input.txt'"
cat message.txt

echo
echo "test 5"
echo "It should output the command executed"
echo "It should sleep for 5 seconds"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --command 0 1 2 sleep 5 --wait

echo
echo "test 6"
echo "It should not sleep for 5 seconds and output subcommand executed 'sleep 5'"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --verbose --command 0 1 2 sleep 5

echo
echo
echo "test 7"
echo "It should output 'incorrect file descriptor for subcommand 0'"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --close 1 \
    --command 0 1 2 cat

cat error.txt

echo
echo "test 8"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --pipe \
    --command 0 4 2 wc --command 3 1 2 cat --close 3 --close 4\
    --wait > /dev/null 

echo "It should output the wc of input.txt"
echo "Output.txt is "
cat Output.txt

echo
echo "test 9"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --pipe \
    --command 0 4 2 wc --command 3 1 2 cat --close 3 --close 4\
    --abort

echo "there should be a seg fault"


