#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <math.h>

#include "globals.h"
#include "signal.h"
#include "packet.h"
#include "params.h"
#include "socket.h"

typedef struct s_sockinfo
{
	struct sockaddr_in sockaddr_in;
	char ipstr[INET_ADDRSTRLEN];
	char hostname[NI_MAXHOST];
} t_sockinfo;

typedef struct s_echoResponse
{
	char buffer[1024];
	int ip_hdr_len;
	ssize_t byteReceived;
	struct sockaddr_in src;
	int isValid;
} t_pingResponse;

/**
 * ping entry point
 */
int ping(const char *dest_ip_or_host, int verbose);