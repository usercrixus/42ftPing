#include "signal.h"

void handle_sigint(int signo)
{
	(void)signo;
	struct timeval now;
	gettimeofday(&now, NULL);
	double elapsed = (now.tv_sec - start_time.tv_sec) * 1000.0 + (now.tv_usec - start_time.tv_usec) / 1000.0;
	printf("\n--- ping statistics ---\n");
	printf("%d packets transmitted, %d received, %.0f%% packet loss, time %.0fms\n",
		   transmitted,
		   received,
		   transmitted ? ((transmitted - received) * 100.0 / transmitted) : 0.0,
		   elapsed);
	if (received > 0)
	{
		double avg = rtt_sum / received;
		double mdev = sqrt(rtt_sum2 / received - avg * avg);
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", rtt_min, avg, rtt_max, mdev);
	}
	if (sockfd >= 0)
		close(sockfd);
	exit(0);
}