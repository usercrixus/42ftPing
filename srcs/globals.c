#include "globals.h"

int sockfd = -1;
int transmitted = 0;
int received = 0;
double rtt_sum = 0;
double rtt_sum2 = 0;
double rtt_min = 0;
double rtt_max = 0;
uint16_t seq = 1;
struct timeval start_time;
struct timespec send_ts;