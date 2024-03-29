/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define PORT "3491"  // the port users will be connecting to
#define BUFFSIZE 256
#define BACKLOG 10	 // how many pending connections queue will hold
//#define _POSIX_C_SOURCE

int callScript(void){
	char *cmd = "/home/pi/3.7venv/bin/python3 /home/pi/smile_detector.py \"/home/pi/s2.mp4\"";
    return WEXITSTATUS(system(cmd));
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;	
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv, len;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// servinfo now points to a linked list of 1 or more struct addrinfos

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
 		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		
		
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr), // using the function we made to handeling both ipv4 and ipv6
			s, sizeof s); // destination ip to presentation
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			char p_array[BUFFSIZE];
			int recv_bytes;
			char ending_char[BUFFSIZE];
			snprintf( ending_char, BUFFSIZE, "%d", 1);

			int nb;
			close(sockfd); // child doesn't need the listener
			while(1){
				FILE *image = fopen("s2.mp4", "w");
				if (image == NULL){
					fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
				if ((nb = recv(new_fd, p_array, BUFFSIZE, 0)) == -1){
					perror("recv");
					exit(1);
					bzero(p_array,BUFFSIZE);
				}
				while (nb == 0){
					nb = recv(new_fd, p_array, BUFFSIZE, 0);
					bzero(p_array,BUFFSIZE);
				}
				
				int file_size = atoi(p_array);
				
				int remain_data = file_size;
				
				printf("File size is %d bytes\n", remain_data);
				if (file_size == 1){
					// closing the connection
					printf("server 2 received ending message: closing the connection\n");
					break; //terminating the child process
				}
				//printf("%d\n", nb);
				bzero(p_array,BUFFSIZE);
				
				while ((remain_data > 0) && ((nb = recv(new_fd, p_array, BUFFSIZE, 0)) > 0)) {
					fwrite(p_array, sizeof(char), nb, image);
					remain_data -= nb;
					fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", nb, remain_data);
					bzero(p_array,BUFFSIZE);
				}
				
			
				fclose(image);
				int code = callScript();
				len = snprintf(NULL, 0, "%d", code);
    			char* retStatus = malloc(len+1);
				snprintf(retStatus, len+1, "%d", code);


				if (len = send(new_fd, retStatus, len, 0) < 0){
					fprintf(stderr, "Error on sending response --> %s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
				free(retStatus);
			}
			close(new_fd); // closing the connection
			printf("server: waiting for new connections...\n");
			exit(0); //terminating the child process

		}

		close(new_fd);  // parent doesn't need this
		
	}

	return 0;
}

