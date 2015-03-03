#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/udp.h>   //Provides declarations for udp header

/* this defines the size of our buffer */
#define MAX_NUM_CLIENTS 10

// total udp header length: 8 bytes (=64 bits)
 
// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
	unsigned short csum(unsigned short *buf, int nwords)
	{       //
	        unsigned long sum;
	        for(sum=0; nwords>0; nwords--)
	            sum += *buf++;
	        sum = (sum >> 16) + (sum &0xffff);
	        sum += (sum >> 16);
	        return (unsigned short)(~sum);
	}

struct iphdr {
 unsigned int      ihl:5, version:4;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
};	

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
	} else if (mode==1) { //connectionless sockets
	 	listen_fd = socket(AF_INET, SOCK_DGRAM,0);
	} else {
		listen_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	}

	if (listen_fd ==- 1) {
	 	printf("Couldn't make a socket");
	 	exit(1);
	}

	/*Prepare server socket*/
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr.sin_port = htons(port);

	char buffer[packet_size]; //For sending packets
 	int bytes_to_send;

	struct iphdr *iph= (struct iphdr *) buffer;
	struct udphdr *udph = (struct udphdr *) (buffer + sizeof(struct iphdr));

	if (mode == 2) {
	
		//Fill in the IP Header
	    iph->ihl = 5;
	    iph->version = 4;
	    iph->tos = 0;
	    iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr);
	    iph->id = htonl (54321); //Id of this packet
	    iph->frag_off = 0;
	    iph->ttl = 255;
	    iph->protocol = IPPROTO_UDP;
	    iph->check = 0;      //Set to 0 before calculating checksum
	    iph->saddr = servaddr.sin_addr.s_addr;
	    iph->daddr = client.sin_addr.s_addr;

	    //UDP header
	    udph->uh_sport = htons (port);
	    udph->uh_dport = client.sin_port;
	    udph->uh_ulen = htons(8 + bytes_to_send); //udp header size
	    udph->uh_sum = 0; //leave checksum 0 now

		iph->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));
		// Inform the kernel do not fill up the packet structure. we will build our own...
		int one = 1;
		if(setsockopt(listen_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
		{
		perror("setsockopt() error");
		exit(-1);
		}
		else
		printf("setsockopt() is OK.\n");
	}

	if (bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr))==-1) {
	 	printf("Socket binding fails\n");
	 	close(listen_fd);
	 	exit(1);
	 }

	if (mode == 0) { //Only applies for connect ori
		listen(listen_fd, MAX_NUM_CLIENTS);
		printf("[server]\tlistening on port: %d\n", port);
	} 


	char str_buf[20];
	sprintf(str_buf, "%i", packet_size);
	for (int c =strlen(str_buf); c < 18; c+=1 ) {
		str_buf[c] = ' '; 
	}
	str_buf[19]= '\0';


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


		// str_buf[strlen(str_buf)] = '\0';
		// printf("buf length is %lu\n", strlen(str_buf));

		//Say the packet size
		if (mode==0) {	
			write(comm_fd, str_buf, strlen(str_buf));
			if (read(comm_fd, buf, 7) ==-1)
			{
				printf("ERROR GETTING ACK");
			}
		} else {
			sendto(listen_fd, str_buf, strlen(str_buf), 0, (struct sockaddr *) &client, client_len);
			if (recvfrom(listen_fd, buf, 8 , 0,NULL, NULL) == -1)
			{
				printf("ERROR GETTING ACK");

			}

		} 

		usleep(1);
		while(1)
		{
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




			if (mode==0) {				
				write(comm_fd, buffer, bytes_to_send);
			} else {
				sendto(listen_fd, buffer, bytes_to_send, 0, (struct sockaddr *) &client, client_len);
			} 
			/* delay */
			if (packet_delay != 0) {
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