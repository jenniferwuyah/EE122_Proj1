#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
	int sd, ret, listen_fd, comm_fdn, bytes_to_read, conless_send;
	unsigned short port;
	unsigned int mode;
	char *filename, *stats_filename, *server_address;
	struct sockaddr_in servaddr, placeholder;
    int buflen = 4096;
    char buf[buflen], time_str[100];
    struct timeval start, end;
    float sec_delay;

    FILE * fp;
    FILE * stat_fp;

    if(argc != 6 || !strcmp(argv[1], "-h"))
    {
        /* incorrect number of arguments given or help flag given.
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

    /* Parse args */
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

	printf("Filename is: %s\n", filename);

    if (mode ==0) {
        sd = socket(AF_INET, SOCK_STREAM,0);
    } else {
        sd = socket(AF_INET, SOCK_DGRAM,0);
    }
	if (sd == -1) {
	    fprintf(stderr, "Can't create a socket\n");
		exit(1); 
	}

	inet_pton(AF_INET, server_address, &(servaddr.sin_addr));
	servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;

	/* Connecting to the server */
    printf("Connected: server's address is %s\n", inet_ntoa(servaddr.sin_addr));
    puts("Waiting for connection");

    int first_pkt = 1; //For delay timing
	
    if (mode==0) {
    	if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
            printf("Couldn't connect to server");
            close(sd);
            exit(1);
        }
        
        printf("Connected: server's address is %s\n", inet_ntoa(servaddr.sin_addr));
        fp = fopen (filename, "w");
        stat_fp = fopen (stats_filename, "w");

        while (read(sd, buf, buflen) > 0) {
            gettimeofday(&end, NULL);
            if (first_pkt!=1) {
                sec_delay = (float)(end.tv_sec - start.tv_sec) + ((float)end.tv_usec - (float)start.tv_usec)/1000000 ;
                sprintf(time_str, "%f\n",sec_delay);
                printf("delay is %s", time_str);
                fputs(time_str, stat_fp); //Write into file:
                bzero(time_str, 100);
            } else {
                first_pkt = 0;
            }
            fputs(buf, fp); //Write into file:
            bzero(buf, buflen);
            gettimeofday(&start, NULL);
        } 
    } else {
        char *temp = "Initial";
        //Talk to server to begin. 
        if (sendto(sd, temp, strlen(temp) , 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
            printf("Couldn't send to the server");
            close(sd);
            exit(1);
        }

        printf("Sent intial to server\n");
        fp = fopen (filename, "w");
        stat_fp = fopen (stats_filename, "w");
        printf("Actually waiting now\n");

        while (recvfrom(sd, buf, buflen, 0, NULL, NULL) > 0) {
            gettimeofday(&end, NULL);
            if (first_pkt!=1) {
                sec_delay = (float)(end.tv_sec - start.tv_sec) + ((float)end.tv_usec - (float)start.tv_usec)/1000000 ;
                sprintf(time_str, "%f\n",sec_delay);
                printf("delay is %s", time_str);
                fputs(time_str, stat_fp); //Write into file:
                bzero(time_str, 100);
            } else {
                first_pkt = 0;
            }
            fputs(buf, fp); //Write into file:
            bzero(buf, buflen);
            gettimeofday(&start, NULL);
        } 

    }
    fclose(stat_fp);
    fclose(fp);
    close(sd);
	return 0;

}
