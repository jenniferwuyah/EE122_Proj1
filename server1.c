#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* this defines the size of our buffer */
#define MAX_NUM_CLIENTS 10

 int main(int argc, char** argv)
 {
 	int ret, listen_fd, comm_fd, client_len,port, char_read;
	//unsigned short port;
 	unsigned int mode, packet_size;
 	double packet_delay;
 	char *filename;
 	struct sockaddr_in servaddr, client;
 	struct sockaddr client_test;
 	char buf[7]; //used for connectless recv to establish connection

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

	printf("Filename is: %s\n", filename);
	printf("packet size %i\n", packet_size);

	/*Server set up*/
	if (mode==0) {
	 	listen_fd = socket(AF_INET, SOCK_STREAM,0);
	} else { //connectionless sockets
	 	listen_fd = socket(AF_INET, SOCK_DGRAM,0);
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
	while(1)
	{


		if (mode == 0) {
		 	comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		} else {		 	
		 	comm_fd = recvfrom(listen_fd, buf, 8 , 0,(struct sockaddr *) &client, (socklen_t *)&client_len); 
		 	client.sin_family = AF_INET;
		}
		if (comm_fd==-1) {
		 	printf("Error with accepting or recieving a client\n");
		 	exit(1);
		}

		printf("\n[server]\tGot a new client!\n");

		int file_handle = open(filename, O_RDONLY, S_IREAD);
		char *temp = "Initial";

		while(1)
		{
			bzero(buffer, packet_size);

			/* Read packet_size bytes from the file*/
		 	char_read = read(file_handle, buffer, packet_size-1);

		 	if(char_read ==0)
		 	{
				break; //Finish reading the file
			}
			/*(including null-terminator) */
			buffer[packet_size-1] = '\0';

			if (mode==0) {
				write(comm_fd, buffer, strlen(buffer));
			} else {
				sendto(listen_fd, buffer, strlen(buffer), 0, (struct sockaddr *) &client, client_len);
			} 
			/* delay */
			usleep(packet_delay * 1000000);
		}

		/*Send last empty packet for connectless to finish*/
		char *done = "";
		if (mode==1) {
			sendto(listen_fd, done, strlen(done), 0, (struct sockaddr *) &client, client_len);	
		} else {
			/* close the connection */
			close(comm_fd);
		}
		printf("[server]\tClient left.\n");
	}
	return 0;
}