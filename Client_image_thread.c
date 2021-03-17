/*
** Client.c -- a stream socket Client demo
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
#include <pthread.h>
#include <stdbool.h>


//#define PORT "3490" // the port Client will be connecting to 
#define MAXDATASIZE 2048 // max number of bytes we can get at once 

struct arg_struct{
	int socketfd;
	int numOfPic;
	bool th_end_flag;
	int num_of_server;
	int *output;
};

void saving_results(int *results, int numofpics){
	FILE *txt = fopen("results.txt", "w");
	if (txt==NULL){
		printf("Client : Error opening results.txt file!\n");
		exit(1);
	}
	for(int i=1; i < numofpics+1; i++){
		if(results[i]==0){
			fprintf(txt, "%d. Smile detected\n", i);
		}
		else if(results[i]==1){
			fprintf(txt, "%d. No face detected\n", i);
		}
		else if(results[i]==2){
			fprintf(txt, "%d. No smile detected\n", i);
		}
		else{
			fprintf(txt, "%d. There has been a problem with this file\n", i);
		}
		

	}
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* close_servers(void* arguments){
	struct arg_struct *args = (struct arg_struct *)arguments;
	int thsocketfd = args-> socketfd;
	int len;
	char ending_char[1024];
	off_t ending_size = 1;
	sprintf(ending_char, "%ld", ending_size);	

	if (len = send(thsocketfd, ending_char, sizeof(ending_char), 0) < 0){
		fprintf(stderr, "Client: Error on sending ending message --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
	}
	args->th_end_flag = true;
}

void *send_video(void* arguments){
	char buf[MAXDATASIZE], file_size[1024];
    int fd, len, numbytes, sent_bytes = 0;
	struct stat file_stat;
	int thsocketfd;
	
	struct arg_struct *args = (struct arg_struct *)arguments;

	thsocketfd = args-> socketfd;

	len = snprintf(NULL, 0, "%d", args -> numOfPic);
    char* pic_add = malloc(len+5);
	snprintf(pic_add, len+1, "%d", args -> numOfPic);
	strcat(pic_add, ".mp4");
	pic_add[len+5] = '\0';
	
	
	printf("Client: sending %s for server %d\n", pic_add, args->num_of_server);
	fd = open(pic_add, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "Client: Error openning file --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (stat(pic_add, &file_stat) < 0){
		fprintf(stderr, "Client: Error stat --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	

	fprintf(stdout, "Client: File Size: %ld bytes\n", file_stat.st_size);
	sprintf(file_size, "%ld", file_stat.st_size);
	socklen_t sock_len = sizeof(struct sockaddr_in);

	
	if (len = send(thsocketfd, file_size, sizeof(file_size), 0) < 0){
		fprintf(stderr, "Client: Error on sending greetings(file size) --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	//fprintf(stdout, "Client: sent %d bytes for the size\n", len);
	off_t offset =0;
	int remain_data = file_stat.st_size;
	
	while (((sent_bytes = sendfile(thsocketfd, fd, &offset, MAXDATASIZE)) > 0) && (remain_data > 0)){
        
        remain_data -= sent_bytes;
        //fprintf(stdout, "2. Client sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
    }

	if (remain_data == 0)
		printf("Client: sent the video number %d successfully\n", args -> numOfPic);

	if ((numbytes = recv(thsocketfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        exit(1);
		bzero(buf,MAXDATASIZE);
    }
	while (numbytes == 0){
		numbytes = recv(thsocketfd, buf, MAXDATASIZE, 0);
		bzero(buf,MAXDATASIZE);
	}
	buf[numbytes] = '\0';

	printf("Client: received code '%s' from server %d for %s\n", buf, args->num_of_server, pic_add);
	int code;
	sscanf(buf, "%d", &code);
	args->output[args->numOfPic - 1] =  code;
	
	args->th_end_flag = true;
	free(pic_add);
	
}

int create_socket(char* ip_add, char* port)
{	
	int sockfd, numbytes;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(ip_add, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "Client: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("Client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("Client: connecting to %s on port %s\n", s, port);

	freeaddrinfo(servinfo);// all done with this structure

	return sockfd;
}

int main(int argc, char *argv[])
{	
	struct flags *stat_ptr;
	pthread_t th1, th2;
	struct arg_struct args1;
	struct arg_struct args2;

	if (argc != 4) {
			fprintf(stderr,"usage: Client arguments\n");
			exit(1);
		}

	
	

	int picNum;
	sscanf(argv[1], "%d", &picNum); //converting input number to an integer
	int *result_codes = malloc(picNum);

	int sockfd1 = create_socket(argv[2], "3490");
	int sockfd2 = create_socket(argv[3], "3491");

	args1.socketfd = sockfd1;
	args2.socketfd = sockfd2;
	args1.num_of_server = 1;
	args2.num_of_server = 2;
	args1.numOfPic = 1; //first image for first server
	args2.numOfPic = 2; //first image for second server
	args1.output, args2.output = result_codes;

	int i = 3;

	args1.th_end_flag = false;
	int err;
	err = pthread_create(&th1, NULL, &send_video, (void*)&args1);
	if (err)
    {
        printf("Client: An error occured during creating s1 thread: %d", err);
        return 1;
    }

	args2.th_end_flag = false;
	err = pthread_create(&th2, NULL, &send_video, (void*)&args2);
	if (err)
    {
        printf("Client: An error occured during creating s2 thread: %d", err);
        return 1;
    }
	
	
	while((i > 2) && (i <= picNum)){
		if(args1.th_end_flag){
			pthread_join(th1, NULL);
			args1.numOfPic = i;
			args1.th_end_flag = false;
			err = pthread_create(&th1, NULL, &send_video, (void*)&args1);
			if (err)
    		{
       			printf("Client: An error occured during creating s1 thread: %d", err);
       			return 1;
    		}
			i++;
			continue;
		}
		if(args2.th_end_flag){
			pthread_join(th2, NULL);
			args2.numOfPic = i;
			args2.th_end_flag = false;
			err = pthread_create(&th2, NULL, &send_video, (void*)&args2);
			if (err)
    		{
       			printf("Client: An error occured during creating s2 thread: %d", err);
       			return 1;
    		}
			i++;
			continue;
		}
	}

	
	
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	
	args1.th_end_flag = false;
	pthread_create(&th1, NULL, &close_servers, (void*)&args1);
	args2.th_end_flag = false;
	pthread_create(&th2, NULL, &close_servers, (void*)&args2);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);

	close(sockfd1);
	close(sockfd2);

	saving_results(result_codes, picNum);

	printf("Client: sockets closed and ending message sent to servers\n");
	return 0;
}

