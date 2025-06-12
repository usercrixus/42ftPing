#include "sendPing.h"
#include <arpa/inet.h>       // for htons
#include <netinet/in.h>      // for sockaddr_in
#include <netinet/ip_icmp.h> // for icmphdr, icmphdr::(anonymous union)::(a...
#include <stdint.h>          // for uint16_t, uint32_t, uint8_t
#include <stdio.h>           // for perror
#include <string.h>          // for memset
#include <sys/socket.h>      // for sendto
#include <sys/time.h>        // for gettimeofday
#include <unistd.h>          // for getpid
#include "params.h"          // for PACKET_SIZE
#include "globals.h"

static uint16_t checksum(void *data, int len)
{
    uint16_t *buf = data;
    uint32_t sum = 0;
    for (int i = len; i > 1; i -= 2) // sum the whole packet
        sum += *buf++;
    if (len == 1) // if packet size is odd
        sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF); // add 16 lower bits with 16 higher bits
    sum += (sum >> 16); // if there is a carry bit, add it, as we have mawimum of information
    return ~sum; // ~x + x = 0xFFFF if x is on 16 bits, so easy to verify the checksum.
}

static int build_packet(char *packet)
{
    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(stat.seq);
    icmp->checksum = 0;
    memset(packet + sizeof(*icmp), 0xA5, PACKET_SIZE - sizeof(*icmp));
    icmp->checksum = checksum(packet, PACKET_SIZE);
    return PACKET_SIZE;
}

int sendPing(struct sockaddr_in *dest)
{
    char packet[PACKET_SIZE];
    int len = build_packet(packet);
    gettimeofday(&stat.send_time, NULL);
    if (sendto(stat.sockfd, packet, len, 0, (struct sockaddr *)dest, sizeof(*dest)) <= 0)
        return (perror("sendto"), -1);
    stat.transmitted++;
    stat.seq++;
    return len;
}