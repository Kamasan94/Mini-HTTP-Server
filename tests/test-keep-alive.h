/*Function for testing the keeping alive of 
* the connection, if the connection: keep-alive
* header is present, the connection is not dropper
*/

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(char* message) { perror(message); exit(EXIT_FAILURE);}

void test_keep_alive() {

   /*The structure containing the addres*/
   struct sockaddr_in sock_addr;

   /*Structure of the address*/
   struct in_addr address;

   /*IPV4 dot notation -> binary format*/
   inet_aton("127.0.0.1",&address);

   /*IP Address and Port Family*/
   sock_addr.sin_family = AF_INET;

   /*Host byte order -> network byte order*/
   sock_addr.sin_port = htons(8080);

   sock_addr.sin_addr = address;

   char response[4096];

   char *keep_request = 
   "GET / HTTP/1.1\n "
   "Host: localhost\n"
   "Connection: keep-alive\n"
   "\r\n\r\n";
   
   char *close_request = 
   "GET / HTTP/1.1\n "
   "Host: localhost\n"
   "Connection: close\n"
   "\r\n\r\n";

   /*Client file descriptor*/
   int fd;

   if(bind(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }
   
   if(connect(fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) != 0) {
      printf("nice");
   }

   /* Send the requets */
   int r1_len = strlen(keep_request);
   int r2_len = strlen(close_request);
   int bytes;
   int sent = 0;
   while(sent < r1_len){
      bytes = write(fd, keep_request + sent, r1_len - sent);
      if(bytes < 0) error("ERROR writing message to socket");
      sent += bytes;
   }

   /* Receive the response */
   memset(response, 0, sizeof(response));

   read(fd, response, sizeof(response));

   printf("%d\n", response);

}