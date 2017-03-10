#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<getopt.h>
#include<time.h>
#include<ctype.h>
#include<string.h>
#include<assert.h>
#include<stdbool.h>
#include"SortedList.h"

int opt_yield = 0;
int opt_sync = 0;

pthread_t *thread_pid;
pthread_mutex_t lock;
volatile int spinlock = 0;

struct thread_argu
{
	int iter_num;
	long long counter;
};

void* iteration_add(void* arg);
void add(long long *pointer, long long value);
void mutex_add(long long *pointer, long long value);
void spin_lock_add(long long *pointer, long long value);
void sync_compare_add(long long *pointer, long long value);

int main(int argc, char *argv[])
{
	//short ERROR_FLAG = 0; //1: Argument nan
	int opt_index = 0;
	int opt = 0;
	int thread_num = 1;
	int iteration_num = 1;
	int rc;
	
	struct thread_argu* Sum_val;
	struct timespec Time1;
	struct timespec Time2;

	char *command_buffer;
	char *buffer;

	SortedList_t *Mylist = NULL;

	Sum_val = malloc(sizeof(struct thread_argu));
	buffer = malloc(sizeof(char)*100);
	command_buffer = malloc(sizeof(char)*100);

	strcpy(command_buffer,"add");

	while(1)
	{
		static struct option long_options[] = {
			{"threads", required_argument, NULL, 'T'},
			{"iterations", required_argument, NULL, 'I'},
			{"yield", required_argument, NULL, 'Y'},
			{"sync", required_argument, NULL, 'S'},
		};

		opt = getopt_long(argc, argv, "T:I:Y:S", long_options, &opt_index);
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
			case 'Y':
				strcat(command_buffer, "-yield=");
				if (optarg) {
					strcat(command_buffer, optarg);
					for (int i=0; optarg[i] != '\0'; ++i) {
						if (optarg[i] == 'i') { opt_yield |= INSERT_YIELD; }
						else if (optarg[i] == 'd') { opt_yield |= DELETE_YIELD; }
						else if (optarg[i] == 's') { opt_yield |= LOOKUP_YIELD; }																				
					}
				}
				break;
			case 'S':
				if (optarg != NULL && isalpha(optarg[0]) && strlen(optarg) == 1){
						strcat(command_buffer, "-");
						strcat(command_buffer, optarg);
						switch (optarg[0]){
							case 'm':
								opt_sync = 1;
								pthread_mutex_init(&lock, NULL);
								break;
							case 's':
								opt_sync = 2;
								break;
							case 'c':
								opt_sync = 3;
								break;
							case '?':
								perror("Undefined --sync argument\n");
								exit(1);
								break;
						}
					}
				else {
					perror("Incorrect --Sync argument!\n");
					exit(1);
				}
				break;
			case '?':
				exit(1);
				break;

		}
	}
	
	//Create threads and do addition and subtraction and record time
	if (!(opt_yield || opt_sync))
		strcat(command_buffer, "-none");

	Sum_val->iter_num = iteration_num;
	assert (clock_gettime(CLOCK_REALTIME, &Time1) ==0);
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

	assert(clock_gettime(CLOCK_REALTIME, &Time2)==0);
   
	long TimePassed = (Time2.tv_sec - Time1.tv_sec) * 1000000000;
	TimePassed += (Time2.tv_nsec - Time1.tv_nsec);
	
	if (opt_sync == 1)
		pthread_mutex_destroy(&lock);

	sprintf(buffer, ",%d,%d,%d,%ld,%ld,%lld\n", thread_num, iteration_num,
			iteration_num*2,TimePassed,TimePassed/(iteration_num*2),
			Sum_val->counter);
	strcat(command_buffer,buffer);
	printf("%s", command_buffer);

	return 0;
}

void* iteration_add(void* arg)
{
	struct thread_argu *th_ar = arg;
	for (int i = 0; i < th_ar->iter_num; i++)
		switch (opt_sync){
			case 0:
				add(&th_ar->counter, 1);
			break;
			case 1:
				mutex_add(&th_ar->counter, 1);
			break;
			case 2:
				spin_lock_add(&th_ar->counter, 1);
			break;
			case 3:
				sync_compare_add(&th_ar->counter, 1);
			break;
		}
	
	for (int i = 0; i < th_ar->iter_num; i++)
		switch (opt_sync){
			case 0:
				add(&th_ar->counter, -1);
			break;
			case 1:
				mutex_add(&th_ar->counter, -1);
			break;
			case 2:
				spin_lock_add(&th_ar->counter, -1);
			break;
			case 3:
				sync_compare_add(&th_ar->counter, -1);
			break;
		}	
	return NULL;
}

void add(long long *pointer, long long value){
		long long sum = *pointer + value;
		if (opt_yield)
			assert(sched_yield() == 0);			
		*pointer = sum;
}

void mutex_add(long long *pointer, long long value){
	assert(pthread_mutex_lock(&lock) == 0);
	long long sum = *pointer + value;
		if (opt_yield)
			assert(sched_yield() == 0);
		*pointer = sum;
	assert(pthread_mutex_unlock(&lock) == 0);
}

void sync_compare_add(long long *pointer, long long value){
	long long sum = 0;
	long long prev = 0;
	do {
		prev = *pointer;
		sum = prev + value;
		if (opt_yield)
			assert(sched_yield() == 0);
	}	
	while (__sync_val_compare_and_swap(pointer, prev, sum) != prev);
}	

void spin_lock_add(long long *pointer, long long value){
	while (__sync_lock_test_and_set(&spinlock, 1));
	long long sum = *pointer + value;
	if (opt_yield)
		assert(sched_yield() == 0);
	*pointer = sum;
	__sync_lock_release(&spinlock);	
}
