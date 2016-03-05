#ifndef KFIFO_HEADER_H 
#define KFIFO_HEADER_H

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

//判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))
//取a和b中最小值
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define BUFFER_SIZE 1024*1024

struct ring_buffer
{
    void         *buffer;     //缓冲区
    uint32_t     size;       //大小
    uint32_t     in;         //入口位置
    uint32_t       out;        //出口位置
    pthread_mutex_t *f_lock;    //互斥锁
};

#ifdef __cplusplus
extern "C"
{
#endif
struct ring_buffer* ring_buffer_init(void *buffer, uint32_t size, pthread_mutex_t *f_lock);
void ring_buffer_free(struct ring_buffer *ring_buf);
uint32_t __ring_buffer_len(const struct ring_buffer *ring_buf);
uint32_t __ring_buffer_get(struct ring_buffer *ring_buf, void * buffer, uint32_t size);
uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
uint32_t ring_buffer_len(const struct ring_buffer *ring_buf);

#ifdef __cplusplus
}
#endif

#endif

