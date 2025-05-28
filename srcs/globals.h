#pragma once

#include <signal.h>
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

extern struct timespec send_ts;
extern uint16_t seq;
extern int transmitted;
extern int received;
extern double rtt_sum;
extern double rtt_sum2;
extern double rtt_min;
extern double rtt_max;
extern struct timeval start_time;
extern int sockfd;