#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<fcntl.h>
#include<string.h>
#include<getopt.h>
#include<stdlib.h>

void sig_handler(int signo)
{
	if (signo == SIGSEGV)
		{
			perror("Seg Fault detects!\n");
			exit(3);
		}
}

int main(int argc, char *argv[])
{
	int opt_index = 0;
	char* fd0 = NULL;
	char* fd1 = NULL;
	int f_d0 = 0;
	int f_d1 = 1;
	char buffer; 
	int seg = 0;
	int size, opt = 0;
	char* dummy = NULL;

	while(1)
	{
		static struct option long_options[] = {
			{"input", required_argument, NULL, 'i'},	
			{"output", required_argument, NULL, 'o'},
			{"catch", no_argument, NULL, 'c'},
			{"segfault", no_argument, NULL, 's'},
			{0,0,0,0},
		};
		
		opt = getopt_long(argc, argv, "i:o:sc", long_options, &opt_index);
		if (opt == -1)
			break;

		switch(opt) {
			case 'i':
				fd0 = optarg;
				printf("%s\n",fd0);
				break;
			case 'o':
				fd1 = optarg;
				printf("%s\n",fd1);
				break;
			case 'c':
				signal(SIGSEGV, sig_handler);
				break;
			case 's':
				seg = 1;
				break;
			case '?':
				perror("Undefined arguments!\n");
				return 1;
				break;
		}
	} 
	if (fd0 != NULL)
		if((f_d0 = open(fd0, O_RDONLY, 00666)) == -1)
		{
			perror("Error: unable to open the input file\n");
			exit(1);
		}
	if (fd1 != NULL)
		if((f_d1 = open(fd1, O_CREAT | O_WRONLY, 00666)) == -1)
		{
			perror("Error: unable to open the output file\n");
			exit(2);
		}

	if (seg)
		{
			raise(SIGSEGV);
			//if (f_d0 >= 0)
			//	while((size = read(f_d0, dummy, 1)) != EOF)
			//		write(f_d1, dummy, size);
		}
	else 
		{
			if (f_d0 >= 0)
				//printf("%d\n",f_d0);
				while((size = read(f_d0, &buffer, 1)) == 1)
						write(f_d1, &buffer, 1);
		}
	
	if (f_d0 > 0)
		{
			close(0);
			dup(f_d0);
			close(f_d0);
		}
	if (f_d1 > 0)
		{
			close(1);
			dup(f_d1);
			close(f_d1);
		}

	return 0;

}




