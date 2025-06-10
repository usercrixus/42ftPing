#pragma once

#include <signal.h>
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
	struct timeval send_time;
	uint16_t seq;
	int transmitted;
	int received;
	double rtt_sum;
	double rtt_squared_sum;
	double rtt_min;
	double rtt_max;
	struct timeval start_time;
	int sockfd;
} t_stat;

extern t_stat stat;

/**
 * initialize the stat global var
 */
void initStat();