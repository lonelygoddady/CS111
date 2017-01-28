#!/bin/bash

echo "CS 111 lab1 test" > Input.txt        
touch Output.txt        
touch error.txt        
touch message.txt

echo "test 1"        
./simpsh --rdonly Input.txt > Output.txt        
cat Output.txt
        
echo "test 2"        
./simpsh --rdonly non_exist.txt > Output.txt        
cat Output.txt
        
echo "test 3"        
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --verbose --command 0 1 5 wc > message.txt
               
echo "error.txt is "        
cat error.txt
        
echo "message.txt is"
cat message.txt

echo "test 4"
./simpsh \
    --rdonly Input.txt \
    --wronly Output.txt \
    --wronly error.txt \
    --verbose --command 0 1 2 cat Input.txt > message.txt

echo "Input.txt is "
cat Input.txt

echo " "
echo "Output.txt is "

cat Output.txt
echo " "

echo "error.txt is "
cat error.txt

echo "message.txt is"
cat message.txt
