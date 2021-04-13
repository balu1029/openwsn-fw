#ifndef __UDPSOCKET_H
#define __UDPSOCKET_H

#include "udp.h"
#include "opendefs.h"
//=========================== define ==========================================

#define IP_ADDR_LEN     16


/*************************Sockets***************************/
#define NUM_SOCKETS     4

/***********************************************************/


/* Supported address families. */
#define AF_INET6        10      /* IP version 6  */

/* Supported Socket types */
#define SOCK_DGRAM      (1)     /**< Datagram socket */

/* error numbers */
#define NOT_ENOUGH_SOCKETS       -128
#define SOCKET_ALREADY_EXISTS    -127
#define NOT_SUPPORTED            -126
#define PPP_OPEN_FAILED          -125
#define INVALID_SOCKET	         -1


/* Protocol families, same as address families. */
#define PF_INET6        AF_INET6


#define MAX_PAYLOAD_SIZE       128

//=========================== typedef =========================================

// Generic address structure
BEGIN_PACK
struct sockaddr
{
	uint16_t     sa_family;
	char         sa_data[26];
};
END_PACK

// IPv6 Address Structure
struct in6_addr
{
	//uint8_t ip6_addr[IP_ADDR_LEN];             // IPv6 Address
	union
	{
		uint8_t  Byte[16];
		uint16_t Word[8];
		uint32_t u32_addr[4];
	} u;
};


// Socket Address Structure
BEGIN_PACK
struct sockaddr_in6 {
    uint16_t            sin6_family;           // adress family, AF_INET6
    uint16_t            sin6_port;             // port number, Network Byte Order
    uint32_t            sin6_flowinfo;          // IPv6 flow information, 0 = any
    struct in6_addr     sin6_addr;              // IPv6 address
    uint32_t            sin6_scope_idr;         // Scope ID
};
END_PACK


// Socket storage structure
BEGIN_PACK
struct sockaddr_storage
{
	uint16_t        ss_family;
	uint16_t        s2_data1[1];
	uint32_t        s2_data2[1];
	uint32_t        s2_data3[4];
	uint32_t        s2_data4[1];
};
END_PACK


typedef owerror_t (* udp_socket_input_callback_t) (OpenQueueEntry_t* msg);

typedef void (*udp_callbackReceive_cbt)(OpenQueueEntry_t* msg);
typedef void (*udp_callbackSendDone_cbt)(OpenQueueEntry_t* msg, owerror_t error);

typedef struct udp_resource_desc_t udp_resource_desc_t;

struct udp_resource_desc_t {
   uint16_t                 port;             ///< UDP port that is associated with the resource
   udp_callbackReceive_cbt  callbackReceive;  ///< receive callback,
                                              ///< if NULL, all message received for port will be discarded
   udp_callbackSendDone_cbt callbackSendDone; ///< send completion callback,
                                              ///< if NULL, the associated message will be release without notification
   udp_resource_desc_t*     next;
};


BEGIN_PACK
typedef struct
{
    uint8_t                     socket_state;
    uint16_t                    local_port;
    int16_t                     domain;
    int16_t                     sock_type;
    int16_t                     protocol;
    uint8_t                     recv_udp_flag;        /* flag byte packet received */
    uint8_t                     datalen;
    bool                        O_NONBLOCK;
    struct sockaddr_in6         sockaddr;
    struct sockaddr_in6         hostaddr;
    void                        *dataptr;
    uint8_t                     sock_buf[MAX_PAYLOAD_SIZE];
    udp_resource_desc_t         udp_rsc_desc;
} EH_UDP_SOCKET_T;
END_PACK
typedef EH_UDP_SOCKET_T * PSOCKET_INFO;






//=========================== variables =======================================

extern uint8_t null_addr[IP_ADDR_LEN];

typedef struct {

} udp_socket_vars_t;

//=========================== prototypes ======================================

void udp_socket_init();

/**
 * @brief   Create an endpoint for communication.
 * @details Shall create an unbound socket in a communications domain, and
 *          return a file descriptor that can be used in later function calls
 *          that operate on sockets.
 *
 * @param[in] domain    Specifies the communications domain in which a socket
 *                      is to be created. Support for values other than AF_INET6
 *                      is not implemented yet
 * @param[in] type      Specifies the type of socket to be created. The allowed
 *                      value is SOCK_DGRAM, other values are not implemented yet.
 * @param[in] protocol  Specifies a particular protocol to be used with the
 *                      socket. Specifying a protocol of 0 causes socket() to
 *                      use an unspecified default protocol appropriate for
 *                      the requested socket type.
 *
 * @return  Upon successful completion, socket() shall return a non-negative
 *          integer, the socket file descriptor. Otherwise, a value of -1 shall
 *          be returned
 */

int16_t udp_socket_socket(int16_t domain, int16_t type, int16_t protocol);




/**
 * @brief   Bind a name to a socket.
 * @details The bind() function shall assign a local socket address *address*
 *          to a socket identified by descriptor socket that has no local
 *          socket address assigned. Sockets created with the socket() function
 *          are initially unnamed; they are identified only by their address
 *          family.
 *
 *
 * @param socket        Specifies the file descriptor of the socket to be bound.
 * @param address       Points to a sockaddr structure containing the address
 *                      to be bound to the socket. The length and format of the
 *                      address depend on the address family of the socket.
 * @param address_len   Specifies the length of the sockaddr_in6 structure pointed
 *                      to by the *address* argument.
 * @return  Upon successful completion, bind() shall return 0; otherwise, -1
 *          shall be returned and errno set to indicate the error.
 */
int16_t udp_socket_bind(int16_t socket, const struct sockaddr *address,
         uint8_t address_len);




/**
 * @brief   Receive a message from a socket.
 * @details The recvfrom() function shall receive a message from a
 *          connectionless-mode socket. It is normally used
 *          with connectionless-mode sockets because it permits the application
 *          to retrieve the source address of received data.
 *
 *
 * @param[in] socket        Specifies the socket file descriptor.
 * @param[out] buffer       Points to a buffer where the message should be
 *                          stored.
 * @param[in] length        Specifies the length in bytes of the buffer pointed
 *                          to by the buffer argument.
 * @param[in] flags         Specifies the type of message reception. Support
 *                          for values other than 0 is not implemented yet.
 * @param[out] address      A null pointer, or points to a sockaddr structure
 *                          in which the sending address is to be stored. The
 *                          length and format of the address depend on the
 *                          address family of the socket.
 * @param[out] address_len  Either a null pointer, if address is a null pointer,
 *                          or a pointer to a socklen_t object which on input
 *                          specifies the length of the supplied sockaddr
 *                          structure, and on output specifies the length of
 *                          the stored address.
 *
 * @return  Upon successful completion, recvfrom() shall return the length of
 *          the message in bytes. If no messages are available to be received,
 *          recvfrom() shall return 0. Otherwise, the function shall return -1
 */
int16_t udp_socket_recvfrom( int16_t socket, void *buffer, size_t buf_len, int16_t flags, \
                  struct sockaddr *addr, int16_t *addr_len);






/**
 * @brief   Send a message on a socket.
 * @details Shall send a message through a connectionless-mode socket.
 *          If the socket is a connectionless-mode socket, the message shall be
 *          sent to the address specified by @p address
 *
 * @param[in] socket        Specifies the socket file descriptor.
 * @param[in] buffer        Points to the buffer containing the message to send.
 * @param[in] length        Specifies the length of the message in bytes.
 * @param[in] flags         Specifies the type of message reception. Support
 *                          for values other than 0 is not implemented yet.
 * @param[in] address       Points to a sockaddr structure containing the
 *                          destination address. The length and format of the
 *                          address depend on the address family of the socket.
 * @param[in] address_len   Specifies the length of the sockaddr structure pointed
 *                          to by the @p address argument.
 *
 *
 * @return  Upon successful completion, sendto() shall return the number of bytes
 *          sent. Otherwise, -1 shall be returned.
 */
int16_t udp_socket_sendto(int16_t socket, const void *buffer, size_t length, int16_t flags, \
               const struct sockaddr *address, int16_t address_len);




/**
 * @brief   Close a given socket.
 * @details The given socket is closed and all allocated resources are freed. No
 *          communication can take place after the socket has been closed.
 * @param socket Specifies the socket file descriptor.
 * @return The call to close() shall return 0 on success, -1 otherwise.
 */
int16_t udp_socket_close(int16_t socket);


 /* ================  SPECIFIC TO OPENUDP  ============================ */

/**
 * @brief Deliver a received message to the SOCKETS module. This will be used
          as a callback in the UDP service list.
 *        NOTE: the function will take care of freeing the message, as all other modules
 *              running over openUdp do.
 * @param msg Message to be delivered to sockets
 */
void udp_socket_deliver_msg_to_sockets(OpenQueueEntry_t* msg);

#endif