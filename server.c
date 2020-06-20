//
// Created by omer.reuveni on 6/13/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_WRQ 512
#define MAX_STRING 254
#define MAX_DATA 512
#define DATA_PACKET_SIZE 516

// STRUCT DECLARATIONS
typedef struct Wrq_struct {
    uint16_t opcode;
    char filename[MAX_STRING + 1];
    char trans_mode[MAX_STRING + 1];

} __attribute__((packed)) Wrq_struct;

typedef struct ack_struct {
    uint16_t opcode, block_num;
} __attribute__((packed)) ack_struct;

typedef struct data_struct {
    uint16_t opcode, block_num;
    char data[MAX_DATA];
} __attribute__((packed)) data_struct;


//  function headers
void recieveData(int socketfd, struct sockaddr_in *client_addr, socklen_t client_addr_length, FILE *fptr);

void WRQ_parser(char *buffer, Wrq_struct *Wrq);

void ACK_response(int socketfd, int ack_num, struct sockaddr_in *client, socklen_t client_addr_length);


int main(int argc, char **argv) {

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
    struct sockaddr_in my_addr, client_addr = {0};
    Wrq_struct Wrq = {0};
    socklen_t client_addr_len = sizeof(client_addr);
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
    if (bind(socketfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1) // case bind or listen failed
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
        /**if (recvMsgSize =
                    recvfrom(socketfd, Wrq_buffer, MAX_WRQ, 0, (struct sockaddr *) &client_addr_len, &client_addr_len) <0)**/
        recvMsgSize = recvfrom(socketfd, Wrq_buffer, MAX_WRQ, 0, (struct sockaddr *) &client_addr_len, &client_addr_len);
        if (recvMsgSize<0)
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
        printf("IN:WRQ, %s, %s\nWhere %s and %s are values of appropriate fields in the packet.\n", Wrq.filename,
               Wrq.trans_mode, Wrq.filename, Wrq.trans_mode);
        fflush(stdout);

        FILE *fptr = fopen(Wrq.filename, "w");
        if (fptr == NULL)
        {
            //  generic system-call failure message
            perror("TTFTP_ERROR: ");
            fflush(stdout);
            continue;
        }

        //  respond WRQ with ACK
        ACK_response(socketfd,0,  &client_addr, client_addr_len);
        recieveData(socketfd, &client_addr, client_addr_len, fptr);

        if (fclose(fptr) != 0)
        {
            //  generic system-call failure message
            perror("TTFTP_ERROR: ");
            fflush(stdout);
        }
    }

}




/** upon recieiving WRQ this func gets the data from the buffer and parsing it to our struct
 * @param buffer - raw data
 * @param Wrq - structered data
 */
void WRQ_parser(char *buffer, Wrq_struct *Wrq) {
    memcpy(&Wrq->opcode, buffer, 2);
    Wrq->opcode = ntohs(Wrq->opcode);    // fix network byte order
    int filename_len = strlen(buffer + 2);
    strncpy(Wrq->filename, buffer + 2, filename_len);    // copy filename
    if (strrchr(Wrq->filename, '/'))    // remove full path if exists
        strcpy(Wrq->filename, strrchr(Wrq->filename, '/') + 1);
    strncpy(Wrq->trans_mode, buffer + 2 + filename_len + 1,
            strlen(buffer + 2 + filename_len + 1));    // copy transmission mode

}

/**
 * Once we start communicate with the client and processed the WRQ
 * writing the input data to fptr
 */
void recieveData(int socketfd, struct sockaddr_in *client_addr, socklen_t client_addr_length, FILE *fptr) {
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
                    recvMsgSize = recvfrom(socketfd, &data, DATA_PACKET_SIZE, 0, (struct sockaddr *) client_addr,&client_addr_length);
                    if (recvMsgSize < 0)
                    {
                        // generic system-call failure message
                        perror("TTFTP_ERROR: ");
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
            printf("RECVFAIL\n");
            fflush(stdout);
            return;
        }


        // case writing is successful
        // respond the client with an ACK
        ack_num++;
        ACK_response(socketfd, ack_num, client_addr, client_addr_length);


    }while (recvMsgSize >= DATA_PACKET_SIZE);  // means we still have left blocks from the client to read

    //  all data was transmitted
    printf("RECVOK\n");
    fflush(stdout);
}


/**responding by sending ACK
 * sending an ACK with the proper ack_num to the client-side.
 * function ends only after it was able to send the ACK successfully (otherwise it'll keep trying).
 * @param ack_num - packet num we acknowledge
 * @param socketfd, client, client_addr_length - are network parameters
 */
void ACK_response(int socketfd, int ack_num, struct sockaddr_in *client_addr, socklen_t client_addr_length) {
    ack_struct ack = {htons(4), htons(ack_num)}; // create an ack for sending
    int bytes_sent = 0;
    do
    { // keep trying to send the ack, until it succeeds
        bytes_sent = sendto(socketfd, &ack, sizeof(ack), 0,
                            (struct sockaddr_in *) client_addr, client_addr_length);
        if (bytes_sent < 0) // sending of ack has falied
            perror("TTFTP_ERROR: ");
    } while (bytes_sent != sizeof(ack));
    // success!! you are a lucky person
    printf("OUT:ACK, %d\n", ack_num);
    fflush(stdout);
}
