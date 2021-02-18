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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>


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
    int fd;
	struct stat file_stat;
	char file_size[256];
	int len, numbytes;
	int sent_bytes = 0;
	
    char* pic_add = "c1.png";
	fd = open(pic_add, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "Error openning file --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fstat(fd, &file_stat) < 0){
		fprintf(stderr, "Error fstat --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "File Size: \n%ld bytes\n", file_stat.st_size);
	sprintf(file_size, "%ld", file_stat.st_size);
	socklen_t sock_len = sizeof(struct sockaddr_in);

	if (len = send(sockfd, file_size, sizeof(file_size), 0) < 0){
		fprintf(stderr, "Error on sending greetings(file size) --> %s", strerror(errno));
        exit(EXIT_FAILURE);
	}
	while (len == 0){
		len = send(sockfd, file_size, sizeof(file_size), 0);
	}
	

	fprintf(stdout, "Client sent %d bytes for the size\n", len);
	off_t offset =0;
	int remain_data = file_stat.st_size;

	while (((sent_bytes = sendfile(sockfd, fd, &offset, MAXDATASIZE)) > 0) && (remain_data > 0)){
        //fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
        remain_data -= sent_bytes;
        fprintf(stdout, "2. Client sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
    }

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
		bzero(buf,MAXDATASIZE-1);
    }
	while (numbytes == 0){
		numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
		bzero(buf,MAXDATASIZE-1);
	}
	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);
	
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

