
/**
 * @file   proj1_server.c
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

/* standard libraries */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* this defines the size of our buffer */
#define MAX_NUM_CLIENTS 10

/**
 * An echo server will listen on the
 * specified port, and repeat back whatever
 * it receives.
 */
int main(int argc, char** argv)
{
	int ret, listen_fd, comm_fd;
	unsigned short port;
	unsigned int mode, packet_size;
	double packet_delay;
	char *filename;
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
			   "\t%s <mode>\n\n"
		       "\t <port>\n\n"
		       "\t <filename>\n\n"
		       "\t <packet_size>\n\n"
		       "\t <packet_delay>\n\n"
		       " This server will do as the hw instruction.\n\n",argv[0]);
		return 1; /* failure */
	}

	/* get the port */
	port = atoi(argv[2]);
	mode = atoi(argv[1]);
	filename = argv[3];
	packet_size = atoi(argv[4]);
	packet_delay = atof(argv[5]);

	if(port < 1000)
	{
		fprintf(stderr, "Invalid port number: %d\n", port);
		fprintf(stderr, "(Only accepts ports over 1000)\n");
		return 1; /* failure */
	}
	//success

	printf("Filename is: %s\n", filename);

	/*-------------------*/
	/* set up the server */
	/*-------------------*/

	/* first create a socket to use 
	 *
	 * This is like opening a file, and
	 * gives you a file descriptor using IP addressing */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* prepare the structure that represents this socket*/

	/* uses IP addressing scheme */
	servaddr.sin_family = AF_INET;
	
	/* open to all clients 
	 *
	 * the call to 'htons' means converting numbers from
	 * host format (little-endian) to network format (big-endian) */
	servaddr.sin_addr.s_addr = htons(INADDR_ANY); 
	
	/* listen on input port */
	servaddr.sin_port = htons(port);

	/* binding the server structure to this socket means
	 * that the file descriptor for this socket will
	 * have the properties defined in this structure
	 * (e.g. port number, address) */
	bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	/* once we call listen, then the port will be open to any
	 * incoming connections.  The "backlog" indicates how many
	 * clients can be waiting to connect at once.  If more than
	 * this number try to connect simulatenously, they will
	 * be dropped. */
	listen(listen_fd, MAX_NUM_CLIENTS);
	printf("[server]\tlistening on port: %d\n", port);

	/*-------------------------*/
	/* accept incoming clients */
	/*-------------------------*/
	char buffer[packet_size];
	/* infinite-loop */
	while(1)
	{
		/* take next client on this socket
		 *
		 * This call will block, meaning that the
		 * program won't continue until a client
		 * connects. */
		comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		printf("\n[server]\tGot a new client!\n");

		/* echo this client for as long as it is connected */
		int file_handle = open(filename, O_RDONLY, S_IREAD);

		while(1)
		{
			/* clear the buffer */
			bzero(buffer, packet_size);
			

			/* read the clients message */
			if(0 == read(file_handle, buffer, packet_size))
			{
				/* reading zero bytes means
				 * the socket was closed by
				 * the client. */
				break;
			}

			/* print it to screen */
			printf(" - %s", buffer);

			/* echo it back to client
			 * (including null-terminator) */
			write(comm_fd, buffer, strlen(buffer)+1);

			/* delay */
			usleep(packet_delay * 1000000);
		}

		/* close the connection */
		close(comm_fd);
		printf("[server]\tClient left.\n");
	}

	/* since this server should theoretically
	 * run forever (or until you ctrl-c it),
	 * then we should never get here. */
	return 0;
}

/**
 * Parses input command-line arguments
 */
// int parse_args(unsigned short* port, unsigned int* mode, unsigned int* packet_size, 
// 	unsigned int* packet_delay, char[]* filename, int argc, char** argv)
// {
// 	/* check if sufficient arguments were given */
// 	if(argc != 6 || !strcmp(argv[1], "-h"))
// 	{
// 		/* incorrect number of arguments given,
// 		 * or help flag given.
// 		 *
// 		 * Print usage */
// 		printf(" Usage:\n\n"
// 		       "\t%s <port>\n\n"
// 		       " This echoserver will listen on <port>\n"
// 		       " and repeat everything to the client.\n\n",argv[0]);
// 		return -1; /* failure */
// 	}

// 	/* get the port */
// 	*port = atoi(argv[2]);
// 	*mode = atoi(argv[1]);
// 	*filename = argv[3];
// 	*packet_size = atoi(argv[4]);
// 	*packet_delay = atoi(argv[5]);

// 	if(*port < 1000)
// 	{
// 		fprintf(stderr, "Invalid port number: %d\n", *port);
// 		fprintf(stderr, "(Only accepts ports over 1000)\n");
// 		return -2; /* failure */
// 	}

// 	/* success */
// 	return 0;
// }