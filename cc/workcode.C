/***********************************************************************

              Filename       :  workcode.c
              Author         :  xiaolewen_bupt, create on 2015-11-03
              Last modified  :  xiaolewen_bupt 2015-12-21
              CopyRight(c)   :  2015 CINtel

***********************************************************************/



#include "workcode.h"
#include "configfile.h"
#include "setting_datastructure.h"
#include <time.h>
#include <pthread.h>
#include <sched.h> 
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "manageport.h"
#include "common.h"
#include "version.h"
#include "ring_buffer.h"

#define MODULE      "hssp"
#define FIFO        "HSSP"

#define MAX_NUM_OF_DEVICE 10  //网络设备的最大数目
#define MAX_NUM_OF_TASKNODE 20 //每个网络设备下对应的最多的任务数
#define SFDelete( ptr )    do { if ( ptr ) { free ( ptr ); ( ptr ) = NULL; } } while(0)  

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


devicelist* arrayofdevice[MAX_NUM_OF_DEVICE];   //全局数组

extern int count;//网卡个数,利用这个count来开线程，一张网卡对应一个父线程,每个父线程对应多个task子线程，每个子线程对应各自的回调函数

#define filenamelen 50 //主机名+起始时间+结束时间

const char *filetype=".pcap";   //文件后缀类型

pcap_t *handle[MAX_NUM_OF_DEVICE]; /* 会话句柄数组*/

extern const char *version;

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
    //log("%s receive SIGSEGV signal\n", MODULE);
    signal(SIGSEGV,on_segv);
    //exit(0);
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

void *ftptostorage(void *args)
{
    char *filename=(char *)args;
    char temp[128]={0};
    sprintf(temp, "%s/cin/bin/col_ftp %s", getenv("HOME"), filename);
    system(temp);   //执行脚本+参数的命令
}


//void loop_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
//{
//
//    const struct packet_ethernet *ethernet;  /* The ethernet header [1] */
//    const struct packet_ip *ip;              /* The IP header */
//    const struct packet_tcp *tcp;            /* The TCP header */
//    const struct packet_icp *icp;            /*The partial icp header*/
//    
//    struct tm *p;     //两个用来指向暂存换算秒数的指针
//    time_t q;
//
//   
//
//    p=localtime(&(header->ts.tv_sec));     //结构体p中保存了时间的每个分量 
//    int currentpacketmin=p->tm_min;
//    int endtime=((currentpacketmin)/result.timeslice+1)*result.timeslice%60;
//    
//    int size_ip;
//    int size_tcp;
//    /* 以太网头 */ 
//    ethernet = (struct packet_ethernet*)(packet);
// 
//    /* IP头 */
//    ip = (struct packet_ip*)(packet + SIZE_ETHERNET);
//    size_ip = IP_HL(ip)*4;
//    if (size_ip < 20) {
//        printf("无效的IP头长度: %u bytes\n", size_ip);
//        return;
//    }
// 
//    if ( ip->ip_p != IPPROTO_TCP ){ // TCP,UDP,ICMP,IP
//    return;
//    }
// 
//    /* TCP头 */
//    tcp = (struct packet_tcp*)(packet + SIZE_ETHERNET + size_ip);
//    size_tcp = TH_OFF(tcp)*4;
//    if (size_tcp < 20) {
//        printf("无效的TCP头长度: %u bytes\n", size_tcp);
//        return;
//    }
// 
//    int sport =  ntohs(tcp->th_sport);
//    int dport =  ntohs(tcp->th_dport);
//    int is_useful=(icp_frame_type(icp));
//    if(is_useful!=0 && is_useful!=1 && is_useful!=15)
//    {   //这三个数字用宏定义的形式描述
//        if (firstpacket)
//        {
//                
//            //求当前分钟数之前最近的那个起始分钟数
//            int nearstarttime=(currentpacketmin)/result.timeslice*result.timeslice;
//            //求文件命名时间（用上面得到的分钟数补全成年月日时间换算成秒＋时间片＊60得秒数后再换算为截止命名时间）
//            //使用函数time_t mktime(struct tm *tm); //将struct tm 结构的时间转换为从1970年至今的秒数 
//            p->tm_min=nearstarttime;
//            q=mktime(p);
//            q+=result.timeslice*60;
//            p=localtime(&q);
//            
//            lasttime=endtime; //这个endtime用来控制文件的切换
//            sprintf(filetowrite,"%s%s_%d%02d%02d%02d%02d%s",result.filepath,result.hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
//            dumpfp=pcap_dump_open(handle,filetowrite);
//            if(dumpfp == NULL)
//            {
//                printf("Erro on opening outputfile\n");
//                return;
//            }
//    
//            if(packet!=NULL)
//                pcap_dump((u_char *)dumpfp,header,packet);   //open用文件地址做参数，dump用打开文件句柄做参数
//            firstpacket=false;
//            return;
//
//
//        }   
//
//  
//
//        if ((currentpacketmin % result.timeslice ==0  && endtime!=lasttime) || endtime-lasttime >=result.timeslice)   //用endtime和lasttime来控制模时间片为0的那一分钟不会不断打开新文件
//        {
//            
//            pcap_dump_close(dumpfp);  //更换写入文件后开始执行ftp至远端的操作
//            //设置线程绑定模式为非分离模式;
//            pthread_attr_t attr; 
//            pthread_t tid;
//            pthread_attr_init(&attr); 
//            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
//            char filenamewithoutpath[50];
//            sprintf(filenamewithoutpath,"%s_%04d%02d%02d%02d%02d%s",result.hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
//            //由于ftp脚本的规则，只取文件名
//            //create一个线程传入需要传走的文件民;
//            pthread_create(&tid, &attr, ftptostorage, (void *)filenamewithoutpath);   //该参数传给线程函数的参数
//            //线程调用ftp脚本;
//            lasttime=endtime;
//            //求文件命名时间（用当前包的分钟数补全成年月日时间换算成秒＋时间片＊60得秒数后再换算为截止时间）
//            q=mktime(p);
//            q+=result.timeslice*60;
//            p=localtime(&q);
//            sprintf(filetowrite,"%s%s_%d%02d%02d%02d%02d%s",result.filepath,result.hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
//            //文件名生成规则，年月日时分，分特指该包截止的那个整时间片分钟
//            dumpfp=pcap_dump_open(handle,filetowrite);
//            pcap_dump((u_char *)dumpfp,header,packet);
//    
//        }
//
//        else
//        {
//            pcap_dump((u_char *)dumpfp,header,packet);
//        }   
//
//        
//    }
//}  

   
//void *mainloop(void* arg)
//{
//    pcap_loop(handle, -1, loop_callback, NULL);
//}

void loop_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    
    //printf("the netcard thread are catching the packet\n");
    int current_device_index = *(int *)args;
    

    task_node *tmp_head=arrayofdevice[current_device_index]->head;
    while(tmp_head != NULL)
    {
        pthread_mutex_lock(tmp_head->ring_buf->f_lock);
        __ring_buffer_put(tmp_head->ring_buf,(void *)(&header->ts),sizeof(struct timeval));
        __ring_buffer_put(tmp_head->ring_buf,(void *)(&header->caplen),sizeof(bpf_u_int32));
        __ring_buffer_put(tmp_head->ring_buf,(void *)(&header->len),sizeof(bpf_u_int32));
        __ring_buffer_put(tmp_head->ring_buf,(void *)packet,header->caplen);
        pthread_mutex_unlock(tmp_head->ring_buf->f_lock);

        tmp_head=tmp_head->next;
    }
}


void *task_func(void *args)
{
    task_node *current_task=(task_node *)args;
    bool firstpacket = true;
    pcap_dumper_t *dumpfp;
    char filetowrite[filenamelen]; 
    int lasttime=0;
    while(1)
    {
        pthread_mutex_lock(current_task->ring_buf->f_lock);

        if((current_task->ring_buf->in - current_task->ring_buf->out) <= 0)
        {
            pthread_mutex_unlock(current_task->ring_buf->f_lock);
            continue;
        }
        struct pcap_pkthdr *header=(struct pcap_pkthdr *)malloc(sizeof(struct pcap_pkthdr));
        __ring_buffer_get(current_task->ring_buf, (void *)&header->ts,sizeof(struct timeval));
        __ring_buffer_get(current_task->ring_buf,(void *)&header->caplen,sizeof(bpf_u_int32));
        __ring_buffer_get(current_task->ring_buf,(void *)&header->len,sizeof(bpf_u_int32));

        u_char *current_packet=(u_char *)malloc(sizeof(u_char)*(header->caplen));
        __ring_buffer_get(current_task->ring_buf,(void *)current_packet,sizeof(u_char)*(header->caplen));
        pthread_mutex_unlock(current_task->ring_buf->f_lock);
        
        const struct packet_ethernet *ethernet;  /* The ethernet header [1] */
        const struct packet_ip *ip;              /* The IP header */
        const struct packet_tcp *tcp;            /* The TCP header */
        

        struct tm *p;     //两个用来指向暂存换算秒数的指针
        time_t q;

        p=localtime(&(header->ts.tv_sec));     //结构体p中保存了时间的每个分量 
        int currentpacketmin=p->tm_min;
        int endtime=((currentpacketmin)/(current_task->timeslice_mins)+1)*(current_task->timeslice_mins)%60;

        if(current_task->firstpacket)
        {
            //求当前分钟数之前最近的那个起始分钟数
            int nearstarttime=(currentpacketmin)/current_task->timeslice_mins*current_task->timeslice_mins;
            //求文件命名时间（用上面得到的分钟数补全成年月日时间换算成秒＋时间片＊60得秒数后再换算为截止命名时间）
            //使用函数time_t mktime(struct tm *tm); //将struct tm 结构的时间转换为从1970年至今的秒数 
            p->tm_min=nearstarttime;
            q=mktime(p);
            q+=current_task->timeslice_mins*60;
            p=localtime(&q);
            
            lasttime=endtime; //这个endtime用来控制文件的切换
            sprintf(filetowrite,"%s%s_%d%02d%02d%02d%02d%s",current_task->type,current_task->hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
            //调试的时候先用type变量做为文件的暂存路径
            dumpfp=pcap_dump_open((handle[current_task->deviceindex_of_this_task]),filetowrite);
            if(dumpfp == NULL)
            {
                printf("Erro on opening outputfile\n");
                SFDelete(header);
                SFDelete(current_packet);
                return NULL;
            }
    
            if(current_packet!=NULL)
                pcap_dump((u_char *)dumpfp,header,current_packet);   //open用文件地址做参数，dump用打开文件句柄做参数
            current_task->firstpacket=false;
            SFDelete(header);
            SFDelete(current_packet);
            continue;

        }
        if ((currentpacketmin % current_task->timeslice_mins ==0  && endtime!=lasttime) || endtime-lasttime >=current_task->timeslice_mins)   //用endtime和lasttime来控制模时间片为0的那一分钟不会不断打开新文件
        {
            
            pcap_dump_close(dumpfp);  //更换写入文件后开始执行ftp至远端的操作
            //设置线程绑定模式为非分离模式;
            //pthread_attr_t attr; 
            //pthread_t tid;
            //pthread_attr_init(&attr); 
            //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            //char filenamewithoutpath[50];
            //sprintf(filetowrite,"%s%s_%d%02d%02d%02d%02d%s",current_task->type,current_task->hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
            ////由于ftp脚本的规则，只取文件名
            ////create一个线程传入需要传走的文件名;
            //pthread_create(&tid, &attr, ftptostorage, (void *)filenamewithoutpath);   //该参数传给线程函数的参数
            //线程调用ftp脚本;
            lasttime=endtime;
            //求文件命名时间（用当前包的分钟数补全成年月日时间换算成秒＋时间片＊60得秒数后再换算为截止时间）
            q=mktime(p);
            q+=current_task->timeslice_mins*60;
            p=localtime(&q);
            sprintf(filetowrite,"%s%s_%d%02d%02d%02d%02d%s",current_task->type,current_task->hostname,1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,filetype);
            //文件名生成规则，年月日时分，分特指该包截止的那个整时间片分钟
            dumpfp=pcap_dump_open(handle[current_task->deviceindex_of_this_task],filetowrite);
            pcap_dump((u_char *)dumpfp,header,current_packet);
    
        }
        else
            pcap_dump((u_char *)dumpfp,header,current_packet);

        SFDelete(header);
        SFDelete(current_packet);
    }
}

void *netcardfunc(void *args)
{
    int current_device_index=*(int *)args;
    char errbuf[PCAP_ERRBUF_SIZE]; /* 存储错误信息的字符串 */
    bpf_u_int32 mask; /* 所在网络的掩码 */
    bpf_u_int32 net; /* 主机的IP地址 */
    int dev_is_correct=pcap_lookupnet(arrayofdevice[current_device_index]->devicename, &net, &mask, errbuf); /* 探查设备属性 */
    if (dev_is_correct==-1)
    {
        printf("%s\n",errbuf);
        exit(0);
    }
    handle[current_device_index]=pcap_open_live(arrayofdevice[current_device_index]->devicename, 65536, 1, 0, errbuf); /* 以混杂模式打开会话 */
    //网卡句柄已经打开，之后开始执行网卡线程
    task_node *tmp_head=arrayofdevice[current_device_index]->head;
    while(tmp_head != NULL) //创建各自的task线程去执行各自的loopback
    {
        tmp_head->tasknode_attr=(pthread_attr_t *)malloc(sizeof(pthread_attr_t));
        pthread_attr_init(tmp_head->tasknode_attr);
        pthread_attr_setdetachstate(tmp_head->tasknode_attr,PTHREAD_CREATE_DETACHED);
        
        //循环创建缓冲队列
        void * buffer= NULL;
        uint32_t size= 0;
        pthread_mutex_t *f_lock= (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        if (pthread_mutex_init(f_lock, NULL) != 0)
        {
            fprintf(stderr, "Failed init mutex,errno:%u,reason:%s\n",errno, strerror(errno));
            return NULL;
        }
        buffer= (void *)malloc(BUFFER_SIZE);
        if (!buffer)
        {
            fprintf(stderr, "Failed to malloc memory.\n");
            return NULL;
        }
        size = BUFFER_SIZE;
        tmp_head->ring_buf= ring_buffer_init(buffer, size, f_lock);
        if (!tmp_head->ring_buf)
        {
            fprintf(stderr, "Failed to init header ring buffer.\n");
            return NULL;
        }
        tmp_head->firstpacket=true;
        tmp_head->deviceindex_of_this_task=current_device_index;
        tmp_head->tasknode_threadID=(pthread_t *)malloc(sizeof(pthread_t));
        pthread_create(tmp_head->tasknode_threadID,tmp_head->tasknode_attr,task_func,(void *)tmp_head); 
        tmp_head=tmp_head->next;
 
    }

    //创建完抓包线程后开始执行pcap_loop(),pcap_loop()
    pcap_loop(handle[current_device_index], -1, loop_callback, (u_char *)&current_device_index);

}


int main(int argc, char *argv[]) {
    if(argc==1 || (argc>1 && strcmp(argv[1], FIFO)!=0))
    {
        printversion();
        exit(0);
    }

    initSystem();
    initSignal();
    char temp[50]={0}; //用来保存配置文件的位置
    sprintf(temp, "%s/cin.cemc/etc/config.hssp", getenv("HOME"));
    
    readconfigfile(temp);   
    pthread_t netcardthreads[count];   //网卡线程族
    //netcardthreads=(pthread_t *)malloc(sizeof(pthread_t *)*(count));
    pthread_attr_t netcardattr[count];
    //netcardattr=(pthread_attr_t *)malloc(sizeof(pthread_attr_t *)*(count));
    int device_index; 
    int deviceID[count];
    for(device_index=0;device_index<count;device_index++)   //循环创建网卡级线程,count为网卡个数，i要是设计成全局变量， 此处可能要用到互斥锁+条件互斥量防止循环速度快于回调函数
    {
        pthread_attr_init(&(netcardattr[device_index]));
        pthread_attr_setdetachstate(&(netcardattr[device_index]),PTHREAD_CREATE_DETACHED);
        deviceID[device_index] = device_index;  //传给线程的参数不会因为取到同一块地址而出现变化
        pthread_create(&(netcardthreads[device_index]),&(netcardattr[device_index]),netcardfunc,(void *)&deviceID[device_index]); 
        //netcardfunc即网卡需要去执行的函数，传入网卡链表
    }
  // char *dev; /* 执行嗅探的设备 */
  // char errbuf[PCAP_ERRBUF_SIZE]; /* 存储错误信息的字符串 */
  // struct bpf_program filter; /* 已经编译好的过滤器 */
  // bpf_u_int32 mask; /* 所在网络的掩码 */
  // bpf_u_int32 net; /* 主机的IP地址 */
  // struct pcap_pkthdr header; /* 由pcap.h定义 */
  // 
  // const u_char *packet; /* 实际的包 */
  // /* Define the device */
  // /* dev = pcap_lookupdev(errbuf); */
  // dev = result.device; /* 网卡名称 */
  // int deviscorrect=pcap_lookupnet(dev, &net, &mask, errbuf); /* 探查设备属性 */
  // if (deviscorrect==-1)
  // {
  //     printf("%s\n",errbuf);
  //     exit(0);
  // }
  // handle = pcap_open_live(dev, 65536, 1, 0, errbuf); /* 以混杂模式打开会话 */
  // int filtRulesiscorrect=pcap_compile(handle, &filter, result.filterrules, 0, net); /* 编译并应用过滤器 */
  // if (filtRulesiscorrect==-1)
  // {
  //     printf("%s\n", "grammer is not standardized,compile error");
  //     exit(-1);
  // }
//
  //  pcap_setfilter(handle, &filter);
  //  pthread_attr_t attr; 
  //  pthread_t tid;
  //  pthread_attr_init(&attr); 
  //  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  //  pthread_create(&tid, &attr, mainloop, (void *)handle);  
//
   while(1)
   {
       if(step_flag)
       {
           step();
       }
       
       usleep(10);
   }

    return(0);

}



