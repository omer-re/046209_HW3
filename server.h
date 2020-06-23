#ifndef _SERVER_H
#define _SERVER_H


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

void recieveData(int socketfd, struct sockaddr_in* client_addr, socklen_t client_addr_length, FILE* fptr);


void WRQ_parser(char* buffer, Wrq_struct* Wrq);

void ACK_response(int socketfd, int ack_num, struct sockaddr_in* client, socklen_t client_addr_length);

#endif
