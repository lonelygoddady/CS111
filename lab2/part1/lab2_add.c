#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<getopt.h>
#include<time.h>
#include<ctype.h>

struct thread_argu
{
	int iter_num;
	long long counter;
};

void* iteration_add(void* arg);

int main(int argc, char *argv[])
{
	//short ERROR_FLAG = 0; //1: Argument nan
	int opt_index = 0;
	int opt = 0;
	int thread_num = 0;
	int iteration_num = 0;
	int rc;
	
	pthread_t *thread_pid;
	struct thread_argu* Sum_val;
	struct timespec Time1;
	struct timespec Time2;

	Sum_val = malloc(sizeof(struct thread_argu));

	while(1)
	{
		static struct option long_options[] = {
			{"threads", required_argument, NULL, 'T'},
			{"iterations", required_argument, NULL, 'I'},
		};

		opt = getopt_long(argc, argv, "T:I:", long_options, &opt_index);
		if (opt == -1)
			break;
		
		switch(opt){
			case 'T':
				if (optarg != NULL && isdigit(optarg[0])){
					thread_num = atoi(optarg);
					thread_pid = malloc(sizeof(pthread_t)*thread_num);
				}
				else {
					perror("Threads number option argument is nan\n");
					exit(1);
				}
				break;

			case 'I':
				if (optarg != NULL && isdigit(optarg[0])){
					iteration_num = atoi(optarg);
				}
				else {
					perror("Iteration number option argument is nan\n");
					exit(1);
				}
				break;

			case '?':
				exit(1);
				break;

		}
	}
	
	//Create threads and do addition and subtraction and record time
	Sum_val->iter_num = iteration_num;
	if (clock_gettime(CLOCK_REALTIME, &Time1)!=0)
	{
		perror("Gettime fails!\n");
		exit(3);
	}
	for (int i = 0; i < thread_num; i++)
	{
		rc = pthread_create(&thread_pid[i],NULL,iteration_add,(void*)Sum_val);	
		if (rc != 0)
		{
			perror("failed to create threads!\n");
			exit(2);
		}
	}
	for (int i = 0; i < thread_num; i++)
	{
		pthread_join(thread_pid[i],NULL);	
	}

	if (clock_gettime(CLOCK_REALTIME, &Time2)!=0)
	{
		perror("Gettiom fails!\n");
		exit(3);
	}
	Time2.tv_sec -= Time1.tv_sec;
	Time2.tv_nsec -= Time1.tv_nsec;
	printf("It takes: %ld second and %ld nanosecend\n", Time2.tv_sec, Time2.tv_nsec);
	printf("The counter value is: %lld\n", Sum_val->counter); 

	return 0;
}

void* iteration_add(void* arg)
{
	struct thread_argu *th_ar = arg;
	for (int i = 0; i < th_ar->iter_num; i++)
		th_ar->counter ++; 
	
	for (int i = 0; i < th_ar->iter_num; i++)
		th_ar->counter --;	
	
	return NULL;
}


