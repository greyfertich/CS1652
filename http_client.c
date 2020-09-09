/*
 * CS 1652 Project 1
 * (c) Jack Lange, 2020
 * (c) <Student names here>
 *
 * Computer Science Department
 * University of Pittsburgh
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 1024

int
main(int argc, char ** argv)
{

    char * server_name = NULL;
    int    server_port = -1;
    char * server_path = NULL;
    char * req_str     = NULL;

    int ret = 0;
    int res = 0;
    int sockfd = 0;
    struct hostent * serverIP;
    struct sockaddr_in server_address;

    /*parse args */
    if (argc != 4) {
        fprintf(stderr, "usage: http_client <hostname> <port> <path>\n");
        exit(-1);
    }

    server_name = argv[1];
    server_port = atoi(argv[2]);
    server_path = argv[3];



    ret = asprintf(&req_str, "GET %s HTTP/1.0\r\n\r\n", server_path);

    if (ret == -1) {
        fprintf(stderr, "Failed to allocate request string\n");
        exit(-1);
    }

    /*
     * NULL accesses to quiesce compiler errors
     * Delete the following lines
     */
    //(void)server_name;
    //(void)server_port;

    /* make socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      fprintf(stderr, "Error creating socket\n");
      exit(-1);
    }

    /* get host IP address  */
    /* Hint: use gethostbyname() */
    serverIP = gethostbyname(server_name);
    if (serverIP == NULL) {
      fprintf(stderr, "Error getting host IP address\n");
      exit(-1);
    }
    /* set address */
    //bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, serverIP->h_addr, serverIP->h_length);
    server_address.sin_port = htons(server_port);

    /* connect to the server socket */
    if (connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      perror("pError: ");
      fprintf(stderr, "Error connecting to server\n");
      exit(-1);
    }
    /* send request message */
    if ((res = write(sockfd, req_str, ret)) <= 0) {
      fprintf(stderr, "Error writing request to socket\n");
      exit(-1);
    }
    /* wait till socket can be read. */
    /* Hint: use select(), and ignore timeout for now. */
    if ((res = read(sockfd, req_str, sizeof(req_str)-1)) <= 0) {
      fprintf(stderr, "Error reading from socket\n");
      exit(-1);
    }
    req_str[res] = 0;
    printf("received: %s", req_str);
    close(sockfd);
    return 0;
    /* first read loop -- read headers */

    /* examine return code */

    //Skip "HTTP/1.0"
    //remove the '\0'

    // Normal reply has return code 200

    /* print first part of response: header, error code, etc. */

    /* second read loop -- print out the rest of the response: real web content */

    /*close socket and deinitialize */


}
