#include "globals.h"

t_stat stat;

void initStat()
{
	stat.sockfd = -1;
	stat.transmitted = 0;
	stat.received = 0;
	stat.rtt_sum = 0;
	stat.rtt_squared_sum = 0;
	stat.rtt_min = 0;
	stat.rtt_max = 0;
	stat.seq = 1;
}