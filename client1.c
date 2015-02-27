
/**
 * @file   proj1_client.c
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file was written as an example program for
 *
 * 	EE 122: Introduction to Communication Networks
 * 	Spring 2015
 */

/* needed for sockets */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

/* standard libraries */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256

int main(int argc, char** argv)
{
	char buffer[BUFFER_SIZE];
	int sd, ret, listen_fd, comm_fd;
	unsigned short port;
	unsigned int mode;
	char *filename, *stats_filename, *server_address;
	struct sockaddr_in servaddr;

	/* parse some args */
	// ret = parse_args(&port, &mode, &packet_size, &packet_delay, filename, argc, argv);
	// if(ret)
	// 	return 1;
	if(argc != 6 || !strcmp(argv[1], "-h"))
	{
		/* incorrect number of arguments given,
		 * or help flag given.
		 *
		 * Print usage */
		printf(" Usage:\n\n"
			   "\t <mode>\n\n"
		       "\t <server_address>\n\n"
		       "\t <port>\n\n"
		       "\t <received_filename>\n\n"
		       "\t <stats_filename>\n\n"
		       " This client will do as the hw instruction.\n\n");
		return 1; /* failure */
	}

	/* get the port */
	port = atoi(argv[3]);
	mode = atoi(argv[1]);
	filename = argv[4];
	stats_filename = argv[5];
	server_address = argv[2];

	if(port < 1000)
	{
		fprintf(stderr, "Invalid port number: %d\n", port);
		fprintf(stderr, "(Only accepts ports over 1000)\n");
		return 1; /* failure */
	}
	//success

	printf("Filename is: %s\n", filename);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	    fprintf(stderr, "Can't create a socket\n");
		exit(1); 
	}
	inet_pton(AF_INET, server_address, &(servaddr.sin_addr));
	servaddr.sin_port = htons(port);

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        fprintf(stderr, "Can't connect\n");
        exit(1);
	}
	printf("Connected: server's address is %s\n", server_address);

	// printf("Receive:\n");
	// bp = rbuf;
	// bytes_to_read = BUFLEN;
	// while ((n = read(sd, bp, bytes_to_read)) > 0) {
	//         bp += n;
	//         bytes_to_read -= n;
	// }
	// printf("%s\n", rbuf);
	close(sd);
	return(0);

}
