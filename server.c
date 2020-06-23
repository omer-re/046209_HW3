
#include "server.h" 

/** upon recieiving WRQ this func gets the data from the buffer and parsing it to our struct
 * @param buffer - raw data
 * @param Wrq - structered data
 */
void WRQ_parser(char* buffer, Wrq_struct* Wrq) {
	memcpy(&Wrq->opcode, buffer, 2);
	Wrq->opcode = ntohs(Wrq->opcode);    // fix network byte order
	int filename_len = strlen(buffer + 2);
	strncpy(Wrq->filename, buffer + 2, filename_len);    // copy filename
	if (strrchr(Wrq->filename, '/'))    // remove full path if exists
		strcpy(Wrq->filename, strrchr(Wrq->filename, '/') + 1);
	strncpy(Wrq->trans_mode, buffer + 2 + filename_len + 1,
		strlen(buffer + 2 + filename_len + 1));    // copy transmission mode

}



/**responding by sending ACK
 * sending an ACK with the proper ack_num to the client-side.
 * function ends only after it was able to send the ACK successfully (otherwise it'll keep trying).
 * @param ack_num - packet num we acknowledge
 * @param socketfd, client, client_addr_length - are network parameters
 */
void ACK_response(int socketfd, int ack_num, struct sockaddr_in* client_addr, socklen_t client_addr_length) {
	ack_struct ack = { htons(4), htons(ack_num) }; // create an ack for sending
	int bytes_sent = 0;
	do
	{ // keep trying to send the ack, until it succeeds
		bytes_sent = sendto(socketfd, &ack, sizeof(ack), 0, (struct sockaddr*) client_addr, client_addr_length);
		if (bytes_sent < 0)
		{ // sending of ack has falied
			perror("TTFTP_ERROR: ");
			perror("339\n");  //TODO remove
		}
	} while (bytes_sent != sizeof(ack));
	// success!! you are a lucky person
	printf("OUT:ACK, %d\n", ack_num);
	fflush(stdout);
}





/**
 * Once we start communicate with the client and processed the WRQ
 * writing the input data to fptr
 */
void recieveData(int socketfd, struct sockaddr_in* client_addr, socklen_t client_addr_length, FILE* fptr) {
	const int WAIT_FOR_PACKET_TIMEOUT = 3;
	const int NUMBER_OF_FAILURES = 7;
	fd_set fd_recieve;
	struct timeval timeout;  // as recommended on pdf
	int num_of_ready = 0;
	int num_of_timeoutExpired = 0;
	int ack_num = 0;
	int recvMsgSize = 0;
	data_struct data;

	FD_ZERO(&fd_recieve); //  Initializes the file descriptor set fdset to have zero bits for all file descriptors.
	FD_SET(socketfd, &fd_recieve);  //  Sets the bit for the file descriptor fd in the file descriptor set fdset.


	//  given skeleton



	do
	{
		memset(&data, 0, sizeof(data));  //  recieve new input data packets
		do
		{
			do
			{

				/*************************************************************/
				/* Initialize the timeval struct to 3 minutes.  If no        */
				/* activity after 3 minutes this program will end.           */
				/*************************************************************/
				timeout.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
				timeout.tv_usec = 0;

				/**********************************************************/
				/* Call select() and wait 3 minutes for it to complete.   */
				/**********************************************************/
				num_of_ready = select(socketfd + 1, &fd_recieve, NULL, NULL, &timeout);

				/**********************************************************/
				/* Check to see if the 3 minute time out expired.         */
				/**********************************************************/
				if (num_of_ready == 0)
				{  //  there was a data at the socket
					//  resend last ACK
					printf("FLOWERROR: timeout expired. packet did not arrive on time\n");
					fflush(stdout);
					num_of_timeoutExpired++;
					ACK_response(socketfd, ack_num, client_addr, client_addr_length);
				}

				/**********************************************************/
				/* Check to see if the select call succeeded.
				 * timeout wasn't a problem*/
				 /**********************************************************/
				if (num_of_ready > 0)
				{
					//  process the data packet from the socket
					recvMsgSize = recvfrom(socketfd, &data, DATA_PACKET_SIZE, 0, (struct sockaddr*) client_addr, &client_addr_length);
					if (recvMsgSize < 0)
					{
						// generic system-call failure message
						perror("TTFTP_ERROR: ");
						perror("242\n");  //TODO remove
						fflush(stdout);
						continue;
					}
					else
					{ //  convert to endian
						data.opcode = ntohs(data.opcode);
						data.block_num = ntohs(data.block_num);
						break; // packet been received, go process it.
					}
				}

				if (num_of_timeoutExpired >= NUMBER_OF_FAILURES)  // reached maximum timeouts
				{// FATAL ERROR BAIL OUT
					printf("FLOWERROR: exceeded number of allowed timeouts, bailing out...\n");
					printf("RECVFAIL\n");
					fflush(stdout);
					return;
				}
			} while (1); // keep running as long sockets are ready, but recvfrom failed.

			if (data.opcode != 3)  // this isn't data
			{
				// FATAL ERROR BAIL OUT
				printf("RECVFAIL\n");
				fflush(stdout);
				return;
			}

			if (data.block_num != ack_num + 1)
			{  //  that is not the block we expected, but this is a valid data packet

				// FATAL ERROR BAIL OUT
				printf("FLOWERROR: block number does not agree with expected\n");
				printf("RECVFAIL\n");
				fflush(stdout);
				return;
			}
		} while (false); // TODO: wtf is this condition?!?!
		/***********************************************************************/
		/* A do/while(FALSE) loop is used to make error cleanup easier.  The   */
		/* close() of each of the socket descriptors is only done once at the  */
		/* very end of the program.                                            */
		/***********************************************************************/


		//  this is where we know that our packet is valid and matches expectations
		printf("IN:DATA, %d, %d\n", data.block_num, recvMsgSize);
		fflush(stdout);
		num_of_timeoutExpired = 0;

		//  print all but header
		printf("WRITING: %d\n", recvMsgSize - 4);
		fflush(stdout);

		if (fwrite(data.data, 1, recvMsgSize - 4, fptr) != recvMsgSize - 4)
		{    // case it failed to write the data to the file
			// FATAL ERROR BAIL OUT
			perror("TTFTP_ERROR: ");
			perror("301\n");  //TODO remove

			printf("RECVFAIL\n");
			fflush(stdout);
			return;
		}


		// case writing is successful
		// respond the client with an ACK
		ack_num++;
		ACK_response(socketfd, ack_num, client_addr, client_addr_length);


	} while (recvMsgSize >= DATA_PACKET_SIZE);  // means we still have left blocks from the client to read

	//  all data was transmitted
	printf("RECVOK\n");
	fflush(stdout);
}
