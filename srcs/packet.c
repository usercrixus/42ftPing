#include "packet.h"

static uint16_t checksum(void *data, int len)
{
    uint16_t *buf = data;
    uint32_t sum = 0;
    for (int i = len; i > 1; i -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int build_packet(char *packet)
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