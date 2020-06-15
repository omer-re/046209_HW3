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
typedef struct wrq_struct {
    uint16_t opcode;
    char filename[MAX_STRING + 1];
    char trans_mode[MAX_STRING + 1];

} __attribute__((packed)) wrq_struct;

typedef struct ack_struct {
    uint16_t opcode, block_num;
} __attribute__((packed)) ack_struct;

typedef struct data_struct {
    uint16_t opcode, block_num;
    char data[MAX_DATA];
} __attribute__((packed)) data_struct;


//  function headers
void recieveData(int socketfd, struct sockaddr_in *client_addr, socklen_t client_addr_length, FILE *fptr);

void WRQ_parser(char *buffer, wrq_struct *wrq);

void ACK_response(int socketfd, int ack_num, struct sockaddr_in *client_addr, socklen_t client_addr_length);


int main(int argc, char **argv) {

    // verify arg num
    if (argc < 2) {
        printf("Expecting more arguments\n");  //TODO: check if there's a specific error message defined. couldn't find any.
        fflush(stdout);
        exit(1);
    }

    // establish new socket
    int socketfd = socket(AF_INET, SOCKET_DGRAM, 0);
    int num_bytes_recv = 0;
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
    wrq_struct Wrq = {0};
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
//  given skeleton

    const int WAIT_FOR_PACKET_TIMEOUT = 3;
    const int NUMBER_OF_FAILURES = 7;
    do {
        do {
            do {
                // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                // for us at the socket (we are waiting for DATA)
                if ()// TODO: if there was something at the socket and
                    // we are here not because of a timeout
                {
                    // TODO: Read the DATA packet from the socket (at
                    // least we hope this is a DATA packet)
                }
                if (...) // TODO: Time out expired while waiting for data
                // to appear at the socket
                {
                    //TODO: Send another ACK for the last packet
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount >= NUMBER_OF_FAILURES) {
                    // FATAL ERROR BAIL OUT
                }
            } while (...) // TODO: Continue while some socket was ready
            // but recvfrom somehow failed to read the data
            if (...) // TODO: We got something else but DATA
            {
                // FATAL ERROR BAIL OUT
            }
            if (...) // TODO: The incoming block number is not what we have
            // expected, i.e. this is a DATA pkt but the block number
            // in DATA was wrong (not last ACK’s block number + 1)
            {
                // FATAL ERROR BAIL OUT
            }
        } while (FALSE);
        timeoutExpiredCount = 0;
        lastWriteSize = fwrite(...); // write next bulk of data
        // TODO: send ACK packet to the client

    } while (...); // Have blocks left to be read from client (not end of transmission)


}

/** upon recieiving WRQ this func gets the data from the buffer and parsing it to our struct
 * @param buffer - raw data
 * @param wrq - structered data
 */
void WRQ_parser(char *buffer, wrq_struct *wrq) {
    memcpy(&wrq->opcode, buffer, 2);
    wrq->opcode = ntohs(wrq->opcode);    // fix network byte order
    int filename_len = strlen(buffer + 2);
    strncpy(wrq->filename, buffer + 2, filename_len);    // copy filename
    if (strrchr(wrq->filename, '/'))    // remove full path if exists
        strcpy(wrq->filename, strrchr(wrq->filename, '/') + 1);
    strncpy(wrq->trans_mode, buffer + 2 + filename_len + 1,
            strlen(buffer + 2 + filename_len + 1));    // copy transmission mode

}


void recieveData(int socketfd, struct sockaddr_in *client_addr, socklen_t client_addr_length, FILE *fptr) {}

/**responding by sending ACK
 * sending an ACK with the proper ack_num to the client-side.
 * function ends only after it was able to send the ACK successfully (otherwise it'll keep trying).
 * @param ack_num - packet num we acknowledge
 * @param socketfd, client, client_addr_length - are network parameters
 */
void ACK_response(int socketfd, int ack_num, struct sockaddr_in *client, socklen_t client_addr_length) {
    ack_struct ack = {htons(4), htons(ack_num)}; // create an ack for sending
    int bytes_sent = 0;
    do { // keep trying to send the ack, until it succeeds
        bytes_sent = sendto(socketfd, &ack, sizeof(ack), 0,
                            (struct sockaddr_in *) client_addr, client_addr_length);
        if (bytes_sent < 0) // sending of ack has falied
            perror("TTFTP_ERROR: ");
    } while (bytes_sent != sizeof(ack));
    // success!! you are a lucky person
    printf("OUT:ACK, %d\n", ack_num);
    fflush(stdout);


}
