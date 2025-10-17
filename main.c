#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "request.h"
#include "response.h"



#define PORT 8080
#define ADDRESS "0.0.0.0"
#define MINSPARETHREADS 25
#define MAXTHREADS 200

void* handle_connection(struc request* req, struct resp* 

void main(int argc, char  const* argv[]){
    //Defining the file descriptor returned by socket()
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //Defining the socket address to bind
    struct sockaddr_in sock_addr;
    struct in_addr address;
    inet_aton(ADDRESS,&address);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr = address;
    
    //Bind the socket address to the socket file descriptor
    int opt = 1;
	//Set the sochet option to reuse the port if already in use
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if(bind(server_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    //Prepare to accept connections on file descriptor
    if(listen(server_fd, 1) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    };

    printf("Accepting...\n");

    //Preparing the client file descriptor
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

	//Declaring request and response variables
    struct request* req = malloc(sizeof(struct request));
    struct response* resp = malloc(sizeof(struct response));  

	pthread_t threads[MAXTHREADS];
	//Create a thread_data_t argument array
	thread_data_t thr_data[MAXTHREADS];

	while(1) { 
		//Accept a connection, this puts the client address in the structure 
		client_fd = accept(server_fd, 
					 (struct sockaddr*)&client_addr, 
					 &client_addr_len);

		if(client_fd > 0) {
            printf("Connection accepted\n");
			pthread_create(handle_connection(client_fd)
        }
        else {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //Read the request, we use 8kb buffer (should be enough)
        char req_buf[8192];
        char resp_buf[8192];
        if(read(client_fd,req_buf,sizeof(req_buf)) < 0){
            perror("read");
        }
        else
        {
            //Create a request object
            parse_request(req,req_buf);
            //print_request(req);
            char* requested_path = parse_path(req->path);
            printf("%s\n", requested_path);

            //Open file and return a file pointer
            FILE* body_fd = fopen(requested_path, "rb");
            //404
            if(body_fd == NULL){
                resp = create_response("HTTP/1.1", 404, "Not found", "Not found");
            }
            else{
                //Search for the end of the file, the file pointer will point 
				//to the end of the file
                fseek(body_fd, 0, SEEK_END);
                //With this we will take the poisition number 
				//of the end of the file, thus giving us the length
                long fsize = ftell(body_fd);
                //Return to the begin of the file
                fseek(body_fd, 0, SEEK_SET);
                //Allocate a string with the size of the file
                char* body = malloc(fsize + 1);
                fread(body, fsize, 1, body_fd);

                resp = create_response("HTTP/1.1", 200, "OK", body);
                fclose(body_fd);
            }
            int resp_len = serialize_resp(resp, resp_buf, sizeof(resp_buf));
            send(client_fd, resp_buf, strlen(resp_buf), 0);
            free(requested_path);
            close(client_fd);
        }
    }
    

    //Always close the doors
    free(req);
    free(resp);
    close(server_fd);
        
        
}
