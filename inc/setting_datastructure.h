#ifndef _SETTING_DATASTRUCTURE_H
#define _SETTING_DATASTRUCTURE_H

#include "ring_buffer.h"

#define bool char
#define true 1
#define false 0 

typedef struct node
{
  
  char *type;                        //前几个数据项是由用户配置文件决定
  char *filterRules; 
  char *hostname; 
  int timeslice_mins;
  char *fixtime_starttime;
  int fixtime_duration_mins;
  

  bool firstpacket;
  pthread_t *tasknode_threadID;  //任务线程线程id
  pthread_attr_t *tasknode_attr; //任务线程模式
  struct ring_buffer *ring_buf; //线程对应的数据包信息缓冲队列
  int deviceindex_of_this_task;


  struct node* next;

}task_node;


typedef struct device
{
  char *devicename;
  task_node* head;        //每个设备对应的task链表对应的头指针
}devicelist;

//extern devicelist* arrayofdevice[];   //全局数组

   //指针数组，后面要为每个指针成员分配内存

task_node* alloc_node(char *t,char *fr,char* hn,int ts,char*fs,int fdm);
void push(task_node *head,char *t,char *fr,char* hn,int ts,char*fs,int fdm);
void print_list(task_node *head);
void pop(task_node **head);
void remove_last(task_node *head);
void remove_by_index(task_node** head,int n);


#endif

