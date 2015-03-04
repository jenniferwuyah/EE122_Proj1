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
#define PACKET_SIZE 20

 int main(int argc, char** argv)
 {
 	//set up random number generator
 	time_t t;
 	srand((unsigned) time(&t));

 	int ret, listen_fd, comm_fd, client_len, port, char_read;
	//unsigned short port;
 	unsigned int packet_size;
 	double packet_delay;
 	struct sockaddr_in server, client;
 	char buf[7];
 	char buffer[PACKET_SIZE] = "12345678901234567890"; //used for connectless recv to establish connection
 	int bytes_to_send;

 	if(argc != 2 || !strcmp(argv[1], "-h"))
 	{
		/* incorrect number of arguments given or help flag given.
		 * Print usage */
		 printf(" Usage:\n\n"
		 	"\t%s <port>\n\n"
		 	" This server will do as the hw instruction.\n\n",argv[0]);
		return 1; /* failure */
	}

	/* Parse args */
	port = atoi(argv[1]);
	if(port < 1024)
	{
		fprintf(stderr, "Invalid port number: %d\n", port);
		return 1; /* failure */
	}

	



	// printf("Filename is: %s\n", filename);
	// printf("packet size %i\n", packet_size);

	/*Server set up*/
	//connectionless sockets

	if ((listen_fd = socket(AF_INET, SOCK_DGRAM,0)) == -1) {
	 	printf("Couldn't make a socket");
	 	exit(1);
	}

	/*Prepare server socket*/
	//bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr*) &server, sizeof(server))== -1) {
	 	printf("Socket binding fails\n");
	 	perror("bind");
	 	close(listen_fd);
	 	exit(1);
	 }


	printf("[server]\tWaiting for clients at port: %d\n", port);


	while(1)
	{
		memset(&client, 0, sizeof(struct sockaddr_in));
		client_len = sizeof(client);
	 	if ((comm_fd = recvfrom(listen_fd, buf, 8 , 0,(struct sockaddr *) &client, (socklen_t *)&client_len)) == -1) {
		 	printf("Error with accepting or recieving a client\n");
		 	exit(1);
		}
	 	client.sin_family = AF_INET;


		printf("\n[server]\tGot a new client!\n");


		// str_buf[strlen(str_buf)] = '\0';
		// printf("buf length is %lu\n", strlen(str_buf));

		//Say the packet size for connection oriented

		int p;
		for (p = 0; p < 10; p++) { // send 10 packets total
			//set packet delay to random int between 1 and 10
			packet_delay = (rand() / (double)(RAND_MAX/10)) ;
			/* Read packet_size bytes from the file*/
		 	// char_read = read(file_handle, buffer, packet_size-1);

		 // 	char_read = read(file_handle, buffer, packet_size);

		 // 	if(char_read ==0)
		 // 	{
			// 	break; //Finish reading the file
			// }
			//Last packet should end in null character so easier for writing later.
			
			// /*If not packet size then include null terminator to know when to stop*/
			// if (char_read!=packet_size) {
			// 	buffer[char_read] = '\0';
			// 	bytes_to_send = char_read +1;
			// } else {
			// 	bytes_to_send = char_read;
			// }

			if (sendto(listen_fd, buffer, PACKET_SIZE, 0, (struct sockaddr *) &client, client_len) != PACKET_SIZE) {
				printf("\n!!!!!!!!!!!!!!!!!!! NO BUENO!!!!!!!!!!!!!!!!!!!!!!\n");
				perror("sendto");
			}
 
			/* delay */
			if (packet_delay > 0) {		
				printf("delay for %f sec\n", packet_delay);	
				usleep((int)(packet_delay * 1000000));
			}
			puts("woke up");
		}

		/*Send last empty packet for connectless to finish*/
		char *done = "";
		sendto(listen_fd, done, strlen(done), 0, (struct sockaddr *) &client, client_len);	

		printf("[server]\tClient left.\n");
	}
	return 0;
}
