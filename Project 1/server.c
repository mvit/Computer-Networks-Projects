#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NUM_CONNECTIONS 15
#define BUF_SIZE 2048

// Catching Sigterm code adapted from https://airtower.wordpress.com/2010/06/16/catch-sigterm-exit-gracefully/

volatile sig_atomic_t end = 0;

typedef enum {GET, HEAD} RequestType;

typedef struct _Request {
	RequestType type;
	char file[BUF_SIZE];
	char host[BUF_SIZE];
} Request;

void stop(int signum)
{
	end=1;
}

void usage() {
	fprintf(stderr, "Usage: ./server port_number\n");
	exit(EXIT_FAILURE);
}

bool message_valid(char* msg, size_t msg_size) {
	if (msg[msg_size-4] == '\r' && msg[msg_size-3] == '\n' && msg[msg_size-2] == '\r' && msg[msg_size-1] == '\n') return true;
	return false;
}

void send_bad_request(int socket) {
	write(socket, "HTTP/1.1 400 Bad Request\r\n",26);
	write(socket, "\r\n",2);
}

void send_busy_message(int socket) {
	write(socket,"HTTP/1.1 503 Service Unavailable\r\n", 34);
	write(socket, "\r\n",2);
}

void send_file_not_found(int socket, char* fname) {
	write(socket, "HTTP/1.1 404 File Not Found\r\n",29);
	write(socket, "File:",5);
	write(socket, fname, sizeof(fname));
	write(socket, "\r\n\r\n",4);
	write(socket, "Content-length: 46\n", 19);
	write(socket, "Content-Type: text/html\n\n", 25);
	write(socket, "<html><body><H1>Hello world</H1></body></html>",46);
}

void send_file(int socket, char* fname, RequestType type) {
	
	int fd;
	struct stat stat_buf; 


	
	if ((fd=open(fname, O_RDONLY)) == -1) {
		send_file_not_found(socket, fname);
		return;
	}
	
	write(socket, "HTTP/1.1 200 OK\r\n",17);
	write(socket, "File:",5);
	write(socket,fname,sizeof(fname));
	write(socket, "\r\n",2);
	
	if (type == GET) {
		fstat(fd, &stat_buf);
		sendfile(socket, fd, NULL, stat_buf.st_size);
	}
	
	write(socket, "\r\n\r\n", 4);
}

Request* parse_msg(char* msg, size_t msg_size) {
	
	if (!message_valid(msg, msg_size)) return NULL;
	
	Request *temp_msg = malloc(sizeof(Request));
	
	char type[5];
	sscanf(msg, "%5s [/]%s HTTP/1.1\r\n", type, temp_msg->file);
	printf("%s", temp_msg->file);
	//Get Type
	if(strcmp(type,"HEAD") == 0) {
		temp_msg->type = HEAD;
	}
	else if(strcmp(type,"GET") == 0) {
		temp_msg->type = GET;
	}
	else return NULL;
	
	//Get Host
	
	char* token = strtok(msg, "\r\n");
	bool hostbool = false;
	
	while(token) {
		if(sscanf(token, "Host: %s\r\n", temp_msg->host) == 1) {
			hostbool = true;
			break;
		}
		token = strtok(NULL,"\r\n");
	}
	if (!hostbool) return NULL;
	
	return temp_msg;
}

void *client_handler(void *sock) {
	
	int socket = *((int*) sock);
	char buffer[BUF_SIZE];
	
	size_t sent;
	size_t rcv;
	
	Request *msg;
		
	rcv = recv(socket, buffer, BUF_SIZE, 0);
	printf("CLIENT REQUEST:\n%s\n", buffer);
	
	if (!(msg = parse_msg(buffer, rcv))) send_bad_request(socket);
	
	send_file(socket, msg->file, msg->type);

	close(socket);
	pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
	
	int sock;
	int clientsock;
	
	socklen_t addrsize;
	pthread_t threads[NUM_CONNECTIONS];
	char* port;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_storage clientaddr;
	struct sigaction action;
	
	//Make sure that hints is empty.
	memset(&hints, 0, sizeof hints);
	memset(&action, 0, sizeof(struct sigaction));
	
	//Set values for hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	action.sa_handler = stop;
	
	//Check that correct amount of args was passed
	if (argc != 2) usage();
	
	//Parse Port
	port = argv[1];
	
	//Prepare the Socket
	if (getaddrinfo(NULL, port, &hints, &servinfo) < 0) {
		fprintf(stderr, "ERROR: getAddrInfo() returned with Status: %s\n",gai_strerror(errno));
		exit(EXIT_FAILURE);
	} 
	
	//Create the Socket
	if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
		fprintf(stderr, "ERROR: socket() returned File Descriptor: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	//Bind to Socket
	if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
		fprintf(stderr, "ERROR: bind() returned value of %i\n", errno);
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	if (listen(sock, NUM_CONNECTIONS) < 0) {
		fprintf(stderr, "ERROR: listen() returned value of %i\n", errno);
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	sigaction(SIGTERM, &action, NULL);	
	
	while(!end) {
		int i = 0;
		addrsize = sizeof clientaddr;
		clientsock = accept(sock, (struct sockaddr *) &clientaddr, &addrsize);
		while(threads[i] != 0) i++;
		if (i < NUM_CONNECTIONS) pthread_create(&threads[i], NULL, client_handler, (void *) &clientsock);
		else send_busy_message(clientsock);
	}
	
	close(sock);
	printf("Server terminated\n");
	return 0;
}
