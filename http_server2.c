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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 1024
#define FILENAMESIZE 100


static int
handle_connection(int sock)
{

    char * ok_response_f  = "HTTP/1.0 200 OK\r\n"						\
                            "Content-type: text/plain\r\n"				\
                            "Content-length: %d \r\n\r\n";

    char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"			\
                            "Content-type: text/html\r\n\r\n"			\
                            "<html><body bgColor=black text=white>\n"	\
                            "<h2>404 FILE NOT FOUND</h2>\n"				\
                            "</body></html>\n";

    int len = 0, written_chars;
    char buf[BUFSIZE], filename[BUFSIZE], response[BUFSIZE];
    char * filename_start, * filename_end;
    FILE * requested_file;

    memset(buf, 0, sizeof(buf));
    /* first read loop -- get request and headers*/
    if ((len = read(sock, buf, sizeof(buf))) <= 0) {
      fprintf(stderr, "Error reading from socket\n");
      exit(-1);
    }
    buf[len] = 0;
    /* parse request to get file name */

    /* Assumption: For this project you only need to handle GET requests and filenames that contain no spaces */
    filename_start = strchr(buf, ' ') + 1;
    filename_end = strchr(filename_start, ' ');
    strncpy(filename, filename_start, (int) (filename_end - filename_start));
    filename[(int) (filename_end - filename_start)] = '\0';
    printf("Received request for file: %s\n", filename);
    memset(buf, 0, sizeof(buf));

    /* open and read the file */
    if ((requested_file = fopen(filename, "r")) == NULL) {
      // Construct HTTP error response
      strcpy(buf, notok_response);
      printf("Error retrieving file: %s\n", filename);
    } else {
      // Send file in http response
      memset(buf, 0, sizeof(buf));
      size_t newLen = fread(response, sizeof(char), BUFSIZE, requested_file);
      written_chars = sprintf(buf, ok_response_f, (int) newLen);
      if (sizeof(response) + written_chars < BUFSIZE) {
        strncpy(buf+written_chars, response, sizeof(response));
        buf[written_chars + sizeof(response)] = '\0';
      } else {
        strncpy(buf+written_chars, response, sizeof(response)-written_chars);
        buf[BUFSIZE-1] = '\0';
      }
    }

    /* send response */
    if ((len = write(sock, (char*) buf, strlen((char *) buf))) <= 0) {
      fprintf(stderr, "Error writing to socket\n");
      exit(-1);
    }
    /* close socket and free space */
    close(sock);
    return 0;
}



int
main(int argc, char ** argv)
{
    int server_port = -1;
    int ret         =  0;
    int sock        = -1;

    struct sockaddr_in saddr;
    int sock2 = -1, max_clients = 64, i = 0;
    int client_sockets[max_clients], max_sd = -1;
    fd_set readfds;

    /* parse command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: http_server1 port\n");
        exit(-1);
    }

    server_port = atoi(argv[1]);

    if (server_port < 1500) {
        fprintf(stderr, "Requested port(%d) must be above 1500\n", server_port);
        exit(-1);
    }

    for (i = 0; i < max_clients; i++) {
      client_sockets[i] = 0;
    }

    /* initialize and make socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      fprintf(stderr, "Error creating server socke\n");
      exit(-1);
    }
    /* set server address*/
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(server_port);

    /* bind listening socket */
    if (bind(sock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
      fprintf(stderr, "Error binding to socket");
      exit(-1);
    }
    /* start listening */
    if (listen(sock, 32) < 0) {
      fprintf(stderr, "Error listening on socket\n");
      exit(-1);
    }
    /* connection handling loop: wait to accept connection */

    while (1) {
        //sock2 = accept(sock, NULL, NULL);
        /* create read list */
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        max_sd = sock;

        for (i = 0; i < max_clients; i++) {
          if (client_sockets[i] > 0) {
            FD_SET(client_sockets[i], &readfds);
          }
          if (client_sockets[i] > max_sd) {
            max_sd = client_sockets[i];
          }
        }
        /* do a select */
        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
          fprintf(stderr, "Error on select\n");
          exit(-1);
        }
        /* process sockets that are ready */
        /* for the accept socket, add accepted connection to connections */
        /* for a connection socket, handle the connection */
        if (FD_ISSET(sock, &readfds)) {
          if ((sock2 = accept(sock, NULL, NULL)) < 0) {
            fprintf(stderr, "Error accepting on master socket\n");
            exit(-1);
          }
          for (i = 0; i < max_clients; i++) {
            if (client_sockets[i] == 0) {
              client_sockets[i] = sock2;
              break;
            }
          }
        }

        for (i = 0; i < max_clients; i++) {
          if (FD_ISSET(client_sockets[i], &readfds)) {
              ret = handle_connection(client_sockets[i]);
              close(client_sockets[i]);
              client_sockets[i] = 0;
          }
        }
    }
    (void)ret;
}
