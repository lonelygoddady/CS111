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

int debug = 0;
int wait_flag = 0;

void sig_handler(int signo)
{
	if (signo == SIGSEGV)
		{
			perror("Seg Fault detects!\n");
			exit(3);
		}
}

void  execute(char **argv, int* sub_com_fd, int FD_ERR, char* buffer)
{
     pid_t  pid;
     int    status;

     if ((pid = fork()) < 0) {     // fork a child process           
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {          // for the child process:  
	  
	  //change file descriptor of the sub_command
	  dup2(sub_com_fd[0],0);
	  dup2(sub_com_fd[1],1);
	  dup2(sub_com_fd[2],2);
 
          if (execvp(argv[0], argv) < 0) {     // execute the command  
               if (debug)
	        	printf("*** ERROR: exec failed\n");
              
	        exit(1);
          }
	  else{
	        //after sub_com execution, check error flag
		
       	 }
	if (debug) 
		printf("Processing sub_com...\n");
	 
     }
     else {                                  // for the parent:      
          if (debug)
		printf("waiting for completion\n");
	 
	  if(wait_flag)
	 	 while (wait(&status) != pid)       // wait for completion  
               		continue;
	 
	  if (debug)
		 printf("Sub_com finish\n");
     }
}

int main(int argc, char *argv[])
{
	int opt_index = 0;
	int nargc = 0;
	char* fn0 = NULL;
	char* fn1 = NULL;
	//char* fn2 = NULL;
	int f_d[30]; //fd array that stores all fds
	int f_dc = 0;
	//int seg = 0;
	int opt = 0;
	int err_c = 0;

	int sub_com_c = 0; //count how many sub command are there
	int sub_com_fd[10][3]; //2D int array to store sub_command file descriptors, support at most 10 commands	

	int verbose_f = 0;
	int FD_ERR[10];  //File descriptor error array

	char* buffer = NULL;
	buffer = (char*)malloc(100*sizeof(char));

	char* v_buffer = NULL;
	v_buffer = (char*)malloc(100*sizeof(char));

	char** sub_com;
	sub_com = malloc(sizeof(sub_com) * 10);
	 	
 	//default stdout stdin stderr
	f_d[0] = 0;
	f_d[1] = 1;
	f_d[2] = 2;
	
	for (int i = 3; i< 30; i++)
		f_d[i] = -1;
	
	//initialize sub_com_fd
	for (int i = 0; i< 10; i++){
		sub_com_fd[i][0] = 0;
		sub_com_fd[i][1] = 1;
		sub_com_fd[i][2] = 2; 
	}
	
	while(1)
	{
		static struct option long_options[] = {
			{"command", required_argument, NULL, 'c'},	
			{"rdonly", required_argument, NULL, 'r'},
			{"wronly", required_argument, NULL, 'w'},
			{"verbose", no_argument, NULL, 'v'},
			{0,0,0,0},
		};
		
		opt = getopt_long(argc, argv, "-c:r:w:v", long_options, &opt_index);
		if (opt == -1)
			break;

		switch(opt) {
			case 1:
				nargc ++;
				if (debug)
					printf("stdin stdout and stderr fd is: %d,%d,%d\n", f_d[0],f_d[1],f_d[2]);

				switch (nargc) {
					case 1:
						if (optarg != NULL && isdigit(optarg[0])){
							//f_d[1] = optarg[0] - '0';
							//dup2(f_d[1],(optarg[0] - '0'));
							int f_ind = optarg[0] - '0';
							if (debug){
								printf("f_ind for stdout is: %d\n", f_ind);	
								printf("f_ind should be: %s\n", optarg);
								printf("the fd is: %d\n", f_d[f_ind]);	
							}

							if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 19){
								//strcpy(buffer,"Incorrect file descriptor\n");
								//write(1, buffer, strlen(buffer));
								FD_ERR[sub_com_c-1] = 2;
								//f_d[1] = 1;
							}
							else{
								sub_com_fd[sub_com_c-1][1] = f_d[f_ind];
								//dup2(f_d[f_ind],1);
							}
						} 							
	
						if (debug) 
							printf("stdout fd is %d\n",f_d[1]);
						break;
					case 2:
						if (optarg != NULL && isdigit(optarg[0])){
							//f_d[2] = optarg[0] - '0';
							//dup2(f_d[2],(optarg[0] - '0'));
							int f_ind = optarg[0] - '0';
							if (debug){
								printf("f_ind for stderr is: %d\n", f_ind);
								printf("f_ind should be: %s\n", optarg); 
								printf("the fd is: %d\n", f_d[f_ind]);					
								printf("Stderr fd is: %d\n", f_d[2]);
							}
	
							if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 19){
								//strcpy(buffer,"Incorrect file descriptor\n");
								//write(2, buffer, strlen(buffer));
								FD_ERR[sub_com_c-1] = 3;
								//f_d[2] = 2;

								if (debug){
									printf("output stderr msg\n");
									printf("the stderr fd is: %d\n", f_d[2]);	
								}
							}
							else{ 
								//dup2(f_d[f_ind],2);
								sub_com_fd[sub_com_c-1][2] = f_d[f_ind];
							}
						}

						if (debug) 
							printf("stderr fd is: %d\n",f_d[2]);
						break;
					case 3:
						if (optarg != NULL)
							sub_com[0] = optarg;						
						break;
					default:
						if (optarg != NULL)
							sub_com[nargc-3] = optarg;
						break;
				}

				if (nargc >= 3){
					if (debug){
						for(int i = 0; i <= nargc - 3; i++)
							printf("%s\n", sub_com[i]);
					}
					if (verbose_f){
						strcpy(v_buffer, sub_com[0]);
						for(int i = 1; i <= nargc - 3; i++){
							strcat(v_buffer, " ");
							strcat(v_buffer, sub_com[i]);
						}
					}
					//execute(sub_com);
				
				}
				else if(nargc >0) 
				{
					err_c = 1;
				}
				break;
			case 'c':
				if (optarg != NULL && isdigit(optarg[0])){
					int f_ind = optarg[0] - '0';
					if (f_ind > f_dc || f_d[f_ind] == -1 || f_ind > 19){
						//write(f_d[2], buffer, strlen(buffer));
						FD_ERR[sub_com_c-1] = 1;	
					}
					else{ 
				 		sub_com_c ++;	
						sub_com_fd[sub_com_c-1][0] = f_d[f_ind];
					}
				}
				else 
					err_c = 1; //for exit status 

				if (debug) 
					printf("%d\n",f_d[0]);

				break;
			case 'r':
				if (optarg != NULL)
				{	
					fn0 = optarg;
					if((f_d[f_dc] = open(fn0, O_RDONLY, 00666)) == -1)
					{
						fn0 = NULL;
						perror("Cannot open file to read\n");
						err_c = 1;						
					}
						
					f_dc ++; //keep track the number of file descriptor used
				}

				if (debug) 
					printf("%s\n",fn0);

			break;
			case 'w':
				if (optarg != NULL)
				{	
					fn1 = optarg;
					if((f_d[f_dc] = open(fn1, O_WRONLY, 00666)) == -1)
					{
						if (f_dc != 2){
							fn1 = NULL;
							perror("Cannot write to file\n");
							err_c = 1;
						}
						else 
						{
							//fn2 = NULL;
							perror("Standar error file cannot be open\n");
							exit(1);
						}
					}
					f_dc ++;
				}
			
				if (debug)
					printf("%s\n",fn1);
			
				break;
			case 'v':
				verbose_f = 1;
				break;
			case '?':
				strcpy(buffer,"Undefined arguments\n");
				write(2, buffer, strlen(buffer));
				break;
		}
	}
	
	if (sub_com_c > 0){
		if (verbose_f){
			strcat(v_buffer, "\n");

			write(1,v_buffer, strlen(v_buffer));
			
			if(debug)
				printf("Verbose output is: %s\n", v_buffer);					
		}

		for (int i = 0; i < sub_com_c; i++){
			if (debug){
				printf("FD error is: %d\n", FD_ERR[i]); 	
			}
			execute(sub_com, sub_com_fd[i], FD_ERR[i], buffer);	
			
			char num[2];

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
			
			default:
			break;
		}
		
		if(FD_ERR[i] == 1 || FD_ERR[i] == 2 || FD_ERR[i] == 3){ 
			sprintf(num, "%d", i);
			strcat(buffer, num);
			strcat(buffer,"\n");
			write(2, buffer, strlen(buffer));
		}
	     }
	}
	
	exit(err_c);
}




