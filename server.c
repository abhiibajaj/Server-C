/* A simple http 1.0 server that can support html, css, js & jpg
The port number is passed as an argument, along with the path

This was adapted from the server.c file provided in Workshop 4
of Computer Systems

 To compile: make

 Name: Arpit Bajaj
 Login: bajaja@student.unimelb.edu.au
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 2048
#define RUNNING 1

/* Status Line headers */
#define HTTPNOTFOUND "HTTP/1.0 404\r\n"
#define HTTPFOUND "HTTP/1.0 200 OK\r\n"

/* Content Type Header */
#define CONTENTHEADER "Content-Type: "

/* Accepted formats */
#define HTML "html"
#define CSS "css"
#define JPG "jpg"
#define JS "js"

/* Content types, with \r\n\r\n for formatting */
#define HTMLCONTENT "text/html\r\n\r\n"
#define CSSCONTENT "text/css\r\n\r\n"
#define JPGCONTENT "image/jpeg\r\n\r\n"
#define JSCONTENT "application/javascript\r\n\r\n"

#define MAXREQUESTS 2

/* Error messages */
#define WRITEERROR "ERROR writing to socket"
#define ACCEPTERROR "ERROR on accept"
#define READERROR "ERROR reading from socket"
#define OPENERROR "ERROR opening socket"
#define BINDERROR "ERROR on binding"

/* Holds the data that needs to be passed onto the thread */
typedef struct {
	int newsockfd;
	char *absolute_path;
} thread_data_t;


/****************************************************************************************/
/* Function prototypes */

void* handle_request(void *httpdata);
void write_http_response(char *final_path, int newsockfd);
void write_body_content(char *context, int newsockfd, FILE *fp);
void write_specific_content(char *content_type, int newsockfd, FILE *fp);
char* get_final_path(char *buffer, char* absolute_path);
void check_request(int n, char *message);

/****************************************************************************************/

int main(int argc, char **argv) {

	/* Initialise variable required */
	int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr;
	pthread_t tid1;
	thread_data_t httpdata;
	int n;

	if (argc < 3) {
		fprintf(stderr,"ERROR, no port or path provided\n");
		exit(1);
	}

	/* Get the path */
	char *absolute_path = argv[2];

	/* Get the port number */
	portno = atoi(argv[1]);

	 
	/* Create TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	check_request(sockfd, OPENERROR);

	
	bzero((char *) &serv_addr, sizeof(serv_addr));

	
	
	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for 
	 this machine */
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);  // store in machine-neutral format

	 /* Bind address to the socket */
	
	n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	check_request(n, BINDERROR);

	
	/* Listen on socket - means we're ready to accept connections - 
	 incoming connection requests will be queued */
	
	listen(sockfd, MAXREQUESTS);
	
	

	/* Leave the server running so it can accept requests */
	while(RUNNING){

		/* Accept a connection - block until a connection is ready to
		 be accepted. Get back a new file descriptor to communicate on. */
		newsockfd = accept(	sockfd, (struct sockaddr *)NULL, 
							NULL);
		check_request(newsockfd, ACCEPTERROR);
		
		/* Add newsockfd and the path to the struct that is passed on to the thread */
		httpdata.newsockfd = newsockfd;
		httpdata.absolute_path = malloc(strlen(absolute_path));
		strcpy(httpdata.absolute_path, absolute_path);

		/* Create a thread to handle the request through handle_request function and the 
			httpdata struct */
		if(pthread_create(&tid1, NULL, handle_request, (void*) &httpdata)) {
      		printf("\n Error creating thread 1");
      		exit(1);
   		}

			
	}

	close(sockfd);
	return 0; 
}

/****************************************************************************************/

/* Handles the http request and writes the http response for the request*/

void* 
handle_request(void *httpdata){

	int newsockfd;
	char *absolute_path;
	char buffer[BUFFER_SIZE];
	int n;
	
	/* Get the data from the struct */
	thread_data_t *inside_thread_data = (thread_data_t*) httpdata;
	newsockfd = inside_thread_data->newsockfd;
	absolute_path = inside_thread_data->absolute_path;
	
	
	bzero(buffer, BUFFER_SIZE);


	/*Read characters from the connection,
		then process */
		
	n = read(newsockfd, buffer, BUFFER_SIZE-1);
	check_request(n, READERROR);

	/* Get the final path to where the file is located */
	char *final_path = get_final_path(buffer, absolute_path);

	/* Write the response */
	write_http_response(final_path, newsockfd);		

	/* close socket */
	close(newsockfd);	
	return NULL;
	
}	

/****************************************************************************************/

/* Formulates the http response and writes it back */

void
write_http_response(char *final_path, int newsockfd){

	int n;
	FILE *fp;

	if((fp = fopen(final_path, "r"))==NULL){

		/* File doesn't exist, write HTTPNOTFOUND status line */
		n =write(newsockfd, HTTPNOTFOUND, strlen(HTTPNOTFOUND));
		check_request(n, WRITEERROR);

	} else {

		/* File exists, write HTTPFOUND status line and the Content-Type Header */
		n =write(newsockfd, HTTPFOUND, strlen(HTTPFOUND));
		check_request(n, WRITEERROR);
		n = write(newsockfd, CONTENTHEADER, strlen(CONTENTHEADER));
	   	check_request(n, WRITEERROR);	

		/* Get the file format, stored in context */
		char *context;
		char *type_delimitter = ".";
 		strtok_r(final_path, type_delimitter, &context);
  		
		/* Write the content in the file */
  		write_body_content(context, newsockfd, fp);
	   		
   	} 
}

/****************************************************************************************/

/* Combines the path specified in the commandline, with the path in the http request, and 
	returns it as a char array */

char* 
get_final_path(char *buffer, char* absolute_path) {

	/* Extract the path from the http request in the buffer*/
	char *delimitter = " ";
	char *relative_path;
	strtok(buffer, delimitter);
	relative_path = strtok(NULL, delimitter);

	/* Combine them into one single path */
	char *final_path = malloc(strlen(absolute_path)+strlen(relative_path)+1);
	strcpy(final_path, absolute_path);
	strcat(final_path, relative_path);
	return final_path;
}

/****************************************************************************************/

/* Gets the file format, and writes the content of the file based on the format */

void
write_body_content(char *context, int newsockfd, FILE *fp){

	/* Check the file format and pass it to the write specific content function  with the
		correct MIME type */
	if(strcmp(context, CSS)==0){
		write_specific_content(CSSCONTENT, newsockfd, fp);		
	} else if(strcmp(context, HTML)==0){
	   	write_specific_content(HTMLCONTENT, newsockfd, fp);
	} else if(strcmp(context, JPG)==0){
	   	write_specific_content(JPGCONTENT, newsockfd, fp);
	} else if(strcmp(context, JS)==0){
	   	write_specific_content(JSCONTENT, newsockfd, fp);	
	} else {
	   	perror("Not supported type");
	   	exit(1);
	}
	   		
}
/****************************************************************************************/

/* Write the file type as well as the content of the files for the http response */

void
write_specific_content(char *content_type, int newsockfd, FILE *fp) {

	/* Writes the MIME TYPE for the file format*/
	int n = write(newsockfd, content_type, strlen(content_type));
	check_request(n, WRITEERROR);

	/* Get the size of the file and dynamically allocate space for it to be stored in text*/
	/* Adapted from https://stackoverflow.com/questions/3747086/reading-the-whole-text-file-into
                                                    -a-char-array-in-c#comment52907469_22515917 */
	unsigned char *text;
	fseek(fp, 0L, SEEK_END);
	unsigned long file_size = ftell(fp);
	rewind(fp);
	text = calloc(1, file_size+1);

	/* Read the file contents into text variable */
	if(fread(text, file_size, 1, fp)!=1){
		perror("Error reading file");
		exit(1);
	}

	/* Write out the file contents to the response*/
	n = write(newsockfd, text, file_size);
	check_request(n, WRITEERROR);
}

/****************************************************************************************/
/* Socket error checker, displays specified message detailing error*/
void check_request(int n, char* message){
	if (n < 0) {
		perror(message);
		exit(1);
	}
}

/****************************************************************************************/

