/***********************************************************************

              Filename       :  converge.c
              Author         :  justing, create on 2015-10-29
              Last modified  :  justing, 2015-10-29
              CopyRight(c)   :  2015 CINtel

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "manageport.h"
#include "common.h"
#include "conv.hpp"
#include "version.h"

#define MODULE      "converge"
#define FIFO        "CONVERGE"

bool step_flag = false;

extern "C" { void on_chld(int); }
extern "C" { void on_intr(int); }
extern "C" { void on_pipe(int); }
extern "C" { void on_bus(int); }
extern "C" { void on_segv(int); }
extern "C" { void on_alarm(int); }

extern char log_file[];
extern char fifo_file[];
extern char heartbeat[];
extern int  heartbeat_fd;
int last_min;
bool first = true;
TConvManager *convmanager;

extern  const char *version;

void on_intr(int)
{
    log("%s receive a interrupt whith CTL_C\n", MODULE);
    signal(SIGINT,on_intr);
    exit(0);
}

void on_chld(int)
{
    //log("%s receive SIGCHLD signal ! \n", MODULE);
    signal(SIGCHLD,on_chld);
}

void on_pipe(int)
{
    log("%s write on a pipe with no one to read it !\n", MODULE);
    signal(SIGPIPE,on_pipe);
}

void on_bus(int)
{
    log("%s receive SIGBUS signal !\n", MODULE);
    signal(SIGBUS,on_bus);
    exit(0);
}

void on_segv(int)
{
    log("%s receive SIGSEGV signal\n", MODULE);
    signal(SIGSEGV,on_segv);
    exit(0);
}

void on_alarm(int)
{
    step_flag = true;
    signal(SIGALRM, on_alarm);
    alarm(1);
    return;
}

void initSignal()
{
    signal(SIGCHLD, on_chld);
    signal(SIGINT, on_intr);
    signal(SIGPIPE, on_pipe);
    signal(SIGBUS, on_bus);
    signal(SIGSEGV, on_segv);
    signal(SIGALRM, on_alarm);
    alarm(1);
}

void initSystem()
{
    char *p_cin_path;
    
    p_cin_path = getenv("CINDIR");
    if(!p_cin_path)
    {
        printf("please set the CINDIR env variables first !\n");
        exit(0);
    }
    snprintf(log_file, 128, "%s/%s.log", p_cin_path, MODULE);
    snprintf(fifo_file, 128, "%s", MODULE);
    snprintf(heartbeat, 128, "%s/fifo/SCF_CONTROL_%s", p_cin_path, FIFO);
    initManagePort(MODULE);
    
    if((heartbeat_fd = open(heartbeat, O_RDWR|O_NDELAY)) < 0)
    {
        log("%s open heartbeat fifo file[%s] failed !\n", heartbeat, MODULE);
        exit(0);
    }
}

void printversion()
{
    printf("\n_______________________________________________________\n\n");
    printf("%s %s\n", MODULE, version);
    printf("CopyRight (c) CINtel. All Rights reserved.\n");
    printf("_______________________________________________________\n");
}

void step()
{
    step_flag = false;
    if(0 < heartbeat_fd)
    {
        write(heartbeat_fd, "1", 1);
    }
}

int main(int argc, char *argv[])
{
    if(argc==1 || (argc>1 && strcmp(argv[1], FIFO)!=0))
    {
        printversion();
        exit(0);
    }
    
    initSystem();
    initSignal();
    convmanager = new TConvManager();
    convmanager->init();
    log("%s start !\n", MODULE);
    printf("%s start !\n", MODULE);
    
    while(1)
    {
        convmanager->loop();
        
        if(step_flag)
        {
            step();
        }
        
        usleep(10);
    }
}

