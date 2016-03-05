/* manageport.h
   by Zhang Haibin, 2002-10-11
 */

#ifndef _MANAGEPORT_H
#define _MANAGEPORT_H

extern int readAccessFD;
extern int writeAccessFD;
extern int readUDPFD;
#define MaxRegAlarm   5
#define MaxRegMonitor 5
#define MaxRegSt      5
#define MaxLogServer  5

struct THostInfo 
{
   char  address[128];
   unsigned short port;
   THostInfo()
   {
      port=0;
      address[0]=0;
   }
};

int openUDPPort(unsigned short portNumber);
int openUDPPort(const char* host, unsigned short portNumber);
void sendUDPMsg(int fd,char* host,unsigned short port,char* buffer,int length);

void initManagePort(char* name);
void udpSend(char* host,unsigned short port,char* buffer,int length);
void sendUdp(char* host,unsigned short port,char* buffer,int length);

void beginTermToUDPStr();
void pauseTermToUDPStr();
void restartTermToUDPStr();
int termToUDPStr(const char* s);
char* endTermToUDPStr(int *len);
char* getUDPPort(char* buffer,unsigned short* destPort);
void cmd_register(int argc,char* argv[]);
void cmd_unregister(int argc,char* argv[]);
void skipST(char *p);
void term(const char* s,...);

#if defined(_AIX) || defined(_LINUX)
#define SOCK_ADDR_LEN_TYPE socklen_t
#else
#if defined(_UNIXWARE) || defined(_SCO)
#define SOCK_ADDR_LEN_TYPE size_t
#else
#define SOCK_ADDR_LEN_TYPE int
#endif
#endif


#endif

