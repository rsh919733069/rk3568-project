/*
 * File      : ringbuffer.h
 * This file is customized ringbuffer for multi thread header 
 * COPYRIGHT (C) 2019 zmvision
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-24     fengyong    first version
 */

#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include "types_def.h"

typedef struct {
	uint8_t *buffer;	
	uint32_t size;
	uint32_t in;
	uint32_t out;
}ring_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif
ring_buffer_t* ringbuffer_create(uint32_t size);
void ringbuffer_reset(ring_buffer_t *rb);
uint32_t ringbuffer_put(ring_buffer_t *rb, const uint8_t *ptr, uint32_t len);
uint32_t ringbuffer_get(ring_buffer_t *rb, uint8_t *ptr, uint32_t len);
int ringbuffer_is_full(ring_buffer_t *rb);
int ringbuffer_is_empty(ring_buffer_t *rb);
uint32_t ringbuffer_data_len(ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* _RINGBUFFER_H_ */
