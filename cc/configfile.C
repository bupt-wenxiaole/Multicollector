#include "configfile.h"
#include "setting_datastructure.h"

extern devicelist* arrayofdevice[];   //全局数组
int count;

int readconfigfile(const char *filepath)
{
    config_t cfg;  //配置文件整体数据结构
    config_setting_t *configsetting,*devicesetting,*tasksetting;   //代码条理性高于小的内存占用,分配三个层级的结构体指针使结构清晰
    const char *str;
    config_init(&cfg);

    char bufferpath[bufferpath_len];
     /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, filepath))
    {
      fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
      config_error_line(&cfg), config_error_text(&cfg));
      config_destroy(&cfg);
      return(EXIT_FAILURE);
    }
    configsetting = config_lookup(&cfg, "configfile");
    if(configsetting!=NULL)
    {
      count = config_setting_length(configsetting);
      //printf("%d\n", count);
    }
    int i;
    
    for(i=0;i<count;++i)
    {
      
      sprintf(bufferpath,"configfile.device%d",i);           //这里定位到的是device0
      //printf("%s\n",bufferpath);
      
      devicesetting=config_lookup(&cfg,bufferpath);
      int countofdevice=config_setting_length(devicesetting);
      //printf("%d\n",countofdevice);
    

      if(devicesetting!=NULL)        //这里的setting是device对应的setting
      {

          arrayofdevice[i]=(devicelist *)malloc(sizeof(devicelist));

          char tmppath[bufferpath_len];                 //这里定位到的是网卡的位置，也就是第一个位置，之后for循环开始读各task
          sprintf(tmppath,"%s.[0]",bufferpath);

          config_lookup_string(&cfg,tmppath,&str);     //注意看conf_lookup_string和config_setting_lookup_string是不同的
          //printf("networkcard name: %s\n\n", str);
          arrayofdevice[i]->devicename=(char *)str;
          //printf("networkcard name: %s\n\n",arrayofdevice[i]->devicename);

          
          sprintf(tmppath,"%s.[1]",bufferpath);
          tasksetting=config_lookup(&cfg,tmppath);
          const char *type, *filterRules,*hostname,*fixtime_starttime;
          int timeslice_mins,fixtime_duration_mins;

          config_setting_lookup_string(tasksetting, "type", &type);

          config_setting_lookup_string(tasksetting, "filterRules", &filterRules);
  
          config_setting_lookup_string(tasksetting, "hostname", &hostname);
       
          config_setting_lookup_int(tasksetting, "timeslice_mins", &timeslice_mins);
       
          config_setting_lookup_string(tasksetting,"fixtime_starttime",&fixtime_starttime);
        
          config_setting_lookup_int(tasksetting,"fixtime_duration_mins",&fixtime_duration_mins);


          arrayofdevice[i]->head=alloc_node((char *)type,(char *)filterRules,(char *)hostname,timeslice_mins,(char *)fixtime_starttime,fixtime_duration_mins);
          //print_list(arrayofdevice[i]->head);



          int j;

          for(j=2;j<countofdevice;j++)
          {

            sprintf(tmppath,"%s.[%d]",bufferpath,j);
            tasksetting=config_lookup(&cfg,tmppath);
            const char *type, *filterRules,*hostname,*fixtime_starttime;
            int timeslice_mins,fixtime_duration_mins;

            config_setting_lookup_string(tasksetting, "type", &type);
         
            config_setting_lookup_string(tasksetting, "filterRules", &filterRules);
    
            config_setting_lookup_string(tasksetting, "hostname", &hostname);
         
            config_setting_lookup_int(tasksetting, "timeslice_mins", &timeslice_mins);
     
            config_setting_lookup_string(tasksetting,"fixtime_starttime",&fixtime_starttime);
       
            config_setting_lookup_int(tasksetting,"fixtime_duration_mins",&fixtime_duration_mins);

            push(arrayofdevice[i]->head,(char *)type,(char *)filterRules,(char *)hostname,timeslice_mins,(char *)fixtime_starttime,fixtime_duration_mins);

            //printf("\n\n");

          }
          //print_list(arrayofdevice[i]->head);
      }
      
    }
    //config_destroy(&cfg);
  return 0;

}

/*
int main(int argc, char const *argv[])
{
  const char *path="hssp.conf";
  readconfigfile(path);
  return 0;
}
*/
