/*
file:	conv.C
author:	justing
time:	2015-10-29
func:	conveger process
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conv.hpp"
#include "common.h"

extern TConvManager *convmanager;

TConvNode::TConvNode()
{
    id=0;
    state=0;
    name[0]=0;
}

TConvNode::~TConvNode()
{
    
}
    
int TConvNode::check(char* path, char* time)
{
    if(path==NULL || time==NULL)
        return -1;
    char file[128]={0};
    sprintf(file, "%s/%s_%s.%s", path, name, time, SUFFIX);
    
    if(access(file, 0)==-1)
    {
        log("check %s is not exist, wait!\n", file);
    }
    else
    {
        char temp[512]={0};
        sprintf(temp, "mv %s %s", file, convmanager->temppath);
        system(temp);
        return 1;
    }
    
    return 0;
}

TConvManager::TConvManager()
{
    pushpath=NULL;
    base=0;
    interval=5;
}

TConvManager::~TConvManager()
{
    if(pushpath!=NULL)
        free(pushpath);
    pushpath=NULL;
    
    if(temppath!=NULL)
        free(temppath);
    temppath=NULL;
    
    if(!data.empty())
    {
        ListConv::iterator   item = data.begin();
        while(item!=data.end())
        {
            if(*item)
            {
                PTConvNode node = (PTConvNode)(*item);
                delete node;
                node = NULL;
            }
            item++;
        }
    }
    data.clear();
}

void TConvManager::init()
{
    char config_file[128];
    
    sprintf(config_file, "%s/etc/converge.cfg", getenv("CINDIR"));
    FILE *f=NULL;
    f = fopen(config_file, "rt");
    if(f==NULL)
    {
        log("error. converge.cfg open failed!\n");
        exit(-1);
    }
    
    char buffer[MAX_BUFFER]={0};
    char key[128]={0};
    char value[128]={0};
    while(fgets(buffer, MAX_BUFFER, f)!=NULL)
    {
        if(buffer[0]!='#' && buffer[0]!='\n')
        {
            int len=0;
            char *p=NULL;
            char *q=NULL;
            p = strchr(buffer, '=');
            if(p!=NULL)
            {
                len = p-buffer;
                memcpy(key, buffer, len);
                key[len]=0;
                q = strchr(buffer, '\n');
                if(q!=NULL)
                {
                    memcpy(value, p+1, q-p-1);
                    value[q-p-1]=0;
                }
                else
                {
                    strcpy(value, buffer+len+1);
                    value[strlen(value)]=0;
                }
                
                if(strcasecmp(key, "push_path")==0)
                {
                    pushpath = (char*)malloc(sizeof(char)*strlen(value)+1);
                    strcpy(pushpath, value);
                }
                else if(strcasecmp(key, "temp_path")==0)
                {
                    temppath = (char*)malloc(sizeof(char)*strlen(value)+1);
                    strcpy(temppath, value);
                }
                else if(strcasecmp(key, "storage_dir")==0)
                {
                    stordir = (char*)malloc(sizeof(char)*strlen(value)+1);
                    strcpy(stordir, value);
                }
                else if(strcasecmp(key, "node")==0)
                {
                    PTConvNode node = new TConvNode();
                    strcpy(node->name, value);
                    node->id = data.size();
                    data.push_back(node);
                }
                else if(strcasecmp(key, "base")==0)
                {
                    base = atoi(value);
                    if(base<0)
                        base=0;
                }
                else if(strcasecmp(key, "interval")==0)
                {
                    interval = atoi(value);
                    if(interval<=0)
                        interval=5;
                }
                else if(strcasecmp(key, "delay")==0)
                {
                    delay = atoi(value);
                    if(delay<0)
                        delay=0;
                }
            }
        }
    }
    
    if(interval<=base)
        base=0;
    delay *= 1000;
    
    fclose(f);
    if(pushpath==NULL || temppath==NULL || stordir==NULL)
    {
        printf("error, converge.cfg needs push_path, temp_path and storage dir!\n");
        exit(0);
    }
    
    mkdir(pushpath,S_IRWXU);
    mkdir(temppath,S_IRWXU);
    mkdir(stordir,S_IRWXU);
}

void TConvManager::converge(struct tm timer)
{
    char temp[512],fileName[128],storagepath[128];
    
    //mergecap
    int month = timer.tm_mon+1;
    int mday = timer.tm_mday;
    sprintf(temp, "%s/%02d", stordir, month);
    mkdir(temp, S_IRWXU);
    
    sprintf(storagepath, "%s/%02d/%02d", stordir, month, mday);
    mkdir(storagepath, S_IRWXU);
    
    sprintf(fileName, "%04d%02d%02d%02d%02d.pcap",timer.tm_year+1900, 
                timer.tm_mon+1, 
                timer.tm_mday,
                timer.tm_hour, 
                timer.tm_min-base);
    
    sprintf(temp, "mergecap -w %s/%s %s/*.pcap", storagepath,fileName,temppath);
    system(temp);
    
    sprintf(temp, "rm -rf %s/*.pcap", temppath);
    system(temp);
    
    log("%s.pcap converge complete!\n", fileName);
}

extern int last_min;
extern bool first;
void TConvManager::loop()
{
    int res=0, count=0;
    char collect_time[14]={0};
    time_t sec;
    sec = time(NULL);
    struct tm con_timer = *localtime(&sec);
    
    if(con_timer.tm_min!=last_min || first)
    {
        if(first)
            first=false;
        if(con_timer.tm_min%interval==base)
        {
            sprintf(collect_time, "%04d%02d%02d%02d%02d",con_timer.tm_year+1900, 
                con_timer.tm_mon+1, 
                con_timer.tm_mday,
                con_timer.tm_hour, 
                con_timer.tm_min-base);
                
            if(!data.empty())
            {
                ListConv::iterator item = data.begin();
                while(item!=data.end())
                {
                    PTConvNode node = (PTConvNode)(*item);
                    res = node->check(pushpath, collect_time);
                    
                    if(res==0 && delay>0)
                    {
                        usleep(delay);
                        if(node->check(pushpath, collect_time)==1)
                            count++;
                    }
                    else if(res==1)
                        count++;
                    item++;
                }
                
                if(count<data.size())
                {
                    log("converge is not complete!\n");
                }
                
                if(count>0)
                    converge(con_timer);
            }
        }
        last_min = con_timer.tm_min;
    }
}
