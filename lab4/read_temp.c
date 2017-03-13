#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<mraa/aio.h>
#include<math.h>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include "client.h"

sig_atomic_t volatile run_flag = 1;
int volatile send_flag = 1; 
int volatile period = 3;
char volatile F_C = 'F';
FILE * fp;

time_t my_time;
struct tm * timeinfo;

void IRS(int sig)
{
	run_flag = 0;
	exit(0);
}

float temp_convert(uint16_t temp)
{
	float R = 1023.0/((float)temp) - 1.0;
	R = 100000.0 * R;
	float temp_v = 1.0/(log(R/100000.0)/4275+1/298.15)-273.15;
	return temp_v;
}

float C_to_F(float F)
{
	return (F*1.8) + 32.0; 
}

int command_handle(char* buffer)
{
	if (strcmp(buffer, "START") == 0)
		send_flag = 1;
	else if (strcmp(buffer, "STOP") == 0)
		send_flag = 0;
	else if (strcmp(buffer, "ON") == 0)
		run_flag = 1;
	else if (strcmp(buffer, "OFF") == 0)
		run_flag = 0;
	else if ((strcmp(buffer, "SCALE=F") == 0) || (strcmp(buffer, "SCALE=C") == 0))
		F_C = buffer[6];
	else if (strncmp(buffer, "PERIOD=", 7) ==0 )
	{
		char * tmp = buffer;
		tmp += 7;
		int val = atoi(tmp);
		if (val == 0)
		{
			printf("not a number!\n");
			return 0;				
		}
		else 
			period = val;
	}
	else 
	{
		if (strlen(buffer) != 0){
			printf("detect invalid command %s\n", buffer);
			return 0;
		}
	}
	if (strlen(buffer) > 2)
		printf("Command: %s\n", buffer);

	return 1;
}

void command_receive(void* sk_fd)
{
	uint16_t value;
	float F, C;
	char buffer[256];
	int valid_com = 1;
	int socket_fd = *((int*)sk_fd);
	time (&my_time);

	while(run_flag)
	{
		memset(buffer, 0, 256);
		if ((read(socket_fd, buffer, strlen(buffer)))<0)
			exit(client_error("Error reading socket!\n"));
		
		valid_com = command_handle(buffer);
		timeinfo = localtime(&my_time);

		if (valid_com)
			fprintf(fp, "%d:%d:%d %s\n", timeinfo->tm_hour, 
				timeinfo->tm_min, timeinfo->tm_sec, buffer);
		else 
			fprintf(fp, "%d:%d:%d %sI\n", timeinfo->tm_hour, 
				timeinfo->tm_min, timeinfo->tm_sec, buffer);
	}
}

void send_msg(void* sk_fd)
{
	uint16_t value;
	int socket_fd = *((int*)sk_fd);
	char buffer[256];
	float F, C;
	time (&my_time);

	mraa_aio_context temp;
	temp = mraa_aio_init(0);

	while(run_flag)
	{
		if (send_flag)
		{
			value = mraa_aio_read(temp);
			C = temp_convert(value);
			F = C_to_F(C);
			timeinfo = localtime(&my_time);
			memset(buffer, 0, 256);
		
			if (F_C == 'F'){
				// sprintf(buffer, "[504135743] 
				// 		%d:%d:%d %2.1f.\n",
				// 	timeinfo->tm_hour,timeinfo->tm_min,
				// 	timeinfo->tm_sec, F);
				sprintf(buffer, "504135743 TEMP=%2.1f.\n",F);
			}
			else{
				// sprintf(buffer, "504135743: %d:%d:%d %2.1f.\n",
				// 	timeinfo->tm_hour,timeinfo->tm_min,
				// 	timeinfo->tm_sec, C);
				sprintf(buffer, "504135743 TEMP=%2.1f.\n",C);
			}

			
			if ((write(socket_fd, buffer, strlen(buffer))) < 0)			
				exit(client_error("Error writing socket!\n"));

			printf("%s\n", buffer);	
			fprintf(fp, "%d:%d:%d %s\n", timeinfo->tm_hour, 
				timeinfo->tm_min, timeinfo->tm_sec, buffer);
			
			sleep(period);
		}
	}

	mraa_aio_close(temp);
}

int main(int argc, char *argv[])
{	
	int client_socket_fd;
	pthread_t send_tid, receive_tid;
	signal(SIGINT,IRS);

	client_socket_fd = client_init(argc, argv);
	if (client_socket_fd < 0)
		return -1;

	fp = fopen("log.csv", "w+");

	if (pthread_create(&send_tid, NULL, (void *) &send_msg, (void *) &client_socket_fd) != 0){
		fprintf(stderr, "Failed to create msg send thread. Exiting program.\n");
		exit(0);
	}

	if (pthread_create(&receive_tid, NULL, (void *) &command_receive, (void *) &client_socket_fd) != 0){
		fprintf(stderr, "Failed to create msg receive thread. Exiting program.\n");
		exit(0);
	}

	pthread_join(send_tid, NULL);
	pthread_join(receive_tid, NULL);

	fclose(fp);
	return 0;
}
