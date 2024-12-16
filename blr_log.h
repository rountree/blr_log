#ifndef __BLR_LOG_H
#define __BLR_LOG_H
#ifdef BLR_LOG      // Use -DBLR_LOG to enable
#define _GNU_SOURCE         // get safe version of basename(2)
#include <string.h>         // basename(2)
#include <sys/types.h>      // open(2)
#include <sys/stat.h>       // open(2), getpid(2), getppid(2)
#include <fcntl.h>          // open(2)
#include <stdio.h>          // snprintf(3), dprintf(3)
#include <unistd.h>         // getpid(2), getppid(2)
#include <sys/time.h>       // gettimeofday(2)

#define blr_log( fmt, ... )\
    do{ \
        static struct timeval blr_tv; \
        gettimeofday( &blr_tv, NULL ); \
        static double blr_now; \
        blr_now = \
            blr_tv.tv_sec + blr_tv.tv_usec/1000000.0; \
        static const char path[] = \
            "/p/vast1/rountree/tuolumne/traces"; \
        static char filename[128]; \
        snprintf( filename, 127, "%s/%i.blr", \
                path, getpid() ); \
        static int blr_fd; \
        blr_fd = open( filename, \
                O_WRONLY | O_CREAT | O_APPEND, \
                S_IRUSR | S_IWUSR ); \
        if( -1 != blr_fd ){ \
            dprintf( blr_fd, \
                    "%lf %i %i %15s %4d %-25s : " fmt "\n", \
                    blr_now, \
                    getpid(), getppid(), \
                    basename(__FILE__), __LINE__, __func__, \
                    ##  __VA_ARGS__ ); \
            close( blr_fd ); \
        } \
    }while(0)
#else
#define blr_log( fmt, ... )
#endif // BLR_LOG

#endif // __BLR_LOG_H
