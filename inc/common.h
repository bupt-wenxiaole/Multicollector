#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "manageport.h"

const int MAX_COMMAND_LENGTH = 1024;
const int MAX_PARAMETER_NUMBER = 32;

const int SWITCH_OFF = 0;
const int SWITCH_ON = 1;


struct TCode
{
   char  *content;
   int   length;
   
   TCode()
   {
      content = NULL;
      length = 0;
   }
};

void log(const char *p, ...);
void term(const char *p, ...);
void termBinary(char *p, int len);
void closeTerm();
void bcd2asc(char *p_bcd, int length, int o_e, char *p_asc);

#endif

