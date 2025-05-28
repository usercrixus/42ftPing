#include "ping.h"

#define PACKET_SIZE 64

/**
 * checksum — Compute the 16-bit Internet checksum (RFC 1071) over a data block.
 *
 * This routine implements the standard “ones-complement” checksum used by
 * ICMP, IP, TCP, UDP, etc.  It:
 *   1) Adds up all 16-bit words in the buffer.
 *   2) If there’s an odd byte at the end, it pads it to 16 bits and adds it.
 *   3) Folds any carry bits from the upper 16 bits back into the lower 16 bits
 *      (repeat until no more carries).
 *   4) Returns the ones-complement of the final sum.
 *
 * The caller should set the checksum field in the packet to 0 before calling.
 */
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
/**
 * build the ping packet
 */
static int build_packet(char *packet)
{
    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(seq);
    icmp->checksum = 0;
    memset(packet + sizeof(*icmp), 0xA5, PACKET_SIZE - sizeof(*icmp));
    icmp->checksum = checksum(packet, PACKET_SIZE);
    return PACKET_SIZE;
}

/**
 * send a ping to the target
 */
static int send_echo(struct sockaddr_in *dest)
{
    char packet[PACKET_SIZE];
    int len = build_packet(packet);
    clock_gettime(CLOCK_MONOTONIC, &send_ts);
    if (sendto(sockfd, packet, len, 0, (struct sockaddr *)dest, sizeof(*dest)) <= 0)
        return (perror("sendto"), -1);
    transmitted++;
    seq++;
    return len;
}

/*
 * cumpute the rtt
 * RTT stands for Round-Trip Time—the elapsed time between when you send an ICMP “echo request”
 * packet and when you receive the corresponding “echo reply.”
 */
void computeRTT(double ms)
{
    if (received == 1)
        rtt_min = rtt_max = ms;
    else
    {
        if (ms < rtt_min)
            rtt_min = ms;
        if (ms > rtt_max)
            rtt_max = ms;
    }
    rtt_sum += ms;
    rtt_sum2 += ms * ms;
}

/**
 * Wait for the echo response with recvfrom
 * return the responde
 */
static t_echoResponse getEchoResponse(int verbose)
{
    t_echoResponse echoResponse;
    socklen_t addrlen = sizeof(echoResponse.src);
    echoResponse.byteReceived = recvfrom(sockfd, echoResponse.buffer, sizeof(echoResponse.buffer), 0, (struct sockaddr *)&(echoResponse.src), &addrlen);
    echoResponse.isValid = 1;

    if (echoResponse.byteReceived < 0)
    {
        if ((errno == EAGAIN || errno == EWOULDBLOCK) && verbose)
            printf("Request timed out.\n");
        else if (verbose)
            perror("recvfrom");
        echoResponse.isValid = 0;
    }
    else
    {
        echoResponse.ip_hdr_len = (echoResponse.buffer[0] & 0x0F) * 4;
        if ((size_t)(echoResponse.byteReceived) < echoResponse.ip_hdr_len + sizeof(struct icmphdr))
        {
            if (verbose)
                fprintf(stderr, "ft_ping: packet too short (%zd bytes, hdr %d)\n", echoResponse.byteReceived, echoResponse.ip_hdr_len);
            echoResponse.isValid = 0;
        }
    }
    return (echoResponse);
}

/**
 * handle the echo response
 */
static void receiveEcho(t_sockinfo *sockinfo, int verbose)
{
    t_echoResponse echoResponse = getEchoResponse(verbose);

    if (echoResponse.isValid)
    {
        struct icmphdr *reply = (struct icmphdr *)(echoResponse.buffer + echoResponse.ip_hdr_len);
        if (reply->type == ICMP_ECHOREPLY && reply->un.echo.id == htons(getpid() & 0xFFFF) && ntohs(reply->un.echo.sequence) == seq - 1)
        {
            received++;
            struct timespec recv_ts;
            clock_gettime(CLOCK_MONOTONIC, &recv_ts);
            double ms = (recv_ts.tv_sec - send_ts.tv_sec) * 1000.0 + (recv_ts.tv_nsec - send_ts.tv_nsec) / 1e6;
            computeRTT(ms);
            printf("%zd bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n", echoResponse.byteReceived - echoResponse.ip_hdr_len, sockinfo->ipstr, seq - 1, ((struct iphdr *)echoResponse.buffer)->ttl, ms);
        }
        else if (verbose)
        {
            char src_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &echoResponse.src.sin_addr, src_ip, sizeof(src_ip));
            printf("From %s: ICMP type=%d code=%d\n", src_ip, reply->type, reply->code);
        }        
    }
}
/**
 * return a struct sockaddr_in representing the target to ping
 * and a string representing the ip of the target to ping
 */
static t_sockinfo getDestination(const char *dest_ip_or_host)
{
    t_sockinfo sockinfo;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    struct addrinfo *res;
    if (getaddrinfo(dest_ip_or_host, NULL, &hints, &res) != 0)
    {
        fprintf(stderr, "ping: %s: Name or service not known\n", dest_ip_or_host);
        exit(1);
    }
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &ipv4->sin_addr, sockinfo.ipstr, sizeof(sockinfo.ipstr));
    sockinfo.sockaddr_in = *ipv4;
    freeaddrinfo(res);
    return (sockinfo);
}

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

int ping(const char *dest_ip_or_host, int verbose)
{
    signal(SIGINT, handle_sigint);
    gettimeofday(&start_time, NULL);
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return (perror("socket"), EXIT_FAILURE);
    setSocketTimeOut(sockfd);
    if (verbose)
        printf("ping: sockfd: %d (socktype: AF_INET RAW)\n\n", sockfd);
    t_sockinfo sockinfo = getDestination(dest_ip_or_host);
    printf("PING %s (%s) %d data bytes\n", dest_ip_or_host, sockinfo.ipstr, PACKET_SIZE);
    while (1)
    {
        if (send_echo(&(sockinfo.sockaddr_in)) > 0)
            receiveEcho(&sockinfo, verbose);
        sleep(1);
    }
    return EXIT_SUCCESS;
}
