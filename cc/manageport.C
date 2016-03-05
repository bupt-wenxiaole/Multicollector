/*
   manageport.C
   by Zhang Haibin, 2002-10-11 
 */
//for ui compile
//#include "memdebug.h"
#ifndef WIN32
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#ifdef _HP
#include <sys/time.h>
#else
#include <sys/select.h>
#endif
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <netdb.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "manageport.h"

#ifdef WIN32
//#define errno WSAGetLastError()
#define close closesocket
//#define write(x, y, z) ::send(x, y, z, 0)
//#define read(x, y, z) recv(x, y, z, 0)
#endif

#ifdef WIN32
void InitSocketWin32()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Can't Init Socket In Win32\n");
		return;
	}
}
#endif

int readAccessFD=-1;
int writeAccessFD=-1;
int readUDPFD=-1;
unsigned short udpPortNumber=0;

THostInfo regAlarm[MaxRegAlarm];
THostInfo regMonitor[MaxRegMonitor];
THostInfo regSt[MaxRegSt];
THostInfo regLogServer[MaxLogServer];

int openUDPPort(unsigned short portNumber)
{
   int fd=socket(AF_INET,SOCK_DGRAM,0);
   if(fd==-1)
   {
      perror("socket(): ");
      return -1;
   }

#ifdef WIN32
   u_long value = 1;
   ioctlsocket(fd,FIONBIO,&value);
#else
   fcntl(fd,F_SETFL,O_NDELAY);
   if(fcntl(fd,F_SETFD,FD_CLOEXEC)==-1)
      printf("openUDPPort: %d set fd_cloexec error(%d)", fd, errno);
#endif
/*
   int reuse_addr_flag=1;
   int success=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse_addr_flag,sizeof(reuse_addr_flag));
   if(success<0)
   {
      perror("udp setsockopt(SO_REUSEADDR): ");
      close(fd);
      return -1;
   }
 */  
#ifdef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=256*1024;
   sendBufLength=sizeof sendBufSize;
   
   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
#endif

#ifndef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=64*1024;
   sendBufLength=sizeof sendBufSize;
   
   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
   if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_RCVBUF): ");
   }
#endif

   struct sockaddr_in myAddr;
   myAddr.sin_family = AF_INET;
   myAddr.sin_port = htons(portNumber);
   myAddr.sin_addr.s_addr = INADDR_ANY;

   int success=bind(fd,(struct sockaddr *)&myAddr,sizeof(struct sockaddr));
   if(success==-1)
   {
      perror("udp bind(): ");
      close(fd);
      return -1;
   }
   return fd;
}

int openUDPPort(const char* host, unsigned short portNumber)
{
   int fd=socket(AF_INET,SOCK_DGRAM,0);
   if(fd==-1)
   {
      perror("socket(): ");
      return -1;
   }

#ifdef WIN32
   u_long value = 1;
   ioctlsocket(fd,FIONBIO,&value);
#else
   fcntl(fd,F_SETFL,O_NDELAY);
   if(fcntl(fd,F_SETFD,FD_CLOEXEC)==-1)
      printf("openUDPPort: %d set fd_cloexec error(%d)", fd, errno);
#endif
/*
   int reuse_addr_flag=1;
   int success=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse_addr_flag,sizeof(reuse_addr_flag));
   if(success<0)
   {
      perror("udp setsockopt(SO_REUSEADDR): ");
      close(fd);
      return -1;
   }
 */  
#ifdef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=256*1024;
   sendBufLength=sizeof sendBufSize;
   
   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
#endif

#ifndef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=64*1024;
   sendBufLength=sizeof sendBufSize;
   
   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
   if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_RCVBUF): ");
   }
#endif

   struct sockaddr_in myAddr;
   myAddr.sin_family = AF_INET;
   myAddr.sin_port = htons(portNumber);

#ifdef WIN32
#define in_addr_t unsigned long 
#endif
   in_addr_t net_addr=inet_addr(host);
   if(net_addr==(in_addr_t)-1)
   {
      myAddr.sin_addr.s_addr = INADDR_ANY;
   }
   else
   {
      memcpy((struct sockaddr*)&myAddr.sin_addr,(void *)(&net_addr),sizeof(net_addr));
   }
   
   int success=bind(fd,(struct sockaddr *)&myAddr,sizeof(struct sockaddr));
   if(success==-1)
   {
      perror("udp bind(): ");
      close(fd);
      return -1;
   }
   return fd;
}

int getIPAddressByHost(const char* hostName, char* hostIPAddr)
{
   struct hostent *hostAddr;
   if ((hostAddr=gethostbyname(hostName)) == NULL)
      return -1;
   struct in_addr *temp;
   temp=(struct in_addr *)hostAddr->h_addr;
   strcpy(hostIPAddr,inet_ntoa(*temp));
   return 0;
}

int matchHostName(const char* hostName)
{
   char hostAddr[32];
   if(getIPAddressByHost(hostName, hostAddr)!=0)
      return 0;
   char myHostName[256];
   gethostname(myHostName,sizeof(myHostName));
   char myHostAddr[32];
   if(getIPAddressByHost(myHostName, myHostAddr)!=0)
      return 0;
   if(strcmp(hostAddr, myHostAddr)!=0)
      return 0;
   return 1;
}

void initManagePort(char* name)
{
   char* cinDir=getenv("CINDIR");
   if(cinDir==NULL)
   {
      printf("CINDIR not set\n");
      exit(1);
   }
   char configFileName[256];
   sprintf(configFileName,"%s/etc/config.managementports",cinDir);
   FILE* f=fopen(configFileName,"rt");
   unsigned short portNumber=0;
   if(f!=NULL)
   {
      char temp[1024];
      while(fgets(temp,sizeof(temp),f)!=NULL)
      {
         if(temp[0]!='#' && temp[0]!='\n')
         {
            char objectName[1024];
            int objectPort=0;
            objectName[0]=0;
            sscanf(temp,"%s %d",objectName,&objectPort);
            char* hostName=strstr(objectName,"@");
            if(hostName!=NULL)
            {
               *hostName=0;
               if(!matchHostName(hostName+1))
                  continue;
            }
            if(strcmp(name,objectName)==0)
            {
               portNumber=objectPort;
               udpPortNumber=portNumber; // beginTermToUDPStr use
               break;
            }
         }
      }
      fclose(f);
   }
#ifndef WIN32
   if(portNumber==0)
   {
      char readFifoName[256];
      char writeFifoName[256];
      sprintf(readFifoName,"%s/fifo/access.%s",cinDir,name);
      sprintf(writeFifoName,"%s/fifo/%s.access",cinDir,name);
      
      mkfifo(readFifoName, S_IFIFO|0666);
      if((readAccessFD=open(readFifoName,O_RDONLY|O_NDELAY))<0)
      {
         printf("can not open readfifo %s.\n",readFifoName);
         exit(0);
      }
      
      mkfifo(writeFifoName, S_IFIFO|0666);
      if((writeAccessFD=open(writeFifoName,O_RDWR|O_NDELAY))<0)
      {
         printf("can not open writefifo %s.\n",writeFifoName);
         exit(0);
      }
      return;
   }
#endif
   readUDPFD=openUDPPort(portNumber);
   if(readUDPFD<0)
   {
      printf("can not open udp port %d\n",udpPortNumber);
      exit(1);
   }
}

void udpOpenAndSend(char* host,unsigned short port,char* buffer,int length)
{
   int fd=socket(AF_INET,SOCK_DGRAM,0);
   if(fd==-1)
   {
      perror("socket(): ");
      return ;
   }

#ifdef WIN32
   u_long value = 1;
   ioctlsocket(fd,FIONBIO,&value);
#else
   fcntl(fd,F_SETFL,O_NDELAY);
#endif

   int reuse_addr_flag=1;
   int success=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse_addr_flag,sizeof(reuse_addr_flag));
   if(success<0)
   {
      perror("udp setsockopt(SO_REUSEADDR): ");
      close(fd);
      return ;
   }

#ifdef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=256*1024;
   sendBufSize=256*1024;
   sendBufLength=sizeof sendBufSize;

   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
#endif

#ifndef WIN32
   int sendBufSize,sendBufLength;
   sendBufSize=64*1024;
   sendBufLength=sizeof sendBufSize;

   if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_SNDBUF): ");
   }
   if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char*)&sendBufSize,sendBufLength)<0)
   {
      perror("udp setsockopt(SO_RCVBUF): ");
   }
#endif

   sendUDPMsg(fd,host,port,buffer,length);
   if(fd>0)
      close(fd);
}

/* send udp msg by console manage port.  
   this port configed in config.managementports.  */
void udpSend(char* host,unsigned short port,char* buffer,int length)
{
   sendUDPMsg(readUDPFD,host,port,buffer,length);
}

/* send udp msg by common port,this port created by anytime when send udp msg.*/
void sendUdp(char* host,unsigned short port,char* buffer,int length)
{
   udpOpenAndSend(host,port,buffer,length);
}

void sendUDPMsg(int fd,char* host,unsigned short port,char* buffer,int length)
{    
   int success;
#ifdef WIN32
#define in_addr_t unsigned long 
#endif
   in_addr_t net_addr=inet_addr(host);
   if(net_addr==(in_addr_t)-1)
   {
#ifdef WIN32
      net_addr = 0;
#else
      bzero((char *)(&net_addr),sizeof(net_addr));
#endif
      hostent *hp;
      hp = gethostbyname(host);
      if(hp==NULL)
      {
         perror("gethostbyname()");
         //close(fd);
         return;
      }
      memcpy((void *)(&net_addr),(void *)hp->h_addr,hp->h_length);
   }

   struct sockaddr_in destAddr;
   destAddr.sin_family = AF_INET;
   destAddr.sin_port = htons(port);
   memcpy((struct sockaddr*)&destAddr.sin_addr,(void *)(&net_addr),sizeof(net_addr));
   success = sendto(fd,buffer,length,0,(struct sockaddr *)&destAddr,sizeof(struct sockaddr));

   if(success<0)
   {
      char *buf = new char[length + 128];
      sprintf(buf, "sendto(%d, %s, %d, %s, %d):", fd, buffer, length, host, port);
      perror(buf);
      delete []buf;
   }
}

int termToUDPStrFlag=0;
//#define MaxTermUDPStrLen 65536 //for fsm command(many CCB var)
#define MaxTermUDPStrLen 120000
char termTempUDPStr[MaxTermUDPStrLen];
int termTempUDPStrPointer=0;
char udpVersion[10]={0};
char udpSeqNo[128]={0};

//generate head info.
void beginTermToUDPStr()
{
   if(udpVersion[0]!=0)
   {
      sprintf(termTempUDPStr,"%d %s %s ",udpPortNumber,udpVersion,udpSeqNo);
      termToUDPStrFlag=1;
      termTempUDPStrPointer=strlen(termTempUDPStr);
   }
   else
   {
      sprintf(termTempUDPStr,"%d ",udpPortNumber);
      termToUDPStrFlag=1;
      termTempUDPStrPointer=strlen(termTempUDPStr);
   }
}

int termToUDPStr(const char* s)
{
   if(termToUDPStrFlag)
   {
      int len=strlen(s);
      if(termTempUDPStrPointer+len+1<MaxTermUDPStrLen)
      {
         termTempUDPStrPointer+=len;
         strcat(termTempUDPStr,s);
      }
      return 1;
   }
   return 0;
}

void pauseTermToUDPStr()
{
   termToUDPStrFlag=0;
}

void restartTermToUDPStr()
{
   termToUDPStrFlag=1;
}

char* endTermToUDPStr(int *len)
{
   termToUDPStrFlag=0;
   *len=termTempUDPStrPointer;
   return termTempUDPStr;
}

char* getUDPPort(char* buffer,unsigned short* destPort)
{
   int port=0;
   sscanf(buffer,"%d",&port);
   *destPort=port;
   char* p=buffer;
   while(*p>='0'&&*p<='9'&&*p!=0)
      p++;
   while(*p!=0 && (*p==' ' || *p=='\t' || *p=='\n'))
      p=p+1;
   udpVersion[0]=0;
   udpSeqNo[0]=0;
   if(memcmp(p,"v001",4)==0)
   {
      strcpy(udpVersion,"v001");
      p+=4;
      while(*p!=0 && (*p==' ' || *p=='\t' || *p=='\n'))
         p=p+1;
      int i=0;
      while(*p!=0 && *p!=' ' && *p!='\t' && *p!='\n')
      {
         if(i<127)
         {
           udpSeqNo[i]=*p;
           i++;
         }
         p=p+1;
      }
      udpSeqNo[i]=0;
   }
   return p;
}

extern THostInfo regAlarm[MaxRegAlarm];
extern THostInfo regMonitor[MaxRegAlarm];
extern THostInfo regSt[MaxRegAlarm];

void cmd_register(int argc,char* argv[])
{
   int i;
   unsigned short regPort=0;
   char alarmUDPAddress[128];
   char regAddress[128];
   THostInfo *regService;
   int maxRegCount=0;

   if(argc==4 || argc == 5) 
   {
      if(memcmp(argv[1],"alarm",5)==0 ||memcmp(argv[1],"monitor",7)==0
         ||memcmp(argv[1],"st",2)==0 ||memcmp(argv[1],"log",3)==0)
      {
         if( memcmp(argv[1],"alarm",5)==0 )
         { 
            regService=&regAlarm[0];
            maxRegCount=MaxRegAlarm;
         }
         if( memcmp(argv[1],"monitor",7)==0) 
         {
            regService=&regMonitor[0];
            maxRegCount=MaxRegMonitor;
         }
         if( memcmp(argv[1],"st",2)==0) 
         {
            regService=&regSt[0];
            maxRegCount=MaxRegSt;
         }
         if( memcmp(argv[1],"log",3)==0)
         {
            regService=&regLogServer[0];
            maxRegCount=MaxLogServer;
         }
         regPort= atoi(argv[3]);
         int i1,i2,i3,i4;
         sscanf(argv[2],"%d.%d.%d.%d",&i1,&i2,&i3,&i4);
         if(i1>255 || i1<0 || i2>255 || i2<0 || i3>255 || i3<0 || i4>255 ||i4 <0)          
         {
            term("<register>\n");
            term("   <ERR>invalid address</ERR>\n");
            term("</register>\n");
            return;
         }
         strcpy(regAddress,argv[2]); 

         //repeat register 
         for(i=0;i<maxRegCount;i++)
         {
            if(strcmp(regService[i].address,regAddress)==0 
               && regService[i].port==regPort)
            {
               term("register success\n");
               return;
            }
         }

         //first register
         for(i=0;i<maxRegCount;i++)
         {
            if(regService[i].port==0)
            {
               strcpy(regService[i].address,regAddress);
               regService[i].port=regPort;
               term("register success\n");
               return;
            }
         }

         term("<register>\n");
         term("   <ERR>max regservice count limit</ERR>\n");
         term("</register>\n");
         return;
      }
      else
         goto usage;
   }

   if(argc==2)
   {
      if(memcmp(argv[1],"-l",2)==0)
      {
         term("<register>\n");
         for(i=0;i<MaxRegAlarm;i++)
         {
            if(regAlarm[i].port!=0)
            {
               term("   <alarm>\n");
               term("      <address> %s </address>\n",regAlarm[i].address);
               term("      <portNumber> %u </portNumber>\n",regAlarm[i].port);
               term("   </alarm>\n");
            }
         }
         for(i=0;i<MaxRegMonitor;i++)
         {
            if(regMonitor[i].port!=0)
            {
               term("   <monitor>\n");
               term("      <address> %s </address>\n",regMonitor[i].address);
               term("      <portNumber> %u </portNumber>\n",regMonitor[i].port);
               term("   </monitor>\n");
            }
         }
         for(i=0;i<MaxRegSt;i++)
         {
            if(regSt[i].port!=0)
            {
               term("   <st>\n");
               term("      <address> %s </address>\n",regSt[i].address);
               term("      <portNumber> %u </portNumber>\n",regSt[i].port);
               term("   </st>\n");
            }
         }
         for(i=0;i<MaxLogServer;i++)
         {
            if(regLogServer[i].port!=0)
            {
               term("   <logServer>\n");
               term("      <address> %s </address>\n",regLogServer[i].address);
               term("      <portNumber> %u </portNumber>\n",regLogServer[i].port);
               term("   </logServer>\n");
            }
         }

         term("</register>\n");
      }
      else
         goto usage;
      return;
   }
usage:
         term("<usage>register [alarm|monitor|st|log] ipaddress portnumber</usage>\n");
         term("<usage>register -l</usage>\n");
}

void cmd_unregister(int argc,char *argv[])
{
   THostInfo *regService;
   int maxRegService=0;

   int i;
   if(argc>=3)
   {
      if(memcmp(argv[1],"alarm",5)==0 || memcmp(argv[1],"monitor",7)==0 
         || memcmp(argv[1],"st",2)==0 || memcmp(argv[1],"log",3)==0)
      {
         if(memcmp(argv[1],"alarm",5)==0){ regService=regAlarm; maxRegService=MaxRegAlarm;}
         if(memcmp(argv[1],"st",2)==0) {regService=regSt;maxRegService=MaxRegSt;}
         if(memcmp(argv[1],"log",3)==0) {regService=regLogServer;maxRegService=MaxLogServer;}
         if(memcmp(argv[1],"monitor",7)==0) {regService=regMonitor;maxRegService=MaxRegMonitor;}
         for(i=0;i<maxRegService;i++)
         {
            if(regService[i].port==atoi(argv[3])&&strcmp(regService[i].address,argv[2])==0)
            {
               regService[i].port=0;
               regService[i].address[0]=0;
            }
         }
         term("OK.");
         return;
      }
      else
      {
         term("<ERR> no register service %s </ERR>",argv[1]);
         return;
      }
   }
   term("<useage> unregister servicename ipaddress portnumber </usage>\n");
}
