/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>



#include <arpa/inet.h>
//#define _POSIX_C_SOURCE
#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 512 // max number of bytes we can get at once 
char cwd[256];
char buf[MAXDATASIZE];
// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void send_image(int sockfd){
    /**
	int length = snprintf(NULL, 0, "/%d.png", picturenth);
	char* pic_num = malloc( length + 1 );
	snprintf(pic_num, length + 1, "/%d.jpg", picturenth);
	char* pic_add = malloc(strlen(cwd)+10);
	strcpy(pic_add, cwd);
    strcat(pic_add, pic_num);
    **/
    char* pic_add = "c1.png";

	FILE *picture;
	picture = fopen(pic_add, "r");
	//int size;
	//fseek(picture, 0, SEEK_END);
	//size = ftell(picture);
	int nb = fread(buf, 1, sizeof(buf), picture);
    printf("%d\n", nb);
	while(!feof(picture)){
		send(sockfd, buf, nb, 0);
		nb = fread(buf, 1, sizeof(buf), picture);
        printf("%d\n", nb);
	}
    send(sockfd, buf, nb, 0);
	//free(pic_num);
	//free(pic_add);
}

int main(int argc, char *argv[])
{
	int picNum;
	int sockfd, numbytes;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (getcwd(cwd, sizeof(cwd)) == NULL)
		perror("getcwd() error");

	if (argc != 2) {
	    fprintf(stderr,"usage: client arguments\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	
	send_image(sockfd);
	

	close(sockfd);

	return 0;
}

