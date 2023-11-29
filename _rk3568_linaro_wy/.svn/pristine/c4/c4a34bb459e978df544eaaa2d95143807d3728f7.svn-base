/*
 * File      : ringbuffer.c
 * This file is customized ringbuffer for multi thread interface implementaion file 
 * COPYRIGHT (C) 2019 zmvision
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-24     fengyong    first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "ringbuffer.h"
#include "xutils.h"

static inline int is_power_of_2(uint32_t n) 
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

static uint32_t roundup_power_of_two(uint32_t val)
{
    if((val & (val-1)) == 0) {
        return val;
    }	
 
    uint32_t maxulong = (uint32_t)((uint32_t)~0);
    uint32_t andv = ~(maxulong & (maxulong >> 1));
	
    while((andv & val) == 0) {
        andv = andv>>1;
    }
	
    return (andv << 1);
}

void ringbuffer_reset(ring_buffer_t *rb)
{
	rb->in = 0;
	rb->out = 0;
}

inline uint32_t ringbuffer_data_len(ring_buffer_t *rb)
{
	return (rb->in - rb->out);
}

inline int ringbuffer_is_full(ring_buffer_t *rb)
{
	return (rb->size == (rb->in - rb->out));
}

inline int ringbuffer_is_empty(ring_buffer_t *rb)
{
	return (rb->in == rb->out);
}

uint32_t ringbuffer_put(ring_buffer_t *rb, const uint8_t *ptr, uint32_t len)
{
	uint32_t l;
 
    len = min(len, rb->size - rb->in + rb->out);
 
    /*
     * Ensure that we sample the out index -before- we
     * start putting bytes into the ringbuffer.
     */
    __sync_synchronize();
 
    /* first put the data starting from fifo->in to buffer end */
    l = min(len, rb->size - (rb->in  & (rb->size - 1)));
    memcpy(rb->buffer + (rb->in & (rb->size - 1)), ptr, l);
 
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(rb->buffer, ptr + l, len - l);
 
    /*
     * Ensure that we add the bytes to the kfifo -before-
     * we update the fifo->in index.
     */
    __sync_synchronize();
 
    rb->in += len;
 
    return len;
}

uint32_t ringbuffer_get(ring_buffer_t *rb, uint8_t *ptr, uint32_t len)
{
	uint32_t l;
 
    len = min(len, rb->in - rb->out);
 
    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */
    __sync_synchronize();
     
    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, rb->size - (rb->out & (rb->size - 1)));
    memcpy(ptr, rb->buffer + (rb->out & (rb->size - 1)), l);
 
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(ptr + l, rb->buffer, len - l);
 
    /*
     * Ensure that we remove the bytes from the kfifo -before-
     * we update the fifo->out index.
     */
    __sync_synchronize();
 		
    rb->out += len;
 
    return len;
}

ring_buffer_t* ringbuffer_create(uint32_t size)
{
	ring_buffer_t *rb = NULL;
    uint8_t *buf = NULL;
		
	assert(size > 0);

	rb = (ring_buffer_t *)malloc(sizeof(*rb));
    if (rb == NULL) {
        goto exit;
    }	

	memset(rb, 0, sizeof(*rb));

	rb->size = size;
	
	/* round up to the next power of 2 */
    if (!is_power_of_2(size)) {
        rb->size = roundup_power_of_two(size);
    }

	buf = (uint8_t *)malloc(rb->size);
	if (NULL == buf) {
		free(rb);
		rb = NULL;
		goto exit;
	}
		
	rb->buffer = buf;
	rb->in = 0;
	rb->out = 0;
	
exit:
	return rb;
}