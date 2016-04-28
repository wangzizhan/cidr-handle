#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "netmask.h"

void transfer(uint32_t ip_start, uint32_t ip_end) 
{
	struct in_addr addr;

	uint32_t net = ip_start;
	uint32_t mask;
	int i;

	while ( net <= ip_end ) {
		for(i = 32; i >= 0; i--) {
			mask = (i == 0) ? 0 : 0xffffffff << (32 - i);

			if ((net & mask) != net || (net | ~mask) > ip_end) {
				addr.s_addr = htonl(net);
				printf(" - %s/%d\n", inet_ntoa(addr), i + 1);
				mask = 0xffffffff << (32 - i - 1);
				net = (net | ~mask) + 1;
				break;
			}
		}

		if( i == -1 ) {
			printf("0.0.0.0/0\n");
			break;
		}

		if( net == 0 ) {
			break;
		}
	}
}
