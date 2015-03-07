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
    int sd, buflen;
    unsigned short port;
    char *server_address;
    struct sockaddr_in server;
    char time_str[100];
    struct timeval start, end, conn_start, conn_end;
    float sec_delay;
    int bFlag = 0;

    char *temp = "Initial";

    if((argc != 3 && argc != 4) || !strcmp(argv[1], "-h")) {
        /* incorrect number of arguments given or help flag given.
         * Print usage */
        printf("%i\n",argc);
        printf(" Usage:\n\n"
               "\t%s <server_address>\n\n"
               "\t <port>\n\n"
               "\t [-b]\n\n"
               " This client will do as the hw instruction.\n\n",argv[0]);
       return 1; /* failure */
    }

    /* Parse args */
    port = atoi(argv[2]);
    server_address = argv[1];
    if (argc==4) {
	if (strcmp(argv[3],"-b")!=0) {
		bFlag = 0;
	} else {
		bFlag = 1;
	}	
    }    

    if(port < 1024) {
        fprintf(stderr, "[client4]\tError: Invalid port number <%d>.\n", port);
        fprintf(stderr, "\t\t(Only accepts ports over 1000)\n");
        return 1; /* failure */
    }

    //create socket
    if ( (sd = socket(AF_INET, SOCK_DGRAM,0)) == -1 ) {
        fprintf(stderr, "[client4]\tError: Can't create a socket.\n");
        exit(1); 
    }

    inet_pton(AF_INET, server_address, &(server.sin_addr));
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    /* Connecting to the server */

    //setup for delay timing
    int first_pkt = 1;
    int count =0;
    int n =0;

    //Talk to server to begin. 

    if (sendto(sd, temp, strlen(temp) , 0, (struct sockaddr *) &server, sizeof(server)) == -1) {
        fprintf(stderr, "[client4]\tError: Couldn't send to the server.");
        close(sd);
        exit(1);
    }

    char buf[4096];
    buflen = 4096;
    int char_rec;

    if (bFlag == 0) {
	    while ((char_rec = recvfrom(sd, buf, buflen, 0, NULL, NULL)) > 0) {
                puts("[client]\t Received");
    		count+=char_rec;
    	  
    		bzero(buf, buflen);
	    } 
    } else {
	    char client_buf[4096];
	    client_buf[0]= '\0';
            gettimeofday(&end, NULL);

    	while(1) {
    		
    		if (strlen(client_buf)==0) {
    			if ((char_rec = recvfrom(sd, buf, buflen, 0, NULL, NULL)) > 0) {
                    		puts("[client]\t Received");
                    //gettimeofday(&end, NULL);

    				//Received the first 10 bytes
    				memcpy(client_buf, &buf[10], char_rec-10);
    				client_buf[char_rec-10] = '\0'; //Null term			
    			} else {
    				puts("\nEnd");
                    		break;
    			}
                	bzero(buf, buflen);
    		} else {
                	puts("[client]\t Received");
    			//Still want the null terminator
    			memmove(client_buf, &client_buf[10], strlen(client_buf)-10+1);
    		}
            usleep((int)5*1000000);
    	}
    }
    close(sd);
    return 0;
}
