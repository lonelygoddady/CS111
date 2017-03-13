#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<fcntl.h>
#include<string.h>
#include<getopt.h>
#include<stdlib.h>
#include<ctype.h>

#define _GNU_SOURCE
#define subcom_num 10
 
int debug = 0;
int WAIT_FLAG = 0;
int SIGNAL = 0;
int SIGNAL_IGNORE = 0;
pid_t child_pid[10]; //array to store child process's pid number 

void sig_handler(int signo)
{
	if (signo == SIGNAL){
			char tmp[40];
			sprintf(tmp, "%d caught!\n", SIGNAL);
			perror(tmp);
			exit(SIGNAL);
		}
}

void  execute(char **argv, int* sub_com_fd, int com_num)
{
     pid_t  pid;

     if ((pid = fork()) < 0) {     // fork a child process           
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {          // for the child process:  
	 
	  if(debug == 1){
		printf("stdin for this command: %d\n", sub_com_fd[0]);	
		printf("stdout for this command: %d\n", sub_com_fd[1]);
		printf("stderr for this command: %d\n", sub_com_fd[2]);
		printf("the sub command to be execute: %s\n",argv[0]);// argv[0]); 
	  }
	  
	  //if pipe fd exist, we need to close one end before manipulate the other 
	  if (sub_com_fd[3] == 0)
	  	dup2(sub_com_fd[0],0);
	  else{
		dup2(sub_com_fd[0],0);
		close(sub_com_fd[3]);
	  }
	  
	  if (sub_com_fd[4] == 0)	   
	  	dup2(sub_com_fd[1],1);
	  else{
		dup2(sub_com_fd[1],1);
		close(sub_com_fd[4]);
	  }

	  dup2(sub_com_fd[2],2);
 
          if (execvp(argv[0], argv) < 0) {     // execute the command  
               if (debug == 1)
	        	printf("*** ERROR: exec failed\n");
              
	        exit(1);
          }
	  else{
	        //after sub_com execution, check error flag
		if (SIGNAL > 0)
			signal(SIGNAL, sig_handler);
		else if (SIGNAL_IGNORE == 1)
			signal(SIGNAL, SIG_IGN);
		else if (SIGNAL_IGNORE == 2)
			signal(SIGNAL, SIG_DFL);
		
		for (int i = 0; i < 3; i++)
			if (close(sub_com_fd[i]) < 0)
				printf("Cannot close fd: %d!\n", sub_com_fd[i]);  
       	 }
       	 if (debug == 1) 
		printf("Processing sub_com...\n");
	 
       }
     
	child_pid[com_num] = getpid();  // saves children's pid in this array

	if (debug == 2)
		printf("The child PID is: %d\n", child_pid[com_num]);
		
 	if (debug == 1)
		printf("waiting for completion\n");
 
	if (debug == 1)
		 printf("Sub_com finish\n");
}


int main(int argc, char *argv[])
{
	int opt_index = 0;
	int nargc = 0;
	char* fn0 = NULL;
	char* fn1 = NULL;
	//char* fn2 = NULL;
	int f_d[30]; //fd array that stores all fds
	int f_dp[30]; //fd_p mark fd that is a pipe
	int f_dc = 0;
	//int seg = 0;
	int opt = 0;
	
	int ERR_FLAG = 0;
	int OF_FLAG = 0; //o_flag values

	int pipefd[2];
	int sub_com_c = -1; //count how many sub command are there, first item is 0
	int sub_com_fd[subcom_num][6]; //2D int array to store sub_command file descriptors, support at most 10 commands	

	int VERBOSE_FLAG = 0;
	int FD_ERR[subcom_num];  //File descriptor error array
	
	char* buffer = NULL;
	buffer = (char*)malloc(100*sizeof(char));

	char* v_buffer = NULL;
	v_buffer = (char*)malloc(100*sizeof(char));	
	
	char **sub_com_table[subcom_num]; //10 sub command with each can contain 9 arguments, each arguments at most 20 characters

	//default stdout stdin stderr
	f_d[0] = 0;
	f_d[1] = 1;
	f_d[2] = 2;

	for (int i = 0; i < subcom_num; i++)
		child_pid[i] = -1;

	for (int i = 0; i < subcom_num; i++){
		FD_ERR[i] = 0;	
		sub_com_table[i] = (char**)malloc(200*sizeof(char)); 
	}

	for (int i = 3; i < 3*subcom_num; i++)
		f_d[i] = -1;
	
	for (int i = 0; i < 3*subcom_num; i++){
		f_dp[i] = 0;
	}

	//initialize sub_com_fd
	for (int i = 0; i< subcom_num; i++){
		sub_com_fd[i][0] = 0;
		sub_com_fd[i][1] = 1;
		sub_com_fd[i][2] = 2;
		
		for(int j = 3; j < 6; j++)
			sub_com_fd[i][j] = -1; //for pipe fd's   
	}	

	while(1)
	{
		//catch signal while processing arguments 
	
		static struct option long_options[] = {
			{"command", required_argument, NULL, 'c'},	
			{"rdonly", required_argument, NULL, 'r'},
			{"wronly", required_argument, NULL, 'w'},
			{"rdwr", required_argument, NULL, 'x'},	
			{"close", required_argument, NULL, 'l'},	
			{"abort", no_argument, NULL, 'A'},
			{"catch", required_argument, NULL, 'T'},
			{"ignore", required_argument, NULL, 'i'},
			{"default", required_argument, NULL, 'f'},
			{"pause", no_argument, NULL, 'p'},
			{"pipe", no_argument, NULL, 'P'},
			{"verbose", no_argument, NULL, 'v'},
			{"wait", no_argument, NULL, 'W'},
			{"append", no_argument, NULL, 'a'},
			{"cloexec", no_argument, NULL, 'C'},
			{"creat", no_argument, NULL, 'R'},
			{"directory", no_argument, NULL, 'd'},
			{"dsync", no_argument, NULL, 'D'},
			{"excl", no_argument, NULL, 'e'},
			{"nofollow", no_argument, NULL, 'n'},
			{"nonblock", no_argument, NULL, 'N'},
			{"rsync", no_argument, NULL, 'S'},
			{"sync", no_argument, NULL, 's'},
			{"trunc", no_argument, NULL, 't'},
			{0,0,0,0},
		};
		
		opt = getopt_long(argc, argv, "-c:r:w:x:l:AT:i:f:pvWaCRdDenNSst", long_options, &opt_index);
		if (opt == -1)
			break;

		switch(opt) {
			case 1:
				nargc ++;
				if (debug == 1)
					printf("stdin stdout and stderr fd is: %d,%d,%d\n", f_d[0],f_d[1],f_d[2]);

				switch (nargc) {
					case 1:
						if (optarg != NULL && isdigit(optarg[0])){
							int f_ind = atoi(optarg);
							if (debug == 1){
								printf("f_ind for stdout is: %d\n", f_ind);	
								printf("f_ind should be: %s\n", optarg);
								printf("the fd is: %d\n", f_d[f_ind]);	
							}

							if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 29){
								FD_ERR[sub_com_c] = 2;
							}
							else{
								sub_com_fd[sub_com_c][1] = f_d[f_ind];
								
								if(f_dp[f_ind] == -2){
									sub_com_fd[sub_com_c][4] = f_dp[f_ind-1]; //save for pipe read
								}
								else if(f_dp[f_ind] == -1){
									perror("Attempt to write to the read end of the pipe\n");
									FD_ERR[sub_com_c] = 5;
									ERR_FLAG = 1;
								}	
							}
						} 							
	
						if (debug == 1) 
							printf("stdout fd is %d\n",f_d[1]);
						break;
					case 2: 
						//file descriptor 3
						if (optarg != NULL && isdigit(optarg[0])){
							int f_ind = atoi(optarg);
							if (debug == 1){
								printf("f_ind for stderr is: %d\n", f_ind);
								printf("f_ind should be: %s\n", optarg); 
								printf("the fd is: %d\n", f_d[f_ind]);					
								printf("Stderr fd is: %d\n", f_d[2]);
							}
	
							if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 29){
								FD_ERR[sub_com_c] = 3;

								if (debug == 1){
									printf("output stderr msg\n");
									printf("the stderr fd is: %d\n", f_d[2]);	
								}
							}
							else{ 
								sub_com_fd[sub_com_c][2] = f_d[f_ind];
							}
						}

						if (debug == 1) 
							printf("stderr fd is: %d\n",f_d[2]);
						break;
					case 3:
						//this is the sub command 
						if (optarg != NULL){
							//push subcommand to the table
							//strcpy(sub_com_table[sub_com_c][0], optarg);
							sub_com_table[sub_com_c][0] = optarg;
							
							if (debug == 1)
								printf("the sub command is: %s\n", sub_com_table[sub_com_c][0]);		
						}					
						break;
					default:
						if (optarg != NULL){
							//push subcommand argument to the table
							//strcpy(sub_com_table[sub_com_c][nargc-3],optarg);
							sub_com_table[sub_com_c][nargc-3] = optarg;
							
							if (debug == 1)
								printf("the sub command is: %s\n", sub_com_table[sub_com_c][0]);			
						}
						break;
				}
				break;

			case 'c':
				//reset argument index for the sub command
				//increment the sub command found 
				nargc = 0; 
				sub_com_c ++;
			
				if (optarg != NULL && isdigit(optarg[0])){
					int f_ind = atoi(optarg);
					if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 29){
						FD_ERR[sub_com_c] = 1;	
					}
					else{ 
						sub_com_fd[sub_com_c][0] = f_d[f_ind];

						if(f_dp[f_ind] == -1){
							sub_com_fd[sub_com_c][3] = f_dp[f_ind+1]; //save for pipe write
						}
						else if(f_dp[f_ind] == -2){
							perror("Attempt to write to the read end of the pipe\n");
							FD_ERR[sub_com_c] = 5;
							ERR_FLAG = 1;
						}
					
						if (debug == 1){
							printf("Sub_com_fd table has change: %d\n", sub_com_fd[sub_com_c][0]);
							printf("Sub_com_fd table has change: %d\n", sub_com_fd[sub_com_c][3]);	
						}
					}
				}
				else{
					FD_ERR[sub_com_c] = 6;
					ERR_FLAG = 1; //for exit status 
				}
				break;
			case 'r':
				if (optarg != NULL)
				{	
					fn0 = optarg;
					if((f_d[f_dc] = open(fn0, OF_FLAG |  O_RDONLY, 00666)) == -1)
					{
						fn0 = NULL;
						perror("Cannot open file to read\n");
						ERR_FLAG = 1;						
					}
					OF_FLAG = 0; //reset flag values
					f_dc ++; //keep track the number of file descriptor used
				}

				if (debug == 1) 
					printf("Filename for read only: %s\n",fn0);

			break;
			case 'w':
				if (optarg != NULL)
				{	
					fn1 = optarg;
					if((f_d[f_dc] = open(fn1, OF_FLAG | O_WRONLY, 00666)) == -1)
					{
						if (f_dc != 2){
							fn1 = NULL;
							perror("Cannot write to file\n");
							ERR_FLAG = 1;
						}
						else 
						{
							//fn2 = NULL;
							perror("Standar error file cannot be open\n");
							exit(1);
						}
					}
					OF_FLAG = 0;
					f_dc ++;
				}
			
				if (debug == 1)
					printf("Filename for write only: %s\n",fn1);
			
				break;

			case 'x':
				if (optarg != NULL)
				{	
					fn1 = optarg;
					if((f_d[f_dc] = open(fn1, OF_FLAG | O_WRONLY | O_RDONLY, 00666)) == -1)
					{
						if (f_dc != 2){
							fn1 = NULL;
							perror("Cannot write to file\n");
							ERR_FLAG = 1;
						}
						else 
						{
							//fn2 = NULL;
							perror("Standar error file cannot be open\n");
							exit(1);
						}
					}
					OF_FLAG = 0;
					f_dc ++;
				}
				if (debug == 1)
					printf("Filename for read and write: %s\n", fn1);
				break;
			case 'l':
				if (optarg != NULL && isdigit(optarg[0]) && ((optarg[0] - '0') <= f_dc)){
					int f_ind = atoi(optarg);
					if(f_d[f_ind] != -1){
						if (f_dp[f_ind] == 0){
							f_d[f_ind] = -1;
						}
						else{
							//close(f_d[f_ind]);
							//f_d[f_ind] = -1;
							//f_dp[f_ind] = 0;	
						}
					}		
					else 
						perror("This file is not available!\n");
					
				}
				else 
				{
					perror("This file does not exist!\n");
					ERR_FLAG = 1;
				}
				break;
			case 'A':
				raise(SIGSEGV);				
				break;
			case 'T':
				if (optarg != NULL && isdigit(optarg[0])){
					SIGNAL = atoi(optarg);
					signal(SIGNAL, sig_handler);

					if (debug == 2)
						printf("The signal to be catch is: %d\n", SIGNAL);
				}
				else{
					perror("Undfined Signal!\n");
					ERR_FLAG = 1;
				}
				break;
			case 'i':
				if (optarg != NULL && isdigit(optarg[0])){
					SIGNAL = atoi(optarg);
					//SIGNAL_IGNORE = 1;
					signal(SIGNAL, SIG_IGN);
				}
				else{
					perror("Undfined Signal to be ignored!\n");
					ERR_FLAG = 1;
				}
				break;
			case 'f':
				if (optarg != NULL && isdigit(optarg[0])){
					SIGNAL = atoi(optarg);
					//SIGNAL_IGNORE = 2;
					signal(SIGNAL, SIG_DFL);
				}
				else{
					perror("Undfined Signal to be defaulted!\n");
					ERR_FLAG = 1;
				}
				break;
			case 'p':
				while (1)
					pause();	
				break;
			case 'P':
				if (pipe(pipefd) == -1){
					perror("pipe openning fails!\n");
					ERR_FLAG = 1;

					if (debug == 1){
						printf("The pipe fd is: %d %d\n",pipefd[0], pipefd[1]); 
					}
				}
				else {
					f_d[f_dc] = pipefd[0]; //stands for pipe read
					f_dp[f_dc] = -1; //mark for pipe read
					f_dc ++;
					f_d[f_dc] = pipefd[1]; //stands for pipe write
					f_dp[f_dc] = -2; //mark for pipe write 
					f_dc ++;	
				}
				break;
	
			//The following are flags asserting
			case 'v':
				VERBOSE_FLAG = 1;
				break;
			case 'W':
				WAIT_FLAG = 1;
				break;
			case 'a':
				OF_FLAG = OF_FLAG | O_APPEND;
				break;
			case 'C':
				OF_FLAG = OF_FLAG | O_CLOEXEC;
				break;
			case 'R':
				OF_FLAG = OF_FLAG | O_CREAT;
				break;
			case 'd':
				OF_FLAG = OF_FLAG | O_DIRECTORY; 
				break;
			case 'D':
				OF_FLAG = OF_FLAG | O_DSYNC;
				break;
			case 'e':
				OF_FLAG = OF_FLAG | O_EXCL;
				break;
			case 'n':
				OF_FLAG = OF_FLAG | O_NOFOLLOW;
				break;
			case 'N':
				OF_FLAG = OF_FLAG | O_NONBLOCK;
				break;
			case 'S':
				OF_FLAG = OF_FLAG | O_RSYNC;
				break;
			case 's':
				OF_FLAG = OF_FLAG | O_SYNC;
				break;
			case 't':
				OF_FLAG = OF_FLAG | O_TRUNC;
				break;
			case '?':
				strcpy(buffer,"Undefined arguments\n");
				write(2, buffer, strlen(buffer));
				ERR_FLAG = 1;	
				break;
		}
	}
	
	if (debug == 1)
		printf("There is %d sub commands\n", sub_com_c+1);

	//the first sub_com_c is 0
	if (sub_com_c >= 0){
		for (int i = 0; i <= sub_com_c; i++){
			if (debug == 1){
				printf("FD error is: %d\n", FD_ERR[i]); 	
			}
			
			//catch signal while executing sub commands
		
			if (VERBOSE_FLAG){
				int j = 0;
				strcpy(v_buffer,""); //clear the V_buffer
				while(sub_com_table[i][j] != NULL){
					strcat(v_buffer, sub_com_table[i][j]);
					strcat(v_buffer, " ");
					j ++;
				}
				
				write(1,v_buffer, strlen(v_buffer));

				if(debug == 1)
					printf("Verbose output is: %s\n", v_buffer);					
			}
			
			//if sub_command field is empty or not enough file descriptor  
			if (sub_com_table[i][0] == NULL){
				FD_ERR[i] = 4;
				ERR_FLAG = 1;
			}
			
			if (FD_ERR[i] == 0){
				execute(sub_com_table[i], sub_com_fd[i], i);
			}
			else{
				switch(FD_ERR[i]){
				case 1:
					strcpy(buffer, "Incorrect input file descriptor for sub comannd ");
					break;
				case 2:
					strcpy(buffer, "Incorrect output file descriptor for sub command ");	
					break;

				case 3:
					strcpy(buffer, "Incorrect Error msg file descriptor for sub command ");
					break;

				case 4: 
					strcpy(buffer, "Where is sub command ");
					break; 
				
				case 5: 
					strcpy(buffer, "Incorrect access to pipe file descriptor for sub command ");
					break;
				case 6:
					strcpy(buffer, "No command to execute for sub command ");
					break;
				default:
				break;
				}

				//output which "--command" has problem
				char num[2];
				sprintf(num, "%d", i);
				strcat(buffer, num);
				strcat(buffer,"\n");
				write(2, buffer, strlen(buffer));
			 }
		 }
		 //if wait_flag is asserted 
		 if (WAIT_FLAG){
			int i = 0;
			int j = 0; 
			int status;
			
		
			while(child_pid[i] != -1){
				if (debug == 2)
					printf("Child ID to be waited is: %d\n", child_pid[i]);
				
				while(FD_ERR[i] != 0)
					j ++;
				
				i ++;
				waitpid(child_pid[i], &status, 0);//WIFEXITED(status));       // wait for completion
			
				sprintf(buffer, "%d", status);
				strcat(buffer, " ");				

				int k = 0;
				while(sub_com_table[j][k] != NULL){
					strcat(buffer,sub_com_table[j][k]);
					strcat(buffer, " ");
					k ++;
				} 
				
				printf("%s\n", buffer); 
			
			}
			
			for (int m = 0; m < 30; m ++){
				if (f_d[m] != -1)
					if (close(f_d[m]) == -1)
						printf("Cant close fd %d\n", f_d[m]);
			}

		}
	}
	
	exit(ERR_FLAG);
}




