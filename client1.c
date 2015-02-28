
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
#include <sys/time.h>

//#define BUFFER_SIZE 4096

int main(int argc, char** argv)
{
	//char buffer[BUFFER_SIZE];
	int sd, ret, listen_fd, comm_fd;
	unsigned short port;
	unsigned int mode;
	char *filename, *stats_filename, *server_address;
	struct sockaddr_in servaddr;
    int buflen = 4096;
    int n, bytes_to_read;
    char buf[buflen], time_str[100];
    struct timeval start, end;
    float sec_delay;
	/* parse some args */
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

	if ((sd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
	    fprintf(stderr, "Can't create a socket\n");
		exit(1); 
	}
	inet_pton(AF_INET, server_address, &(servaddr.sin_addr));
	servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;

	/* Connecting to the server */
    printf("Connected: server's address is %s\n", inet_ntoa(servaddr.sin_addr));
    puts("waiting for connection");

    int first_pkt = 1;
	
	if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 1) {
    
        printf("Connected: server's address is %s\n", inet_ntoa(servaddr.sin_addr));

        printf("Receive:\n");
        bytes_to_read = buflen;
        FILE * fp;
        FILE * stat_fp;
        fp = fopen (filename, "w");
        stat_fp = fopen (stats_filename, "w");
        while ((n = read(sd, buf, bytes_to_read)) > 0) {
            //printf("bytes writing %i", (int) strlen(buf));
            gettimeofday(&end, NULL);
            if (first_pkt!=1) {
                sec_delay = ((float)end.tv_usec - (float)start.tv_usec) / 1000000;
                //printf("delay is %f", sec_delay);
                sprintf(time_str, "%f\n",sec_delay);
                printf("delay is %s", time_str);
                fputs(time_str, stat_fp); //Write into file:
                bzero(time_str, 100);
            } else {
                first_pkt = 0;
            }
            start=end;
            fputs(buf, fp); //Write into file:
            bzero(buf, buflen);
        }
        fclose(stat_fp);
        fclose(fp);

    }
    close(sd);
	return 0;

}
