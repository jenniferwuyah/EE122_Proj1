#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

/* this defines the size of our buffer */
#define MAX_NUM_CLIENTS 10

 int main(int argc, char** argv)
 {
 	int ret, listen_fd, comm_fd, client_len, port, char_read;
	//unsigned short port;
 	unsigned int mode, packet_size;
 	double packet_delay;
 	char *filename;
 	struct sockaddr_in servaddr, client;
 	struct sockaddr client_test;
 	char buf[7]; //used for connectless recv to establish connection
 	int bytes_to_send;

 	if(argc != 6 || !strcmp(argv[1], "-h"))
 	{
		/* incorrect number of arguments given or help flag given.
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

	/* Parse args */
	port = atoi(argv[2]);
	mode = atoi(argv[1]);
	filename = argv[3];
	packet_size = atoi(argv[4]);
	packet_delay = atof(argv[5]);

	if(port < 1024)
	{
		fprintf(stderr, "Invalid port number: %d\n", port);
	return 1; /* failure */
	}

	// printf("Filename is: %s\n", filename);
	// printf("packet size %i\n", packet_size);

	/*Server set up*/
	if (mode==0) {
	 	listen_fd = socket(AF_INET, SOCK_STREAM,0);
	} else if (mode == 1 ){ //connectionless sockets
	 	listen_fd = socket(AF_INET, SOCK_DGRAM,0);
	} else {
	 	listen_fd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDPLITE);
	}

	if (mode==2) {
		int optval=1;
		setsockopt(listen_fd, SOL_SOCKET, SO_NO_CHECK , (void*)&optval, sizeof(optval));
		// setsockopt(listen_fd, SOL_SOCKET, SO_UDPCKSUM_IN);
	}

	if (listen_fd ==- 1) {
	 	printf("Couldn't make a socket");
	 	exit(1);
	}

	/*Prepare server socket*/
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr))==-1) {
	 	printf("Socket binding fails\n");
	 	close(listen_fd);
	 	exit(1);
	 }

	if (mode == 0) { //Only applies for connect ori
		listen(listen_fd, MAX_NUM_CLIENTS);
		printf("[server]\tlistening on port: %d\n", port);
	} 

	char buffer[packet_size]; //For sending packets

	printf("[server]\tWaiting for clients at port: %d\n", port);


	while(1)
	{
		if (mode == 0) {
		 	comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		} else {
			memset(&client, 0, sizeof(struct sockaddr_in));
			client_len = sizeof(client);
		 	comm_fd = recvfrom(listen_fd, buf, 8 , 0,(struct sockaddr *) &client, (socklen_t *)&client_len); 
		 	client.sin_family = AF_INET;
		}
		if (comm_fd==-1) {
		 	printf("Error with accepting or recieving a client\n");
		 	exit(1);
		}

		printf("\n[server]\tGot a new client!\n");

		int file_handle = open(filename, O_RDONLY, S_IRUSR);
		printf("%d\n", client.sin_port);
		printf("client size? %d", sizeof(client));

		// str_buf[strlen(str_buf)] = '\0';
		// printf("buf length is %lu\n", strlen(str_buf));

		//Say the packet size for connection oriented
		if (mode==0) {	
			char str_buf[20];
			sprintf(str_buf, "%i", packet_size);
			int c;
			for (c =strlen(str_buf); c < 18; c+=1 ) {
				str_buf[c] = ' '; 
			}
			str_buf[19]= '\0';
			write(comm_fd, str_buf, strlen(str_buf));
			if (read(comm_fd, buf, 7) ==-1) {
				printf("ERROR GETTING ACK");
			}
		} /*else {
			sendto(listen_fd, str_buf, strlen(str_buf), 0, (struct sockaddr *) &client, client_len);
			if (recvfrom(listen_fd, buf, 8 , 0,NULL, NULL) == -1)
			{
				printf("ERROR GETTING ACK");

			}

		}*/ 

		while(1) {
			bzero(buffer, packet_size);

			/* Read packet_size bytes from the file*/
		 	// char_read = read(file_handle, buffer, packet_size-1);

		 	char_read = read(file_handle, buffer, packet_size);

		 	if(char_read ==0)
		 	{
				break; //Finish reading the file
			}
			//Last packet should end in null character so easier for writing later.
			
			/*If not packet size then include null terminator to know when to stop*/
			if (char_read!=packet_size) {
				buffer[char_read] = '\0';
				bytes_to_send = char_read +1;
			} else {
				bytes_to_send = char_read;
			}
			puts(buffer);
			puts("*****************************************");

			if (mode==0) {				
				write(comm_fd, buffer, bytes_to_send);
			} else {
				int send_suc;
				send_suc = sendto(listen_fd, buffer, bytes_to_send, 0, (struct sockaddr *) &client, client_len);
				//puts("just sent yes");				
//printf("just sent %i\n", send_suc);
				if (send_suc < 0) {
					printf("\n!!!!!!!!!!!!!!!!!!! NO BUENO!!!!!!!!!!!!!!!!!!!!!!\n");
					perror("sendto");
				}
			}
 
			/* delay */
			if (packet_delay>0) {			
				usleep(packet_delay * 1000000);
			}
		}

		/*Send last empty packet for connectless to finish*/
		char *done = "";
		if (mode==1) {
			sendto(listen_fd, done, strlen(done), 0, (struct sockaddr *) &client, client_len);	
		} else {
			/* close the connection */
			close(comm_fd);
		}
		close(file_handle);
		printf("[server]\tClient left.\n");
	}
	return 0;
}
