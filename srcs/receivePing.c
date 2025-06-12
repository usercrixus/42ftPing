#include "receivePing.h"
#include <arpa/inet.h>        // for ntohs, htons, inet_ntop
#include <errno.h>            // for errno, EAGAIN, EWOULDBLOCK
#include <netinet/in.h>       // for sockaddr_in, INET_ADDRSTRLEN
#include <netinet/ip.h>       // for iphdr
#include <netinet/ip_icmp.h>  // for icmphdr, icmphdr::(anonymous union)::(a...
#include <stdio.h>            // for printf, fprintf, perror, NULL, size_t
#include <string.h>           // for strcmp
#include <sys/socket.h>       // for recvfrom, AF_INET, socklen_t
#include <sys/time.h>         // for timeval, gettimeofday
#include <unistd.h>           // for getpid
#include "globals.h"

/*
 * cumpute the rtt
 * RTT stands for Round-Trip Time — the elapsed time between when you send an ICMP “echo request”
 * packet and when you receive the corresponding “echo reply.”
 */
static void computeRTT(double ms)
{
	if (stat.received == 1)
		stat.rtt_min = stat.rtt_max = ms;
	else
	{
		if (ms < stat.rtt_min)
			stat.rtt_min = ms;
		if (ms > stat.rtt_max)
			stat.rtt_max = ms;
	}
	stat.rtt_sum += ms;
	stat.rtt_squared_sum += ms * ms;
}

static void getPingResponse(int verbose, t_pingResponse *pingResponse)
{
	socklen_t addrlen = sizeof(pingResponse->src);
	pingResponse->byteReceived = recvfrom(stat.sockfd, pingResponse->buffer, sizeof(pingResponse->buffer), 0, (struct sockaddr *)&(pingResponse->src), &addrlen);
	pingResponse->isValid = 1;

	if (pingResponse->byteReceived < 0)
	{
		if ((errno == EAGAIN || errno == EWOULDBLOCK) && verbose)
			printf("Request timed out.\n");
		else if (verbose)
			perror("recvfrom");
		pingResponse->isValid = 0;
	}
	else
	{
		pingResponse->ip_hdr_len = (pingResponse->buffer[0] & 0x0F) * 4;
		if ((size_t)(pingResponse->byteReceived) < pingResponse->ip_hdr_len + sizeof(struct icmphdr))
		{
			if (verbose)
				fprintf(stderr, "ft_ping: packet too short (%zd bytes, hdr %d)\n", pingResponse->byteReceived, pingResponse->ip_hdr_len);
			pingResponse->isValid = 0;
		}
	}
}

static void printNoHostName(t_sockinfo *sockinfo, t_pingResponse *pingResponse, struct icmphdr *reply, double ms, int verbose)
{
	if (verbose)
		printf("%zd bytes from %s: icmp_seq=%u ident=%d ttl=%d time=%.3f ms\n",
			   pingResponse->byteReceived - pingResponse->ip_hdr_len,
			   sockinfo->ipstr,
			   stat.seq - 1,
			   ntohs(reply->un.echo.id),
			   ((struct iphdr *)pingResponse->buffer)->ttl, ms);
	else
		printf("%zd bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
			   pingResponse->byteReceived - pingResponse->ip_hdr_len,
			   sockinfo->ipstr,
			   stat.seq - 1,
			   ((struct iphdr *)pingResponse->buffer)->ttl, ms);
}

static void printHostName(t_sockinfo *sockinfo, t_pingResponse *pingResponse, struct icmphdr *reply, double ms, int verbose)
{
	if (verbose)
		printf("%zd bytes from %s (%s): icmp_seq=%u ident=%d ttl=%d time=%.3f ms\n",
			   pingResponse->byteReceived - pingResponse->ip_hdr_len,
			   sockinfo->hostname,
			   sockinfo->ipstr,
			   stat.seq - 1,
			   ntohs(reply->un.echo.id),
			   ((struct iphdr *)pingResponse->buffer)->ttl, ms);
	else
		printf("%zd bytes from %s (%s): icmp_seq=%u ttl=%d time=%.3f ms\n",
			   pingResponse->byteReceived - pingResponse->ip_hdr_len,
			   sockinfo->hostname,
			   sockinfo->ipstr,
			   stat.seq - 1,
			   ((struct iphdr *)pingResponse->buffer)->ttl, ms);
}

void receivePing(t_sockinfo *sockinfo, int verbose)
{
	t_pingResponse pingResponse;
	getPingResponse(verbose, &pingResponse);

	if (pingResponse.isValid)
	{
		struct icmphdr *reply = (struct icmphdr *)(pingResponse.buffer + pingResponse.ip_hdr_len); // we skip the ip header, so we get the icm response
		if (reply->type == ICMP_ECHOREPLY && reply->un.echo.id == htons(getpid() & 0xFFFF) && ntohs(reply->un.echo.sequence) == stat.seq - 1)
		{
			stat.received++; // we just received a new response
			struct timeval recv_time;
			gettimeofday(&recv_time, NULL);
			double ms = (recv_time.tv_sec - stat.send_time.tv_sec) * 1000.0 + (recv_time.tv_usec - stat.send_time.tv_usec) / 1000.0; // compute the time to receive this ping (rec - send)
			computeRTT(ms);
			if (strcmp(sockinfo->hostname, sockinfo->ipstr) == 0) // if we ping if an ip, this should be true, if we ping with a domain name, we should fall in the else condition
				printNoHostName(sockinfo, &pingResponse, reply, ms, verbose);
			else
				printHostName(sockinfo, &pingResponse, reply, ms, verbose);
		}
		else if (verbose) // unexpected reply...
		{
			char src_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &pingResponse.src.sin_addr, src_ip, sizeof(src_ip));
			printf("From %s: ICMP type=%d code=%d\n", src_ip, reply->type, reply->code);
		}
	}
}