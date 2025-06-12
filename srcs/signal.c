#include "signal.h"
#include "globals.h"
#include <math.h>      // for sqrt
#include <stdio.h>     // for printf
#include <stdlib.h>    // for exit
#include <sys/time.h>  // for timeval, gettimeofday
#include <unistd.h>    // for close, NULL

void handle_sigint(int signo)
{
	(void)signo;
	struct timeval now;
	gettimeofday(&now, NULL);
	double elapsed = (now.tv_sec - stat.start_time.tv_sec) * 1000.0 + (now.tv_usec - stat.start_time.tv_usec) / 1000.0;
	printf("\n--- ping statistics ---\n");
	printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.0fms\n",
		   stat.transmitted,
		   stat.received,
		   stat.transmitted ? ((stat.transmitted - stat.received) * 100.0 / stat.transmitted) : 0.0,
		   elapsed);
	if (stat.received > 0)
	{
		double avg = stat.rtt_sum / stat.received;
		double mdev = sqrt(stat.rtt_squared_sum / stat.received - avg * avg);
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", stat.rtt_min, avg, stat.rtt_max, mdev);
	}
	if (stat.sockfd >= 0)
		close(stat.sockfd);
	exit(0);
}