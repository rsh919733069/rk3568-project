/*
 * File      : logger.c
 * This file is file logger   
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-02    fengyong    first version 
 */
 
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdint.h>
#include <semaphore.h>
#include "log/log.h"

#include "logger.h"

#define L_SLOT (swap_count & 0x1)
#define W_SLOT ((swap_count - 1) & 0x1)

static uint32_t swap_count = 0;

enum logger_info_e {
	LOGGER_SWAP_MAX = 2,			
    LOGGER_SWAP_FILE_PATH_SIZE = 64,
};

typedef struct {
    int fd_logger[LOGGER_SWAP_MAX];
	char logger_file[LOGGER_SWAP_MAX][LOGGER_SWAP_FILE_PATH_SIZE];
    uint32_t line_count;  
}logger_info_t;

static logger_info_t *gs_logger_info = NULL;

static void logger_create_logger_by_count()
{
    logger_info_t *logger_info = gs_logger_info;

	sprintf(logger_info->logger_file[L_SLOT], "/tmp/log_%04u.txt", swap_count);

	logger_info->fd_logger[L_SLOT] = open(logger_info->logger_file[L_SLOT], O_WRONLY | O_TRUNC | O_CREAT, 0644);
    SLOGI("logger_info->fd_logger[L_SLOT] : %d", logger_info->fd_logger[L_SLOT]);
}

static void logger_evolute()
{
    logger_info_t *logger_info = gs_logger_info;

	if ((++logger_info->line_count & 0x1fff) == 0x1fff) {
		close(logger_info->fd_logger[L_SLOT]);
        remove(logger_info->logger_file[L_SLOT]);
		logger_create_logger_by_count(); 
        ++swap_count;
	}
}

int logger_deinit(void) 
{
    logger_info_t *logger_info = gs_logger_info;
    if (gs_logger_info) {
        free(gs_logger_info);
    }
    if(logger_info->fd_logger[L_SLOT]){
        close(logger_info->fd_logger[L_SLOT]);
    }
    if(logger_info->fd_logger[W_SLOT]){
        close(logger_info->fd_logger[W_SLOT]);
    }
}
int logger_init(void) 
{
    int ret = 0;

    gs_logger_info = (void *)malloc(sizeof(logger_info_t));

    if (gs_logger_info) {
        memset((void *)gs_logger_info, 0, sizeof(logger_info_t));

        logger_create_logger_by_count();
        ++swap_count;
	
        logger_create_logger_by_count();    
        ++swap_count;
    }
    else  {
        printf("gs_logger_info malloc failed\n");
        return 1;
    }
    
    return ret;
}

void logger_info(const char *tag, const char *format, ...)
{	
    logger_info_t *logger_info = gs_logger_info;
	va_list va;
	va_start(va, format);
	
	dprintf(logger_info->fd_logger[W_SLOT], "[%s]:", tag);
	vdprintf(logger_info->fd_logger[W_SLOT], format, va);
	dprintf(logger_info->fd_logger[W_SLOT], "\n");

	logger_evolute();

	va_end(va);
}
