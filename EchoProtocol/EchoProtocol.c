/*
 * A server and client that implement the fictitious "echo protocol".
 * The server accept any string from the client, and then return that
 * string with all letters capitalized (if letters exist).
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;					// mutex lock
pthread_cond_t client_condition = PTHREAD_COND_INITIALIZER;		// client condition variable


struct sockaddr_in serverAddr;
int start = 0;

void *client(void *param);
void *server(void *param);

int main(int argc, char *argv[]) {

	pthread_t tid1, tid2;  // thread identifiers

	// Configure settings of the server address struct
	// Address family = Internet
	serverAddr.sin_family = AF_INET;
	// Set port number, using htons function to use proper byte order
	serverAddr.sin_port = htons(7891);
	// Set IP address to localhost
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// Set all bits of the padding field to 0
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	// create the server thread
	if (pthread_create(&tid1, NULL, server, NULL) != 0) {
		fprintf(stderr, "Unable to create server thread\n");
		exit(1);
	}

	// create the client thread
	if (pthread_create(&tid2, NULL, client, NULL) != 0) {
		fprintf(stderr, "Unable to create client thread\n");
		exit(1);
	}

	// wait for created thread to exit
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf("Parent quitting\n");

	return 0;
}

void *server(void *param) {

	int welcomeSocket, newSocket;
	char buffer[1024];
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	pthread_mutex_lock(&m);
		// Create the socket
		welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

		// Bind the address struct to the socket
		bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
		start = 1;

		// Listen on the socket, with 5 max connection requests queued
		if (listen(welcomeSocket, 5) == 0) {
			printf("Listening\n");
		} else {
			printf("Error\n");
		}

	pthread_mutex_unlock(&m);
	pthread_cond_signal(&client_condition);

	// Accept call creates a new socket for the incoming connection
	addr_size = sizeof serverStorage;
	newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

	// Read the message from the server into the buffer
	recv(newSocket, buffer, 1024, 0);
	strupp(buffer);
	printf("Data received: %s\n", buffer);

	return 0;
}

void strupp(char* beg) {
    while (*(++beg) = toupper(*beg)) {}
}


void *client(void *param) {

	pthread_mutex_lock(&m);
	while (start != 1) {
		pthread_cond_wait(&client_condition, &m);
	}
	pthread_mutex_unlock(&m);

	int clientSocket;
	char buffer[1024];
	socklen_t addr_size;

	// Create the socket
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	// Connect the socket to the server using the address struct
	addr_size = sizeof serverAddr;
	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

	// Send message to the server
	strcpy(buffer,"Hello World\n");
	send(clientSocket, buffer, 13, 0);

	return 0;
}
