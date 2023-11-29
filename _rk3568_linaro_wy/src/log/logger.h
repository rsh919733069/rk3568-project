/*
 * File      : logger.h
 * This file is file logger header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-02    fengyong    first version 
 */

#ifndef _USER_LOGGER_H
#define _USER_LOGGER_H

#include <stdint.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif 

int logger_init(void);
void logger_info(const char *tag, const char *format, ...);
int logger_deinit(void);
#ifdef __cplusplus
}
#endif

#endif  /* _USER_LOGGER_H */


