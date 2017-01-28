The tar file contents three files:
	1. The Makefile which supports "make, make clean, make check and make dist" commands.
	2. The simpsh.c C source file 
	3. The test.sh script file which contains 5 simple tests. Those tests can be invoked by "make check" command

Project Feature:
	The current version of the project supports four options:
	--WRONLY "filename"  (set the "filename" write only)
	--RDONLY "filename"  (set the "filename" read only)
	--command fd1 fd2 fd3 sub_command sub_command_argu (execute the sub_command with fd1 as the stdin, fd2 as the stdout and fd3 as the stderr) 
	--verbose (print out the sub_command to stdout, usually use with --command option) 

	If any of above command has syntax error, simpsh will ignore it and execute the next available option but still set the exit status to 1. 

	If the file descriptor for the sub_command is not correct, then a error message of "incorrect file descriptor for sub_command n" will output to stderr of simpsh.  
