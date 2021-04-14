#include "config.h"

#if OPENWSN_UECHO_DUP_C

#include "opendefs.h"
#include "uecho_dup.h"
#include "sock.h"
#include "async.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "udp_socket.h"

//=========================== variables =======================================

sock_udp_t uecho_dup_sock;
int16_t socket;
struct sockaddr_in6* address;

//=========================== prototypes ======================================

void uecho_dup_handler(OpenQueueEntry_t* msg);

//=========================== public =========================================

void uecho_dup_init(void) {
    // clear local variables
    socket = udp_socket_socket(AF_INET6, SOCK_DGRAM, ADDR_128B);
    memset(&uecho_dup_sock, 0, sizeof(sock_udp_t));

    sock_udp_ep_t local;
    address->sin6_family = AF_INET6;
    address->sin6_port = WKP_UDP_ECHO_DUP;

    // if (sock_udp_create(&uecho_dup_sock, &local, NULL, 0) < 0) {
    //     openserial_printf("Could not create socket\n");
    //     return;
    // }
    if (udp_socket_bind(socket, (const struct sockaddr*)address, 0, &uecho_dup_handler) < 0) {
         openserial_printf("Could not create socket\n");
         return;
    }

    openserial_printf("Created a UDP socket\n");
}

void uecho_dup_handler(OpenQueueEntry_t* msg) {

    char buf[50];

    sock_udp_ep_t remote;
    int16_t res;

    if ((res = udp_socket_recvfrom(socket, buf, sizeof(buf), 0, (const struct sockaddr*)address, sizeof(address))) >= 0) {
        openserial_printf("Received %d bytes from remote endpoint:\n", res);
        openserial_printf(" - port: %d", address->sin6_port);
        openserial_printf(" - addr: ");
        //for(int i=0; i < 16; i ++)
            //openserial_printf("%x ", (uint8_t)(address->sin6_addr.u)[i]);

        openserial_printf("\n\n");
        openserial_printf("Msg received: %s\n\n", buf);

        if (udp_socket_sendto(socket, buf, res, 0, (const struct sockaddr*)address, sizeof(address)) < 0) {
            openserial_printf("Error sending reply\n");
        }
    }
}
//=========================== private =========================================

#endif /* OPENWSN_UECHO_DUP_C */
