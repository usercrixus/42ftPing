#pragma once

#include <signal.h>
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef struct
{
    struct timespec send_ts;
    int seq;
} send_record_t;

extern volatile sig_atomic_t running;
extern int transmitted;
extern int received;
extern double rtt_sum;
extern double rtt_sum2;
extern double rtt_min;
extern double rtt_max;
extern struct timeval start_time;
extern send_record_t last_send;