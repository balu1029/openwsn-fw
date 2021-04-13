#include "udp_socket.h"

//=========================== define ==========================================

#define SOCKET_ISACTIVE(i)      (sock_info[(i)].socket_state & 0x1)
#define SOCKET_ACTIVATE(i)      (sock_info[(i)].socket_state |= 0x1)
#define SOCKET_DEACTIVATE(i)    (sock_info[(i)].socket_state = 0x0)

#define UDP_FREE                0x0
#define UDP_REC                 0x7F
#define UDP_BSY                 0x57

//=========================== typedef =========================================

//=========================== variables =======================================

udp_socket_vars_t udp_socket_vars;

//=========================== prototypes ======================================

int16_t udp_socket_init(void) {
    return 0;
}

int16_t udp_socket_sendto(int16_t socket, const void *buffer, size_t length, int16_t flags,
               const struct sockaddr *address, int16_t address_len)
{
   OpenQueueEntry_t*    pkt;
   const struct sockaddr_in6 *ip6_addr = (const struct sockaddr_in6 *)address;

   pkt = openqueue_getFreePacketBuffer(COMPONENT_SOCKET);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_SOCKET,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return -1;
   }
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   pkt->payload = &pkt->packet[127];
   pkt->payload -= length;
   memcpy(pkt->payload, buffer, length);
   pkt->creator = COMPONENT_SOCKET;
   pkt->owner   = COMPONENT_UDP;
   pkt->length = length;
   pkt->l3_destinationAdd.type = ADDR_128B;

   ENABLE_INTERRUPTS();
   if ((openudp_send(pkt))== E_FAIL) {
	   openserial_printError(
	            COMPONENT_SOCKET,
	            ERR_NO_SENT_PACKET,
	            (errorparameter_t)0,
	            (errorparameter_t)0
	         );
      openqueue_freePacketBuffer(pkt);
      return -1;
   }

  return length;
}
