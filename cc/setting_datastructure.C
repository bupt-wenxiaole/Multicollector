#include "setting_datastructure.h"


extern devicelist* arrayofdevice; //定义写在.c当中，声明写在.h当中


task_node* alloc_node(char *t,char *fr,char* hn,int ts,char*fs,int fdm)
{
  task_node* plinkNode=NULL;
  plinkNode = (task_node *)malloc(sizeof(task_node));

  plinkNode->type=t;
  plinkNode->filterRules=fr;
  plinkNode->hostname=hn;
  plinkNode->timeslice_mins=ts;
  plinkNode->fixtime_starttime=fs;
  plinkNode->fixtime_duration_mins=fdm;

  plinkNode->tasknode_threadID=NULL;
  plinkNode->tasknode_attr=NULL;
  plinkNode->ring_buf=NULL;
  



  plinkNode->next=NULL;
  return plinkNode;
}

void push(task_node *head,char *t,char *fr,char* hn,int ts,char*fs,int fdm)
{
  task_node *current=head;
  while(current->next!=NULL)
  {
    current=current->next;
  }

  current->next=(task_node*)malloc(sizeof(task_node));
  current->next->type=t;
  current->next->filterRules=fr;
  current->next->hostname=hn;
  current->next->timeslice_mins=ts;
  current->next->fixtime_starttime=fs;
  current->next->fixtime_duration_mins=fdm;
  
  current->next->tasknode_threadID=NULL;
  current->next->tasknode_attr=NULL;
  current->next->ring_buf=NULL;
  
  current->next->next=NULL;
}

void print_list(task_node *head)
{
  task_node* current=head;

  //while(current!=NULL)
  //{
    printf("%s\n",current->type);
    printf("%s\n", current->filterRules);
    printf("%s\n", current->hostname);
    printf("%d\n", current->timeslice_mins);
    printf("%s\n", current->fixtime_starttime);
    printf("%d\n", current->fixtime_duration_mins);
    printf("@@@@@@@@@@@@@@@@\n");

  //}
    //current=current->next;

}

void pop(task_node **head)
{
  task_node* next_node = NULL;
  if(*head==NULL)
  {
    return ;
  }
  next_node=(*head)->next;
  free(*head);
  *head=next_node;

}

void remove_last(task_node *head)
{
  
  /* if there is only one item in the list, remove it */
  if(head->next==NULL)
  {
    
    free(head);
    head=NULL;//防止野指针
    
  }

  task_node * current=head;

  while (current->next->next != NULL) {   //找到倒数第二个节点
        current = current->next;
    }
    task_node *q;
    q=current->next;
    current->next=NULL;//原来的链表到此结束
    free(q);

}


void remove_by_index(task_node** head,int n)
{
  int i=0;
  task_node *current=*head;
  task_node *temp_node=NULL;

  if(n==0)
  {
    return pop(head);
  }

  for (;i<n-1;i++)
  {
    if(current->next==NULL)
      return ;
    current=current->next;
  }

  temp_node=current->next;
  current->next=temp_node->next;
  free(temp_node);

}

