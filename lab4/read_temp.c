#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<mraa/aio.h>
#include<math.h>
#include<time.h>
#include<string.h>
#include "client.h"

sig_atomic_t volatile run_flag = 1;
int volatile send_flag = 0; 
int volatile period = 3;
char volatile F_C = 'F';

time_t my_time;
struct tm * timeinfo;


void IRS(int sig)
{
	//if (sig == SIGINT)
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

void command_handle(char* buffer)
{
	if (strcmp(buffer, "START") == 0)
		send_flag = 1;
	else if (strcmp(buffer, "STOP") == 0)
		send_flag = 0;
		
}

int client_handle_connection(int socket_fd)
{
	uint16_t value;
	float F, C;
	char buffer[256];
	int valid_com = 1;
	FILE * fp;

	fp = fopen("log.csv", "w+");
	if ((write(socket_fd, "504135743", 10))<0)
		return client_error("Error writting socket!\n");
	printf("Sent UID\n");

	while(run_flag)
	{
		if ((read(socket_fd, buffer, strlen(buffer)))<0)
			return client_error("Error reading socket!\n");
		printf("%s\n",buffer);
		
		valid_com = 1;

		if (strcmp(buffer, "START") == 0)
			send_flag = 1;
		else if (strcmp(buffer, "STOP") == 0)
			send_flag = 0;
		else if (strcmp(buffer, "ON") == 0)
			run_flag = 1;
		else if (strcmp(buffer, "OFF") == 0)
			run_flag = 0;
		else if ((strcmp(buffer, "SCALE=F") == 0) ||
				(strcmp(buffer, "SCALE=C") == 0))
			F_C = buffer[6];
		else if (strncmp(buffer, "PERIOD=", 7) ==0 )
		{
			char * tmp = buffer;
			tmp += 7;
			int val = atoi(tmp);
			if (val == 0)
			{
				printf("not a number!");
				valid_com = 0;				
			}
			else 
				period = val;
		}
		else 
			valid_com = 0;
		
		if (valid_com)
			fprintf(fp, "504135743: %s", buffer);
		else 
			fprintf(fp, "504135743: %sI", buffer);	
	}
	fclose(fp);	
	return 0;
}

int handle_temp()
{	
	char buffer[256];
	mraa_aio_context temp;
	temp = mraa_aio_init(0);
	if (send_flag)
		{
			value = mraa_aio_read(temp);
			C = temp_convert(value);
			F = C_to_F(C);
			time (&my_time);
			timeinfo = localtime(&my_time);
			memset(buffer, 0, 256);
		
			if (F_C == 'F')
				sprintf(buffer, "504135743: 
						%d:%d:%d %2.1f.\n",
					timeinfo->tm_hour,timeinfo->tm_min,
					timeinfo->tm_sec, F);
			else
				sprintf(buffer, "504135743: 
						%d:%d:%d %2.1f.\n",
					timeinfo->tm_hour,timeinfo->tm_min,
					timeinfo->tm_sec, C;
			
			printf("%s\n", buffer);			
			if ((write(socket_fd, buffer, strlen(buffer))) < 0)  					return client_error(
						"Error writing socket!\n");
			sleep(period);
		}
	}
	mraa_aio_close(temp);
	return 0;
}

int main(int argc, char *argv[])
{	
	int client_socket_fd;
	signal(SIGINT,IRS);

	client_socket_fd = client_init(argc, argv);
	if (client_socket_fd < 0)
		return -1;

	client_handle_connection(client_socket_fd);
	return 0;
}
