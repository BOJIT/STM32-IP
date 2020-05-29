// #define NO_SYS                1
// #define NO_SYS_NO_TIMERS      1

#define MEM_ALIGNMENT 4
#define MEM_SIZE      (32*1024)

#define TCP_MSS  1460
#define TCP_SND_BUF (8*TCP_MSS)
#define TCP_WND     (10*TCP_MSS)
#define MEMP_NUM_TCP_SEG TCP_SND_QUEUELEN
#define MEMP_NUM_PBUF 32
#define MEMP_NUM_TCP_PCB 10

#define PBUF_POOL_SIZE 32

//Checksum handled by hardware
#define CHECKSUM_GEN_IP     0
#define CHECKSUM_GEN_UDP    0
#define CHECKSUM_GEN_TCP    0
#define CHECKSUM_GEN_ICMP   0
#define CHECKSUM_CHECK_IP   0
#define CHECKSUM_CHECK_UDP  0
#define CHECKSUM_CHECK_TCP  0
#define CHECKSUM_CHECK_ICMP 0

#define SYS_LIGHTWEIGHT_PROT            1

#define LWIP_ETHERNET 1
#define TCPIP_THREAD_STACKSIZE 1024
#define TCPIP_THREAD_PRIO 24
#define TCPIP_MBOX_SIZE 6

#define DEFAULT_THREAD_STACKSIZE 1024
#define DEFAULT_THREAD_PRIO 3
#define DEFAULT_UDP_RECVMBOX_SIZE 6
#define DEFAULT_TCP_RECVMBOX_SIZE 6
#define DEFAULT_ACCEPTMBOX_SIZE 6
#define RECV_BUFSIZE_DEFAULT 2000000000

#define LWIP_DHCP                   1
#define LWIP_AUTOIP                 1
#define LWIP_DHCP_AUTOIP_COOP       1
#define LWIP_DHCP_AUTOIP_COOP_TRIES 5

#define LWIP_NETIF_HOSTNAME        1
#define LWIP_NETIF_API             0
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK   1

#define LWIP_SOCKET  0
#define LWIP_NETCONN 0

#define SO_REUSE     1
#define LWIP_IGMP    1

// #include <stmlib/rand.h>
// #define LWIP_RAND    rand_value

#define LWIP_STATS         1
#define LWIP_STATS_DISPLAY 1
