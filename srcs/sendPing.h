#pragma once

#include <netinet/in.h>

int sendPing(struct sockaddr_in *dest);