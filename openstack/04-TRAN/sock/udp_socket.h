#ifndef __UDPSOCKET_H
#define __UDPSOCKET_H

#include "udp.h"
#include "opendefs.h"
//=========================== define ==========================================

//=========================== typedef =========================================

BEGIN_PACK
struct sockaddr
{
	uint16_t     sa_family;
	char         sa_data[26];
};
END_PACK

//=========================== variables =======================================

typedef struct {

} udp_socket_vars_t;

//=========================== prototypes ======================================

int16_t udp_socket_init(void);

int16_t udp_socket_sendto(int16_t socket, const void *buffer, size_t length, int16_t flags, const struct sockaddr *address, int16_t address_len);

#endif