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
 	int listen_fd, comm_fd, client_len, port, char_read, bytes_to_send;
 	unsigned int mode, packet_size;
 	double packet_delay;
 	char *filename;
 	struct sockaddr_in server, client;
 	char buf[7]; //used for connectless recv to establish connection
	
	memset(&server, 0, sizeof(struct sockaddr_in));



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

	if(port < 1024) {
		fprintf(stderr, "[server]\tError: Invalid port number <%d>.\n", port);
		return 1; /* failure */
	}

	/*Server set up*/
	if (mode==0) {
	 	listen_fd = socket(AF_INET, SOCK_STREAM,0);
	} else if (mode == 1 ){ //connectionless sockets
	 	listen_fd = socket(AF_INET, SOCK_DGRAM,0);
	} else {
	 	/*listen_fd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDPLITE);
	 	int optval=1;
		setsockopt(listen_fd, SOL_SOCKET, SO_NO_CHECK , (void*)&optval, sizeof(optval));*/
	}


	if (listen_fd ==- 1) {
	 	printf("[server]\tError: Couldn't make a socket.\n");
	 	exit(1);
	}

	/*Prepare server socket*/
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr*) &server, sizeof(server))==-1) {
	 	printf("[server]\tError: Socket binding fails.\n");
	 	close(listen_fd);
	 	exit(1);
	 }

	if (mode == 0) { //Only applies for connect ori
		listen(listen_fd, MAX_NUM_CLIENTS);
		printf("[server]\tlistening on port <%d>\n", port);
	} 

	char buffer[packet_size]; //For sending packets

	printf("[server]\tWaiting for clients at port <%d>.\n", port);

	int packetno = 1;

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
		 	printf("[server]\tError: Cannot receive client.\n");
		 	exit(1);
		}

		printf("\n[server]\tGot a new client!\n");

		int file_handle = open(filename, O_RDONLY, S_IRUSR);

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
				printf("[server4]\tError: Cannot get ACK\n");
			}
		} 

		while(1) {
			bzero(buffer, packet_size);
			/* Read packet_size bytes from the file*/

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

			if (mode==0) {				
				write(comm_fd, buffer, bytes_to_send);
			} else {
				if (sendto(listen_fd, buffer, bytes_to_send, 0, (struct sockaddr *) &client, client_len) != bytes_to_send) {
					printf("[server]\tError: Failed sending packet.\n");
					perror("sendto");
				}
			}
 
			/* delay */
			// if (packet_delay == 0) {
			// 	packet_delay= 0.000001; 
			// }
			if (packet_delay>0) {
				usleep((int)(packet_delay * 1000000));
			}
			packetno++;
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
