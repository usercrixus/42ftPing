#pragma once

#include <stdint.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

#include "params.h"
#include "globals.h"

/**
 * build the ping packet
 */
int build_packet(char *packet);
