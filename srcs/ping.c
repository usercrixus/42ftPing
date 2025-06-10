#include "ping.h"

static int sendPing(struct sockaddr_in *dest)
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

static void receivePing(t_sockinfo *sockinfo, int verbose)
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
                printf("%zd bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n", pingResponse.byteReceived - pingResponse.ip_hdr_len, sockinfo->ipstr, stat.seq - 1, ((struct iphdr *)pingResponse.buffer)->ttl, ms);
            else
                printf("%zd bytes from %s (%s): icmp_seq=%u ttl=%d time=%.3f ms\n", pingResponse.byteReceived - pingResponse.ip_hdr_len, sockinfo->hostname, sockinfo->ipstr, stat.seq - 1, ((struct iphdr *)pingResponse.buffer)->ttl, ms);
        }
        else if (verbose) // unexpected reply...
        {
            char src_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &pingResponse.src.sin_addr, src_ip, sizeof(src_ip));
            printf("From %s: ICMP type=%d code=%d\n", src_ip, reply->type, reply->code);
        }
    }
}

static void getDestination(const char *dest_ip_or_host, t_sockinfo *sockinfo)
{
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPV4 only
    hints.ai_socktype = SOCK_RAW;     // RAW SOCKET
    hints.ai_protocol = IPPROTO_ICMP; // ICMP protocol

    if (getaddrinfo(dest_ip_or_host, NULL, &hints, &res) != 0) // Turn dest into IPv4 address.
    {
        fprintf(stderr, "ping: %s: Name or service not known\n", dest_ip_or_host);
        exit(1);
    }

    struct sockaddr_in *ipV4 = (struct sockaddr_in *)res->ai_addr;
    sockinfo->sockaddr_in = *ipV4;                                                 // save it for later.
    inet_ntop(AF_INET, &ipV4->sin_addr, sockinfo->ipstr, sizeof(sockinfo->ipstr)); // Convert the 32-bit address to dotted-decimal text, e.g. "163.70.128.35", and save in sockinfo->ipstr.

    struct in_addr tmp;
    if (inet_pton(AF_INET, dest_ip_or_host, &tmp) == 1)                           // try to parse dest as an IPv4 literal.
        strncpy(sockinfo->hostname, sockinfo->ipstr, sizeof(sockinfo->hostname)); // If it succeeds, the user literally typed an IP
    else
    {
        if (getnameinfo((struct sockaddr *)ipV4, sizeof(*ipV4), sockinfo->hostname, sizeof(sockinfo->hostname), NULL, 0, NI_NAMEREQD) != 0) // set hostname depending on the ipV4 domain name
            strncpy(sockinfo->hostname, sockinfo->ipstr, sizeof(sockinfo->hostname));                                                       // on error, we still set hostname on IP instead of domain name
    }

    freeaddrinfo(res);
}

static void printHeader(const char *dest_ip_or_host, t_sockinfo *sockinfo)
{
    int icmp_hdr = sizeof(struct icmphdr); // icmp header usually 8 bytes
    int ip_hdr = sizeof(struct iphdr);     // ip header usually 20 bytes
    int payload = PACKET_SIZE - icmp_hdr;  // payload total 56
    int on_wire = PACKET_SIZE + ip_hdr;    // packet total 84
    printf("PING %s (%s) %d(%d) bytes of data\n", dest_ip_or_host, sockinfo->ipstr, payload, on_wire);
}

int ping(const char *dest_ip_or_host, int verbose)
{
    initStat();
    signal(SIGINT, handle_sigint);
    gettimeofday(&stat.start_time, NULL);
    setSocket();
    if (verbose)
        printf("ping: sockfd4.fd: %d (socktype: SOCK_RAW)\n\n", stat.sockfd);
    t_sockinfo sockinfo;
    getDestination(dest_ip_or_host, &sockinfo);
    printHeader(dest_ip_or_host, &sockinfo);
    while (1)
    {
        if (sendPing(&(sockinfo.sockaddr_in)) > 0)
            receivePing(&sockinfo, verbose);
        sleep(1);
    }
    return EXIT_SUCCESS;
}
