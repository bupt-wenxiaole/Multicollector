/*
file:	conv.hpp
author:	justing
time:	2015-10-29
func:	conveger process
*/

#ifndef	_CONV_HPP
#define _CONV_HPP

#include <vector>
using std::vector;
    
#define MAX_BUFFER  1024
#define SUFFIX      "pcap"

class TConvNode
{
    public:
        TConvNode();
        ~TConvNode();
        
        int     id;
        int     state;
        char    name[64];
        
        int check(char* path, char* time);
};

typedef TConvNode* PTConvNode;
typedef vector<PTConvNode>  ListConv;

class TConvManager
{
    public:
        TConvManager();
        ~TConvManager();
        
        char*   pushpath;
        char*   temppath;
        char*   stordir;
        int     base;
        int     interval;
        int     delay;
        
        void    init();
        void    loop();
        void    converge(struct tm timer);
        
    private:
        ListConv   data;
};


#endif

