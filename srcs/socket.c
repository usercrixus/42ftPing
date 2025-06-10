#include "socket.h"

/**
 * If the echo response take more than 1 seconde, recv stop waiting
 */
static void setSocketTimeOut(int sockfd)
{
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		perror("setsockopt");
		close(sockfd);
		exit(1);
	}
}

void setSocket()
{
	stat.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (stat.sockfd < 0)
		return (perror("socket"), exit(1));
	setSocketTimeOut(stat.sockfd);
}