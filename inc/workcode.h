#ifndef  _WORKCODE_C
#define  _WORKCODE_C

#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
#include <arpa/inet.h>
#define SNAP_LEN 1518       // 以太网帧最大长度
#define SIZE_ETHERNET 14   // 以太网包头长度 mac 6*2, type: 2
#define ETHER_ADDR_LEN  6  // mac地址长度



struct packet_ethernet {
    u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
    u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
    u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct packet_ip {
    u_char  ip_vhl;                 /* version << 4 | header length >> 2 长度不足一个字节，不能单独声明成一个变量，将version和headerlength拼在一个字段，采用位运算筛选出来*/
    u_char  ip_tos;                 /* type of service */
    u_short ip_len;                 /* total length */
    u_short ip_id;                  /* identification */
    u_short ip_off;                 /* fragment offset field */   //同上，采用宏定义将各标志位从ip_off中筛选出来
    #define IP_RF 0x8000           /* reserved fragment flag */ 
    #define IP_DF 0x4000            /* dont fragment flag */
    #define IP_MF 0x2000            /* more fragments flag */
    #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    u_char  ip_ttl;                 /* time to live */ 
    u_char  ip_p;                   /* protocol */
    u_short ip_sum;                 /* checksum */
    struct  in_addr ip_src,ip_dst;  /* source and dest address */
    //struct in_addr ip_src;
    //struct in_addr ip_dst;              /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)
 
/* TCP header */
typedef u_int tcp_seq;
 
struct packet_tcp {
    u_short th_sport;               /* source port */
    u_short th_dport;               /* destination port */
    tcp_seq th_seq;                 /* sequence number */
    tcp_seq th_ack;                 /* acknowledgement number */
    u_char  th_offx2;               /* data offset, rsvd */
    #define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
    u_char  th_flags;
    #define TH_FIN  0x01
    #define TH_SYN  0x02
    #define TH_RST  0x04
    #define TH_PUSH 0x08
    #define TH_ACK  0x10
    #define TH_URG  0x20
    #define TH_ECE  0x40
    #define TH_CWR  0x80
    #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;                 /* window */
    u_short th_sum;                 /* checksum */
    u_short th_urp;                 /* urgent pointer */
};

struct packet_icp
{
    u_short length;     //文档中有错误，实际上包头有两个字节的长度 
    u_char icp_PT;      /*Protocol Type*/
    u_char icp_FT;     /*Frame Typef*/
    #define icp_frame_type(icp)   ((icp)->icp_FT & 0x0f)
  
};



void loop_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

#endif 





