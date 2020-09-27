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
    char response[BUFSIZE], status[BUFSIZE], header[BUFSIZE], content[BUFSIZE];
    char * status_line_end, * header_end;

    int ret = 0;
    int res = 0;
    int sockfd = 0;
    struct hostent * serverIP;
    struct sockaddr_in server_address;
    //fd_set rset;

    /*parse args */
    if (argc != 4) {
        fprintf(stderr, "usage: http_client <hostname> <port> <path>\n");
        exit(-1);
    }

    server_name = argv[1];
    server_port = atoi(argv[2]);
    server_path = argv[3];

    ssize_t rv;


    ret = asprintf(&req_str, "GET %s HTTP/1.0\r\n\r\n", server_path);

    if (ret == -1) {
        fprintf(stderr, "Failed to allocate request string\n");
        exit(-1);
    }

    /*
     * NULL accesses to quiesce compiler errors
     * Delete the following lines
     */

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

    /* first read loop -- read headers */
    memset(response, 0, sizeof(response));

    if ((rv = recv(sockfd, response, sizeof(response), MSG_WAITALL)) < 0) {
      perror("pError: ");
      fprintf(stderr, "Error receiving response");
      exit(-1);
    }
    response[rv] = '\0';
    /* examine return code */
    status_line_end = strstr(response, "\r\n");
    strncpy(status, response, (int) (status_line_end-response));
    status[(int) (status_line_end-response)] = '\0';
    if (strstr(status, "OK") == NULL) {
      fprintf(stderr, "Error: Bad Request\n");
      exit(-1);
    }

    //Skip "HTTP/1.0"
    //remove the '\0'

    status_line_end = strstr(response, "\r\n");
    strncpy(status, response, (int) (status_line_end-response));
    status[(int) (status_line_end-response)] = '\0';

    // Normal reply has return code 200
    printf("Status Line: %s\n\n", status);

    /* print first part of response: header, error code, etc. */
    header_end = strstr(status_line_end+2, "\r\n\r\n");
    strncpy(header, status_line_end+2, (int) (header_end-status_line_end));
    header[(int) (header_end-status_line_end)] = '\0';
    printf("Header: \n%s\n", header);

    /* second read loop -- print out the rest of the response: real web content */
    strcpy(content, header_end+4);
    printf("Content: \n%s\n", content);
    /*close socket and deinitialize */
    close(sockfd);
    return 0;

}
