/***********************************************************************

              Filename       :  common.c
              Author         :  qihua.wang, create on 2015-08-05
              Last modified  :  qihua.wang, 2015-08-05
              CopyRight(c)   :  2015 CINtel

***********************************************************************/

#include "common.h"

char        log_file[128] = {0};
char        fifo_file[128] = {0};
char        heartbeat[128] = {0};

int         log_fd = -1;
int         heartbeat_fd = -1;
int         access_port = -1;
int         access_flag = 0;
char        access_ip[32] = {0};
bool        trace = false;
extern int  udpPortNumber;
extern int  access_flag;
extern int  readUDPFD;

void log(const char *p, ...)
{
   char date[64], temp[8192];
   time_t sec;
   struct tm timer;

   sec = time(NULL);
   timer = *localtime(&sec);
   sprintf(date,"<%04d-%02d-%02d %02d:%02d:%02d> ", timer.tm_year+1900, timer.tm_mon+1, timer.tm_mday,
   timer.tm_hour, timer.tm_min, timer.tm_sec);
      
   va_list va;
   va_start(va, p);
   vsprintf(temp, p, va);
   va_end(va);

   if(log_fd <= 0)
   {
      log_fd = open(log_file, O_RDWR | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
      if(log_fd <= 0)
      {
         return;
      }
   }
   if(temp[0] != 0)
   {
      write(log_fd, date, strlen (date));
      write(log_fd, temp, strlen (temp));
   }
   else write(log_fd, "\n", 1);
}

void term(const char *p, ...)
{
   char temp[20480];

   va_list va;
   va_start(va, p);
   vsprintf(temp, p, va);
   va_end(va);

   if(writeAccessFD > 0 && access_flag)
      write(writeAccessFD, temp, strlen(temp));
   else if(access_port > 0 && access_flag) 
   {
      termToUDPStr(temp);
   }
   else
   {
      printf(temp);
   }
}

void closeTerm()
{
   int len;
   char temp[16];
   
   if(writeAccessFD > 0)
   {
      temp[0] = 0;
      len = write(writeAccessFD, temp, 1);
      close(writeAccessFD);
      writeAccessFD = -1;
   }
   if(readAccessFD > 0)
   {
      close(readAccessFD);
      readAccessFD = -1;
   }
   if(access_port > 0)
   {
      sprintf(temp, "%d ", udpPortNumber);
      len = strlen(temp);
      temp[len] = 2;
      len++;
      udpSend(access_ip, access_port, temp, len);
      close(readUDPFD);
      readUDPFD = -1;
      access_port = -1;
   }
}

void termBinary(char *p, int len)
{
   int i, l, x;
   char byte[4], ascII[32], line[256];
   
   i = 0;
   sprintf(line, " %-8d", i);
   strcpy(ascII, "   ");
   for(i = 0; i < len; i++)
   {
      if(i % 16 == 0 && i != 0)
      {
         term("%s%s\n", line, ascII);
         sprintf(line, " %-8d", i);
         strcpy(ascII, "   ");
      }
      if((unsigned char)p[i] < 32 || (unsigned char)p[i] > 126) strcat(ascII, ".");
      else
      {
         sprintf(byte, "%c", p[i]);
         strcat(ascII, byte);
      }
      sprintf(byte, "%02X ", (unsigned char)p[i]);
      strcat(line, byte);
   }
   if((len % 16) != 0)
   {
      l = 16 - (len % 16);
      for(x = 0; x < l ; x++) strcat(line, "   ");
   }
   term("%s%s\n", line, ascII);
}

void bcd2asc(char *p_bcd, int length, int o_e, char *p_asc)
{
   int   i;
   int   asc_i;
   char  byte;
   
   asc_i = 0;
   for(i = 0; i < length; i++)
   {
      byte = ((unsigned char)p_bcd[i] << 4) +((unsigned char)p_bcd[i] >> 4);
      sprintf(p_asc + asc_i, "%02X", (unsigned char)byte & 0xFF);
      asc_i += 2;
   }
   if(o_e)
   {
      p_asc[asc_i - 1] = 0;
   }
}
