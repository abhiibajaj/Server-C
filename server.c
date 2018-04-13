/* A simple server in the internet domain using TCP
The port number is passed as an argument 


 To compile: gcc server.c -o server 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 2048
#define RUNNING 1

#define HTTPNOTFOUND "HTTP/1.0 404\r\n"
#define HTTPFOUND "HTTP/1.0 200 OK\r\n"

#define CONTENTHEADER "Content-Type: "

#define HTML "html"
#define CSS "css"
#define JPG "jpg"
#define JS "js"

#define HTMLCONTENT "text/html\r\n\r\n"
#define CSSCONTENT "text/css\r\n\r\n"
#define JPGCONTENT "image/jpeg\r\n\r\n"
#define JSCONTENT "application/javascript\r\n\r\n"


void write_http_response(char *final_path, int newsockfd);
void write_body_content(char *context, int newsockfd, FILE *fp);
void write_specific_content(char *content_type, int newsockfd, FILE *fp);



int main(int argc, char **argv) {

	int sockfd, newsockfd, portno;// clilen;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr;
	
	int n;

	if (argc < 3) 
	{
		fprintf(stderr,"ERROR, no port or path provided\n");
		exit(1);
	}

	/* Get the path */
	char *absolute_path = argv[2];


	 /* Create TCP socket */
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(1);
	}

	
	bzero((char *) &serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);
	
	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for 
	 this machine */
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);  // store in machine-neutral format

	 /* Bind address to the socket */
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0) 
	{
		perror("ERROR on binding");
		exit(1);
	}
	
	/* Listen on socket - means we're ready to accept connections - 
	 incoming connection requests will be queued */
	
	listen(sockfd,5);
	
	

	/* Accept a connection - block until a connection is ready to
	 be accepted. Get back a new file descriptor to communicate on. */
	while(RUNNING){

		newsockfd = accept(	sockfd, (struct sockaddr *)NULL, 
							NULL);

		if (newsockfd < 0)  {
			perror("ERROR on accept");
			exit(1);
		}

		
		bzero(buffer, BUFFER_SIZE);

		/* Read characters from the connection,
			then process */
		
		n = read(newsockfd, buffer, BUFFER_SIZE-1);


		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		
		const char *delimitter = " ";
		
		
		char *path;
		

		strtok(buffer, delimitter);

		path = strtok(NULL, delimitter);

		char *final_path = malloc(strlen(absolute_path)+strlen(path)+1);

		strcpy(final_path, absolute_path);
		strcat(final_path, path);
		
		write_http_response(final_path, newsockfd);
	  
	    
	  

	   	
		
		
		
		
		/* close socket */
		
		close(newsockfd);
		
	}
	close(sockfd);
	return 0; 
}

void
write_http_response(char *final_path, int newsockfd){
	int n;
	FILE *fp;
	if((fp = fopen(final_path, "r"))==NULL){
		n =write(newsockfd, HTTPNOTFOUND, strlen(HTTPNOTFOUND));
	} else {
		n =write(newsockfd, HTTPFOUND, strlen(HTTPFOUND));

		char *context;
		char *type_delimitter = ".";
   		strtok_r(final_path, type_delimitter, &context);
  		
   		n =write(newsockfd, CONTENTHEADER, strlen(CONTENTHEADER));
	   		
	   	

  		write_body_content(context, newsockfd, fp);
	   		
	   	
   	} 
}

void
write_body_content(char *context, int newsockfd, FILE *fp){

	if(strcmp(context, CSS)==0){
		write_specific_content(CSSCONTENT, newsockfd, fp);		
	} else if(strcmp(context, HTML)==0){
	   	write_specific_content(HTMLCONTENT, newsockfd, fp);
	} else if(strcmp(context, JPG)==0){
	   	write_specific_content(JPGCONTENT, newsockfd, fp);
	} else if(strcmp(context, JS)==0){
	   	write_specific_content(JSCONTENT, newsockfd, fp);	
	} else {
	   	fprintf(stderr, "Not supported \n");
	}
	   		
}

void
write_specific_content(char *content_type, int newsockfd, FILE *fp) {

	

	int n = write(newsockfd, content_type, strlen(content_type));
	unsigned char *text;
	fseek(fp, 0L, SEEK_END);
	unsigned long file_size = ftell(fp);
	rewind(fp);
	text = calloc(1, file_size+1);
	if(fread(text, file_size, 1, fp)!=1){
		fprintf(stderr, "WRONG");
	}
	n = write(newsockfd, text, file_size);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
}