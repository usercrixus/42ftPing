#include "ping.h"
#include "globals.h"
#include <arpa/inet.h>        // for inet_ntop, inet_pton
#include <netdb.h>            // for addrinfo, freeaddrinfo, getaddrinfo
#include <netinet/in.h>       // for sockaddr_in, IPPROTO_ICMP, in_addr
#include <netinet/ip.h>       // for iphdr
#include <netinet/ip_icmp.h>  // for icmphdr
#include <signal.h>           // for signal, SIGINT
#include <stdio.h>            // for printf, NULL, fprintf, stderr
#include <stdlib.h>           // for exit, EXIT_SUCCESS
#include <string.h>           // for strncpy, memset
#include <sys/socket.h>       // for AF_INET, SOCK_RAW
#include <sys/time.h>         // for gettimeofday
#include <unistd.h>           // for sleep
#include "globals.h"          // for initStat
#include "params.h"           // for t_sockinfo, PACKET_SIZE
#include "receivePing.h"      // for receivePing
#include "sendPing.h"         // for sendPing
#include "signal.h"           // for handle_sigint
#include "socket.h"           // for setSocket

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
    t_sockinfo sockinfo;
    getDestination(dest_ip_or_host, &sockinfo);
    if (verbose)
    {
        printf("ping: sockfd4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_INET\n\n", stat.sockfd);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'", sockinfo.hostname);
    }
    printHeader(dest_ip_or_host, &sockinfo);
    while (1)
    {
        if (sendPing(&(sockinfo.sockaddr_in)) > 0)
            receivePing(&sockinfo, verbose);
        sleep(1);
    }
    return EXIT_SUCCESS;
}
