#pragma once

#include <stdint.h>    // for uint16_t
#include <sys/time.h>  // for timeval

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