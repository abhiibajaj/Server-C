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
#define HTMLCONTENT "text/html\r\n\r\n"
#define CSSCONTENT "text/css\r\n\r\n"
#define JPGCONTENT "image/jpeg\r\n\r\n"
#define JSCONTENT "application/javascript\r\n\r\n"

void write_specific_content(char *content_type, int newsockfd, FILE *fp);



int main(int argc, char **argv) {

	int sockfd, newsockfd, portno;// clilen;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	
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
	while(1){
		newsockfd = accept(	sockfd, (struct sockaddr *)NULL, 
							NULL);

		if (newsockfd < 0) 
		{
			perror("ERROR on accept");
			exit(1);
		}

		
		bzero(buffer, BUFFER_SIZE);

		/* Read characters from the connection,
			then process */
		
		n = read(newsockfd, buffer, BUFFER_SIZE-1);


		if (n < 0) 
		{
			perror("ERROR reading from socket");
			exit(1);
		}
		

		
		int inputLength = strlen(buffer);
		char *inputCopy = (char*) calloc(inputLength + 1, sizeof(char));
		strncpy(inputCopy, buffer, inputLength);

		const char *delimitter = " ";
		
		char *get;
		char *path;
		

		get = strtok(buffer, delimitter);
		path = strtok(NULL, delimitter);
		char *final_path = malloc(strlen(absolute_path)+strlen(path)+1);

		strcpy(final_path, absolute_path);
		strcat(final_path, path);
		
	  
	    char *wrong = "HTTP/1.0 404\r\n";
	    char *right = "HTTP/1.0 200 OK\r\n";

	   	FILE *fp;
	   	if((fp = fopen(final_path, "r"))==NULL){
	   		
	   		n =write(newsockfd, wrong, strlen(wrong));
	   		

	   	} else {
	   		char *context;
	   		char *type_delimitter = ".";
	   		char *file_type =  strtok_r(final_path, type_delimitter, &context);
	   		
	   		n =write(newsockfd, right, strlen(right));
	   		fprintf(stderr, "%s", right);
	   		char *content_header = "Content-Type: ";
	   		n =write(newsockfd, content_header, strlen(content_header));
	   		fprintf(stderr, "%s", content_header);
	   	


	   		if(strcmp(context, "css")==0){
	   			
	   			write_specific_content(CSSCONTENT, newsockfd, fp);
	   			
	   		} else if(strcmp(context, "html")==0){
	   			write_specific_content(HTMLCONTENT, newsockfd, fp);
	   			

	   		} else if(strcmp(context, "jpg")==0){
	   			write_specific_content(JPGCONTENT, newsockfd, fp);
	   		} else if(strcmp(context, "js")==0){
	   			write_specific_content(JSCONTENT, newsockfd, fp);
	   			
	   		} else {
	   			fprintf(stderr, "Not supported \n");
	   		}
	   		
	   	
	   	}
	    




	    
		
		
		if (n < 0) 
		{
			perror("ERROR writing to socket");
			exit(1);
		}
		
		/* close socket */
		
		close(newsockfd);
		
	}
	return 0; 
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
	if(strcmp(content_type, JPGCONTENT)==0){
		n = write(newsockfd, text, file_size);
	} else {
		n = write(newsockfd, text, strlen(text));
	}
}