#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define MAXLEN 4096
#define TIMEOUT_SEC 30

int main(int argc, char** argv)
{
    int sd, buflen;
    unsigned short port;
    unsigned int mode;
    char *filename, *stats_filename, *server_address;
    struct sockaddr_in server;
    char time_str[100];
    struct timeval start, end, conn_start, conn_end;
    float sec_delay;
    char str_buf[20]; //Assume largest packet is only 20 digits
    int packet_size;

    memset(&server, 0, sizeof(struct sockaddr_in));

    memset(&start, 0, sizeof(struct timeval));
    memset(&end, 0, sizeof(struct timeval));
    memset(&conn_start, 0, sizeof(struct timeval));
    memset(&conn_end, 0, sizeof(struct timeval));

    char *temp = "Initial";

    FILE * fp;
    FILE * stat_fp;

    if(argc != 6 || !strcmp(argv[1], "-h")) {
        /* incorrect number of arguments given or help flag given.
         * Print usage */
        printf(" Usage:\n\n"
               "\t%s <mode>\n\n"
               "\t <server_address>\n\n"
               "\t <port>\n\n"
               "\t <received_filename>\n\n"
               "\t <stats_filename>\n\n"
               " This client will do as the hw instruction.\n\n",argv[0]);
       return 1; /* failure */
    }

    /* Parse args */
    port = atoi(argv[3]);
    mode = atoi(argv[1]);
    filename = argv[4];
    stats_filename = argv[5];
    server_address = argv[2];

    if(port < 1024) {
        //fprintr(stderr, "[client]\tError: Invalid port number <%d>.\n", port);
        //fprintf(stderr, "\t\t(Only accepts ports over 1000)\n");
        return 1; /* failure */
    }

    //printf("Filename is: %s\n", filename);

    if (mode ==0) {
        sd = socket(AF_INET, SOCK_STREAM,0);
    } else if (mode == 1){
        sd = socket(AF_INET, SOCK_DGRAM,0);
    } else {
        /*sd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDPLITE);
        int optval=1;
        setsockopt(sd, SOL_SOCKET, SO_NO_CHECK , (void*)&optval, sizeof(optval));
   */ }
    if (sd == -1) {
        fprintf(stderr, "[client]\tError: Can't create a socket.\n");
        exit(1); 
    }

    if (mode != 0) {
        struct timeval timeout;  
        memset(&timeout, 0, sizeof(struct timeval));
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if (setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) { }
            //error("setsockopt failed\n");
    }


    bzero((char *)&server, sizeof(server));
    inet_pton(AF_INET, server_address, &(server.sin_addr));
    server.sin_port = htons(port);
    server.sin_family = AF_INET;


    /* Connecting to the server */

    int first_pkt = 1; //For delay timing
    int count =0;
    int n =0;

    if (mode==0) {
        if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
                printf("[client]\tError: Couldn't connect to server.\n");
                close(sd);
                exit(1);
        }

        gettimeofday(&conn_start, NULL);
        
        // printf("Connected: server's address is %s\n", inet_ntoa(server.sin_addr));
        fp = fopen (filename, "w");
        stat_fp = fopen (stats_filename, "w");

        //getting packet size first
        if (read(sd, str_buf, 20) > 0) {
            // printf("client got %s\n", str_buf);
            packet_size = atoi(str_buf);
            // printf("packet size is %i\n", packet_size);
        }

        //ACK packet size. 
        if (write(sd, temp, strlen(temp)) == -1) {
            printf("[client]\tError: Couldn't send to the server.\n");
            close(sd);
            exit(1);
        }

        char buf[packet_size];
        buflen = packet_size;

        while (recv(sd, buf, packet_size,MSG_WAITALL) > 0) {
            gettimeofday(&end, NULL);
            if (first_pkt!=1) {
                sec_delay = (float)(end.tv_sec - start.tv_sec) + ((float)end.tv_usec - (float)start.tv_usec)/1000000 ;
                sprintf(time_str, "%f\n",sec_delay);
                fputs(time_str, stat_fp); //Write into stat file:
                bzero(time_str, 100);
            } else {
                first_pkt = 0;
            }
            buf[packet_size] = '\0';
            n = fputs(buf, fp); //Write into file:
            count+=strlen(buf);
            bzero(buf, buflen);
            gettimeofday(&start, NULL);
        } 
    } else { // connectionless
        //Talk to server to begin. 
        gettimeofday(&conn_start, NULL);
    
        //puts("sending to server");
        if (sendto(sd, temp, strlen(temp) , 0, (struct sockaddr *) &server, sizeof(server)) == -1) {
            fprintf(stderr, "[client4]\tError: Couldn't send to the server.\n");
            close(sd);
            exit(1);
        }
        //puts("just sent to server");
        fp = fopen (filename, "w");
        stat_fp = fopen (stats_filename, "w");

        char buf[MAXLEN];
        buflen = MAXLEN;
        int char_rec;
        //puts("character being recv");
        int packetno = 1;
        while ((char_rec = recvfrom(sd, buf, buflen, 0, NULL, NULL)) > 0) {
            gettimeofday(&end, NULL);
            if (first_pkt!=1) {
                sec_delay = (float)(end.tv_sec - start.tv_sec) + (((float)end.tv_usec - (float)start.tv_usec)/1000000);
                sprintf(time_str, "%f",sec_delay);
                fputs(time_str, stat_fp); //Write into stat file
                bzero(time_str, 100);
            } else {
                first_pkt = 0;
            }
            count+=char_rec;
            fwrite(buf, 1, char_rec, fp);
            bzero(buf, buflen);
            gettimeofday(&start, NULL);
            packetno++;
        } 
        

    }
    gettimeofday(&conn_end, NULL);
    sec_delay = (float)(conn_end.tv_sec - conn_start.tv_sec) + ((float)conn_end.tv_usec - (float)conn_start.tv_usec)/1000000 ;
    fprintf(stderr, "[client]\t Connection lasted %f seconds\n", sec_delay);
    fprintf(stderr, "[client]\t Received a file of size %i bytes\n",count );
    fclose(stat_fp);
    fclose(fp);
    close(sd);
    return 0;
}
