#include "globals.h"

/* now _define_ them exactly once: */
volatile sig_atomic_t running = 1;
int transmitted = 0;
int received = 0;
double rtt_sum = 0;
double rtt_sum2 = 0;
double rtt_min = 0;
double rtt_max = 0;
struct timeval start_time;
send_record_t last_send;
