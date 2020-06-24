//
// Created by omer.reuveni on 6/13/2020.
//


#include "server.h" 

// STRUCT DECLARATIONS


int main(int argc, char** argv) {

	// verify arg num
	if (argc < 2)
	{
		printf("ERROR: too few arguments\n");  //Lior said we can phrase whatever message we want
		fflush(stdout);
		exit(1);
	}

	// establish new socket
	int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	int recvMsgSize = 0;
	// sockaddr_in is a netinet lib struct
	/**
	 * struct sockaddr_in {
	short            sin_family;   // e.g. AF_INET
	unsigned short   sin_port;     // e.g. htons(3490)
	struct in_addr   sin_addr;     // see struct in_addr, below
	char             sin_zero[8];  // zero this if you want to
	};

	struct in_addr {
		unsigned long s_addr;  // load with inet_aton()
	};
	 */
	struct sockaddr_in my_addr, client_addr = { 0 };
	Wrq_struct Wrq = { 0 };
	socklen_t client_addr_length = sizeof(client_addr);
	char Wrq_buffer[MAX_WRQ];

	// initialize my_addr
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	if (socketfd == -1)  // case failure
	{
		perror("TTFTP_ERROR: ");
		fflush(stdout);
		exit(1);
	}

	// binding socket to the requested port
	if (bind(socketfd, (struct sockaddr*) & my_addr, sizeof(my_addr)) == -1) // case bind or listen failed
	{
		close(socketfd);
		perror("TTFTP_ERROR: ");  // as defined for system call failure
		fflush(stdout);
		exit(1);
	}
	/** wait for requests,
	 * validate requests,
	 * respond accordingly (ACK) and process
	 */
	while (1)
	{
		//  clear buffers to ensure valid data
		memset(&Wrq_buffer, 0, MAX_WRQ);
		memset(&Wrq, 0, sizeof(Wrq_struct));

		/* Block until receive message from a client. Networking tutorial P.58*/
		recvMsgSize = recvfrom(socketfd, Wrq_buffer, MAX_WRQ, 0, (struct sockaddr*) & client_addr, &client_addr_length);
		if (recvMsgSize < 0)
		{
			perror("TTFTP_ERROR: ");
			fflush(stdout);
			continue;
		}
		//  struct the WRQ
		WRQ_parser(Wrq_buffer, &Wrq);

		// validate Wrq
		if (Wrq.opcode != 2 || strcmp(Wrq.trans_mode, "octet"))
		{  //  invalid Wrq
			continue;   // todo: check if there are defined error messages or actions for this case
		}

		// the packet is a valid WRQ
		printf("IN:WRQ, %s, %s\n", Wrq.filename, Wrq.trans_mode);
		fflush(stdout);

		FILE* fptr = fopen(Wrq.filename, "w");
		if (fptr == NULL)
		{
			//  generic system-call failure message
			perror("TTFTP_ERROR: ");
			fflush(stdout);
			continue;
		}

		//  respond WRQ with ACK
		ACK_response(socketfd, 0, &client_addr, client_addr_length);
		recieveData(socketfd, &client_addr, client_addr_length, fptr);

		if (fclose(fptr) != 0)
		{
			//  generic system-call failure message
			perror("TTFTP_ERROR: ");
			fflush(stdout);
		}
	}

}

