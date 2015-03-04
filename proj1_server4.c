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

 	int listen_fd, comm_fd, client_len, port;
 	double packet_delay;
 	struct sockaddr_in server, client;
 	char buf[7];
 	char buffer[PACKET_SIZE] = "12345678901234567890"; //used for connectless recv to establish connection

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
		fprintf(stderr, "[server4]\tError: Invalid port number <%d>.\n", port);
		return 1; /* failure */
	}

	/*Server set up*/
	//connectionless sockets

	if ((listen_fd = socket(AF_INET, SOCK_DGRAM,0)) == -1) {
	 	printf("[server4]\tError: Couldn't make a socket.\n");
	 	exit(1);
	}

	/*Prepare server socket*/
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr*) &server, sizeof(server))== -1) {
	 	printf("[server4]\tError: Socket binding failed.\n");
	 	close(listen_fd);
	 	exit(1);
	 }


	printf("[server4]\tWaiting for clients at port <%d>.\n", port);


	while(1)
	{
		memset(&client, 0, sizeof(struct sockaddr_in));
		client_len = sizeof(client);
	 	if ((comm_fd = recvfrom(listen_fd, buf, 8 , 0,(struct sockaddr *) &client, (socklen_t *)&client_len)) == -1) {
		 	printf("[server4]\tError: Cannot receive client.\n");
		 	exit(1);
		}
	 	client.sin_family = AF_INET;


		printf("\n[server4]\tGot a new client!\n");


		int p;
		for (p = 0; p < 10; p++) { // send 10 packets total

			//set packet delay to random double between 1 and 10
			packet_delay = (rand() / (double)(RAND_MAX/10)) ;

			if (sendto(listen_fd, buffer, PACKET_SIZE, 0, (struct sockaddr *) &client, client_len) != PACKET_SIZE) {
				printf("[server4]\tError: %s.\n", perror('sendto'));
			}
 
			/* delay */
			if (packet_delay > 0) {		
				//printf("delay for %f sec\n", packet_delay);	
				usleep((int)(packet_delay * 1000000));
			}
		}

		/*Send last empty packet for connectless to finish*/
		char *done = "";
		sendto(listen_fd, done, strlen(done), 0, (struct sockaddr *) &client, client_len);	

		printf("[server4]\tClient left.\n");
	}
	return 0;
}