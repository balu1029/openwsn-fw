#include "udp_socket.h"

//=========================== define ==========================================

#define SOCKET_ISACTIVE(i)      (sock_info[(i)].socket_state & 0x1)
#define SOCKET_ACTIVATE(i)      (sock_info[(i)].socket_state |= 0x1)
#define SOCKET_DEACTIVATE(i)    (sock_info[(i)].socket_state = 0x0)

#define UDP_FREE                0x0
#define UDP_REC                 0x7F
#define UDP_BSY                 0x57

udp_socket_vars_t udp_socket_vars;

//=========================== typedef =========================================

//=========================== variables =======================================

static EH_UDP_SOCKET_T sock_info[NUM_SOCKETS];
static uint32_t        sockcount = 0;


#define SOCKET_ISACTIVE(i)      (sock_info[(i)].socket_state & 0x1)
#define SOCKET_ACTIVATE(i)      (sock_info[(i)].socket_state |= 0x1)
#define SOCKET_DEACTIVATE(i)    (sock_info[(i)].socket_state = 0x0)

#define UDP_FREE                0x0
#define UDP_REC                 0x7F
#define UDP_BSY                 0x57

//=========================== prototypes ======================================

uint8_t         null_addr[IP_ADDR_LEN] = {0u};

void udp_socket_init() {}
static int16_t find_first_free_socket(void);
static int16_t find_udp_socket(uint16_t local_port, uint8_t *local_addr);

//=========================== public ==========================================


/*****************************************************************************
 * @brief   Create an endpoint for communication.
******************************************************************************/

int16_t udp_socket_socket(int16_t domain, int16_t type, int16_t protocol)
{
    int16_t socket_no;

    socket_no = find_first_free_socket();
    if (socket_no != NOT_ENOUGH_SOCKETS)
    {
        SOCKET_ACTIVATE(socket_no);
        sock_info[socket_no].domain = domain;
        sock_info[socket_no].sock_type = type;
        sock_info[socket_no].protocol = protocol;
        sock_info[socket_no].O_NONBLOCK = TRUE;
        sock_info[socket_no].recv_udp_flag = UDP_FREE;
        sock_info[socket_no].dataptr = &sock_info[socket_no].sock_buf[0];
        sock_info[socket_no].hostaddr.sin6_family = AF_INET6;
        // make sure UDP service is cannot be used yet, wait until socket is bound
        sock_info[socket_no].udp_rsc_desc.port = 0u;
        sock_info[socket_no].udp_rsc_desc.callbackReceive = NULL;
        sock_info[socket_no].udp_rsc_desc.callbackSendDone = NULL;
    }
    else
    {
    	socket_no = INVALID_SOCKET;
    }
    return socket_no;
}



/*****************************************************************************
 * @brief   Bind a name to a socket.
*****************************************************************************/
int16_t udp_socket_bind(int16_t socket, const struct sockaddr *address, uint8_t length)
{
    int16_t retval;
    if (socket < 0)
    {
      retval = -1;
    }
    else
    {
    	// we only support IPv6 for the moment
    	if (address->sa_family == AF_INET6)
    	{
    		const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)address;
    		sock_info[socket].local_port = addr_in6->sin6_port;
    		sock_info[socket].sockaddr.sin6_family = addr_in6->sin6_family;
    		sock_info[socket].sockaddr.sin6_port = addr_in6->sin6_port;
    		memcpy((void*)&sock_info[socket].sockaddr.sin6_addr, (void*)&addr_in6->sin6_addr, IP_ADDR_LEN);
            sock_info[socket].udp_rsc_desc.port = UIP_HTONS(addr_in6->sin6_port); // socket structure is in network order
            sock_info[socket].udp_rsc_desc.callbackReceive = &udp_socket_deliver_msg_to_sockets;
            sock_info[socket].udp_rsc_desc.callbackSendDone = NULL; // use default send_done handler
            openudp_register(&sock_info[socket].udp_rsc_desc);
    		retval = 0;
    	}
    	else
    	{
    		retval = -1;

    		openserial_printError(COMPONENT_SOCKET,ERR_UNSUPPORTED_PORT_NUMBER,
    		    	                               (errorparameter_t)socket,
    		    	                               (errorparameter_t)1);
    	}
    }
    return retval;
}


/*****************************************************************************
 * @brief   Receive a message from a socket.
*****************************************************************************/

int16_t udp_socket_recvfrom( int16_t socket, void *buffer, size_t buf_len, int16_t flags, struct sockaddr *addr, int16_t *addr_len)
{
    int16_t length = -1;
    struct sockaddr_in6 *ip6_addr = (struct sockaddr_in6 *)addr;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    if (((buffer != NULL) && (socket >= 0)) && ((ip6_addr != NULL) && (addr_len != NULL)))
    {
        // Check packet is received
        if (sock_info[socket].recv_udp_flag == UDP_REC)
        {
            sock_info[socket].recv_udp_flag = UDP_BSY;
            sockcount++;
            length = sock_info[socket].datalen;
            if ((length <= buf_len) && (length > 0))
            {
                memcpy(buffer, sock_info[socket].dataptr, length);
                ip6_addr->sin6_family = AF_INET6;
                ip6_addr->sin6_port = sock_info[socket].hostaddr.sin6_port;
                memcpy((void*)&ip6_addr->sin6_addr, (void*)&sock_info[socket].hostaddr.sin6_addr, IP_ADDR_LEN);
                memset(&sock_info[socket].hostaddr, 0, sizeof(struct sockaddr_in6));
                *addr_len =  sizeof(struct sockaddr_in6);
            }
            else
            {
                length = -1;            // error                                    // data too long for buffer size
                sock_info[socket].recv_udp_flag = UDP_FREE;

                openserial_printError(COMPONENT_SOCKET,ERR_UNSUPPORTED_PORT_NUMBER,
                    		    	                               (errorparameter_t)socket,
                    		    	                               (errorparameter_t)3);
            }
        }
        else
        {
           length = 0;          // no message received
        }
    }
    else
    {
       length = -1;            // error

       openserial_printError(COMPONENT_SOCKET,ERR_UNSUPPORTED_PORT_NUMBER,
           		    	                               (errorparameter_t)socket,
           		    	                               (errorparameter_t)2);
    }
    ENABLE_INTERRUPTS();

    return length;

}




/******************************************************************************
 * @brief   Send a message on a socket.
******************************************************************************/
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
   pkt->l4_destination_port = UIP_HTONS(ip6_addr->sin6_port);
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0], (void*)&ip6_addr->sin6_addr, IP_ADDR_LEN);
   pkt->l4_sourcePortORicmpv6Type = UIP_HTONS(sock_info[socket].local_port);
   sock_info[socket].recv_udp_flag = UDP_FREE;

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



void udp_socket_deliver_msg_to_sockets(OpenQueueEntry_t* msg)
{
    int16_t receiving_socket = INVALID_SOCKET;

    receiving_socket = find_udp_socket(UIP_HTONS(msg->l4_destination_port), null_addr);
    if (((receiving_socket >= 0 ) && (receiving_socket <= NUM_SOCKETS)) && (SOCKET_ISACTIVE(receiving_socket)))
    {
        INTERRUPT_DECLARATION();
        DISABLE_INTERRUPTS();
        if (((msg->length > 0) && (msg->length <= MAX_PAYLOAD_SIZE )) && ( sock_info[receiving_socket].dataptr != NULL))
        {
            memcpy((void *)&sock_info[receiving_socket].hostaddr.sin6_addr, (void*)&msg->l3_sourceAdd.addr_128b[0], IP_ADDR_LEN);
            sock_info[receiving_socket].hostaddr.sin6_port =  UIP_HTONS(msg->l4_sourcePortORicmpv6Type);
            memcpy(sock_info[receiving_socket].dataptr, msg->payload, MAX_PAYLOAD_SIZE);
            sock_info[receiving_socket].datalen  = msg->length;
            sock_info[receiving_socket].recv_udp_flag = UDP_REC;
        }
        ENABLE_INTERRUPTS();
    }
    else
    {
        receiving_socket = INVALID_SOCKET;
    }

    openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================


/* searches for an available socket, returns socket number or negative #
   if none found.
*/
static int16_t find_first_free_socket(void)
{
    uint8_t     i;
    int16_t     socket_no = NOT_ENOUGH_SOCKETS;

    for (i=0; i < NUM_SOCKETS; i++)
    {
       if (!SOCKET_ISACTIVE(i))
       {
          socket_no = i;
          break;
       }
    }
    return socket_no;
}

/**
 * Find a local UDP socket given the local port and local address.
 *
 * NOTE: for the moment, we do not support multiple interfaces, so the
 *       address will not be used while searching. Moreover, it is
 *       common to bind sockets to the INADDR_ANY address ('0.0.0.0' for
 *       IPv4 or '::' for IPv6), which means "Any local address". So
 *       in theory we shall search for the local IPv6 address or null_addr.
 *
 * @param local_port Port to which the socket was bound
 * @param local_addr Address to which the socket was bound.
 * @return Socket descriptor if found, INVALID_SOCKET otherwise
 */
static int16_t find_udp_socket(uint16_t local_port, uint8_t *local_addr)
{
    int16_t sock_fd = INVALID_SOCKET;
    uint8_t i;

    for (i=0u; i < NUM_SOCKETS; i++)
    {
        if ((sock_info[i].sockaddr.sin6_port == local_port) && (sock_info[i].local_port == local_port))
        {
            sock_fd = i;
            break;
        }
    }

    return sock_fd;
}

