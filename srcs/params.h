#pragma once

#include <netdb.h>       // for NI_MAXHOST
#include <netinet/in.h>  // for sockaddr_in, INET_ADDRSTRLEN
#include <sys/types.h>   // for ssize_t

#define PACKET_SIZE 64

typedef struct s_sockinfo
{
	struct sockaddr_in sockaddr_in;
	char ipstr[INET_ADDRSTRLEN];
	char hostname[NI_MAXHOST];
	char canonname[NI_MAXHOST];
} t_sockinfo;

typedef struct s_echoResponse
{
	char buffer[1024];
	int ip_hdr_len;
	ssize_t byteReceived;
	struct sockaddr_in src;
	int isValid;
} t_pingResponse;