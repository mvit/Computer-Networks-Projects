#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void usage() {
	fprintf(stderr, "Usage: ./client [-pv] server_url port_number\n");
	exit(EXIT_FAILURE);
}

int main (int argc, char ** argv) {
		
	int opt;	
	//int status;
	int sock;
	
	long nsec;
	
	char* port;
	char* host;
	char* path;
	char* url;
	char* filename;
	char msg[2048];
	char buffer[2048];
	
	size_t len;
	size_t sent;
	size_t rcv;
	
	struct timespec spec1;
	struct timespec spec2;
	
	struct addrinfo hints;
	struct addrinfo *servinfo;
	
	//Make sure that hints is empty.
	memset(&hints, 0, sizeof hints);
	
	//Set values for hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//Boolean for RTT output
	bool getRTT = false;
	//Boolean for verbose output.
	bool verbose = false;
	
	//Process Options and Arguments
	if (argc==1) usage ();
	
	while ((opt = getopt(argc, argv, "pv")) != -1) {
		switch (opt) {
			case 'p': getRTT = true; break;
			case 'v': verbose = true; break;
			default:
				usage();
		}
	}
			
	if ((argc - optind) != 2) usage();
	
	//Parse URL
	url = argv[optind];
	if (path = strchr(url, '/')) {
		host = calloc(path - url + 1, sizeof(char));
		strncpy(host, url, path - url);
	}
	else {
		path = "/";
		host = url;
	}
	
	if (verbose) printf("URL: %s\n", url);
	if (verbose) printf("HOST: %s\n", host);
	if (verbose) printf("PATH: %s\n", path);
	//Parse Port
	port = argv[optind + 1];
	if (verbose) printf("PORT: %s\n", port);
	
	//Start TCP Socket
	if (verbose) printf("Creating TCP Socket\n");

	//Prepare the Socket
	if ((getaddrinfo(host, port, &hints, &servinfo) < 0)) {
		fprintf(stderr, "ERROR: GetAddrInfo returned with Status: %s\n",gai_strerror(errno));
		exit(EXIT_FAILURE);
	} 
	if (verbose) printf("AddrInfo populated with Status: %s\n",gai_strerror(errno));
	
	//Create the Socket
	if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
		fprintf(stderr, "ERROR: Socket() returned File Descriptor: %d\n", sock);
		exit(EXIT_FAILURE);
	}
	if (verbose) printf("Socket Created\n");
	
	//Connect with specified socket
	if (verbose) printf("Starting connection to host at %s:%s\n", host, port);
	
	clock_gettime(CLOCK_REALTIME, &spec1);
	
	if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
		fprintf(stderr, "ERROR: connect returned value of %i\n", errno);
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	clock_gettime(CLOCK_REALTIME, &spec2);
	
	if (verbose) printf("Connection Successful!\n");
	
	//Compose GET Message
	snprintf(msg, 2048,"GET %s HTTP/1.1\r\nHost: %s:%s\r\nAccept:text/html\r\nKeep-Alive:115\r\nConnection: keep-alive\r\n\r\n",path, host, port);
	len = strlen(msg);
	
	//Send GET Message
	if (verbose) printf("Sending GET Message of Content:\n%s\n", msg);
	sent = send(sock, msg, len, 0);
	if (verbose) printf("Message sent with size: %zu\n", sent);
	
	//Fetch Response
	if (verbose) printf("Awaiting Response from host at %s:%s\n", host, port);
	rcv = recv(sock, buffer, 2048, 0);
	printf("%s\n", buffer);
	
	//Close the Socket
	if (verbose) printf("Closing TCP Socket\n");
	close(sock);
	
	nsec = spec2.tv_nsec - spec1.tv_nsec;
	if (getRTT) printf("RTT Time: %ld milliseconds \n", nsec / 1000000);
	return 0;
}

