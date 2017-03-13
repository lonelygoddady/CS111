#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "server.h"

#define PORTNO 5000

static volatile int run_flag = 1;

void ISR(int signal)
{
	if (signal == SIGINT)
		run_flag = 0;
}

void handle_client(CONNECTION* client)
{
	char buffer[256];
	int n; 
	int client_socket_fd = client->sockfd;

	memset(buffer, 0, 256);
	sprintf(buffer, "Client IP is %s", client->ip_addr_str);
	printf("%s", buffer);

	while (run_flag)
	{
		memset(buffer, 0, 256);

		// read what the client sent to the server and store it in "buffer"
		n = read(client_socket_fd, buffer, 255);	
		// an error has occurred
		if (n < 0) {
			server_error("ERROR reading from socket");
			return;
		}
		// no data was sent, assume the connection was terminated
		if (n == 0) { 
			printf("%s has terminated the connection.\n", client->ip_addr_str);
			return;
		}

		// print the message to console
		printf("%s says: %s\n", client->ip_addr_str, buffer);
	}
}

int main(int argc, char *argv[])
{
	CONNECTION *server;
	CONNECTION *client;

	// server = (CONNECTION*)server_init(PORTNO, 10);
	server = server_init(PORTNO, 10);
	
	if ((int) server == -1){
		run_flag = 0;
	}

	while(run_flag) {
		// client = (CONNECTION*)server_accept_connection(server->sockfd);
		client = server_accept_connection(server->sockfd);
		printf("%d\n",(int)client);
		if ((int) client == -1) {
			printf("Waiting for an incoming client connection.\n");
		}
		else {
			handle_client(client);
		}
	}

	return 0;
}