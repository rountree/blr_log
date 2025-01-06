/* blr_log.h
 *
 * A logging almost-library for a very specific set of constraints.
 *
 * Use blr_preamble( ... ) and blr_postamble( ... ) at entry and exit points
 * of functions, respectively, to get stack-depth-based indenting.
 *
 * Not at all thread safe.
 *
 * Definitely heavyweight.
 *
 * Spindle in its current incarnation occasionally shuts down all open file
 * descriptors.  We get around this by opening and closing each log file as
 * needed.
 *
 * Spindle has a complex build architecture and an existing logging infrastructure.
 * Adding a new set of files for a competing logger would be both difficult and
 * impolitic.  That said, Spindle's existing logger has several shortcoming with
 * regard to coverage and funciton naming that I've eliminated in this design.
 *
 * Spindle uses deep magic to manipulate libraries.  I want to avoid any entanglements
 * there, so this logger is contained in a single header file that can be injected
 * via CFLAGS (and similar) compile-time environment variables.  That constraint
 * leads to storing state in the environment.
 */

#ifndef __BLR_LOG_H
#define __BLR_LOG_H
#ifdef BLR_LOG              // Use -DBLR_LOG to enable

// If <string.h> has already been included, we don't know if
// _GNU_SOURCE had been defined prior to its include.  So die.
// This test is fragile, but it'll do for now.
#ifdef _STRING_H
#error Due to GNU basename(3) shenanigans, <blr_log.h> must be #included before <string.h> or any other header that pulls in <string.h>.
#endif

// I don't every recall seeing anyone use <libgen.h>, but just in case....
// Again, very fragile, but will do for now.
#ifdef _LIBGEN_H
#error <blr_log.h> depends on the GNU version of basename(3), but <libgen.h> is already included and defines a conflicting version.
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE         // get safe version of basename(3)
#endif
#include <string.h>         // basename(3)
#include <sys/types.h>      // open(2)
#include <sys/stat.h>       // open(2), getpid(2), getppid(2)
#include <fcntl.h>          // open(2)
#include <stdio.h>          // snprintf(3), dprintf(3)
#include <unistd.h>         // getpid(2), getppid(2)
#include <sys/time.h>       // gettimeofday(2)
#include <stdlib.h>         // getenv(3), setenv(3), atoi(3)

#define blr_xstr(s) blr_str(s)
#define blr_str(s) #s

static int blr_stack_depth;                 // file scope so other log calls can use it.
                                            // Minimum value is 1.
static const int blr_max_indent_depth=15;   // Depends on how wide your screen is, I suppose.
static char blr_new_depth_str[ 22 ];

#define _blr_log( suffix, code, fmt, ... )\
    do{ \
        static struct timeval blr_tv; \
        gettimeofday( &blr_tv, NULL ); \
        static double blr_now; \
        blr_now = \
            blr_tv.tv_sec + blr_tv.tv_usec/1000000.0; \
        static const char path[] = \
            "/p/vast1/rountree/tuolumne/blr_logs"; \
        static char filename[128]; \
        snprintf( filename, 127, "%s/%i." suffix, \
                path, getpid() ); \
        static int blr_fd; \
        blr_fd = open( filename, \
                O_WRONLY | O_CREAT | O_APPEND, \
                S_IRUSR | S_IWUSR ); \
        if( -1 != blr_fd ){ \
            dprintf( blr_fd, \
                    "%lf %i %i %-30s %4d %*s %s (%d): " fmt "\n", \
                    blr_now, \
                    getpid(), getppid(), \
                    basename(__FILE__), __LINE__, \
                    blr_stack_depth < blr_max_indent_depth ? blr_stack_depth : blr_max_indent_depth, code,\
                    __func__, blr_stack_depth, \
                    ##  __VA_ARGS__ ); \
            close( blr_fd ); \
        } \
    }while(0)

#define blr_log( fmt, ... )   _blr_log( "log", ".", fmt, ## __VA_ARGS__ )
#define blr_err( fmt, ... )   _blr_log( "err", "!", fmt, ## __VA_ARGS__ )

#define blr_preamble( fmt, ... )\
    do{ \
        char *depth_str = getenv("BLR_STACK_DEPTH"); \
        if( NULL == depth_str ){ \
            blr_stack_depth = 0; \
        }else{ \
            blr_stack_depth = atoi( depth_str ); \
        } \
        blr_stack_depth++; \
        snprintf( blr_new_depth_str, 21, "%i", blr_stack_depth ); \
        setenv( "BLR_STACK_DEPTH", blr_new_depth_str, 1 ); \
        _blr_log( "preamble", ">", fmt, ## __VA_ARGS__ ); \
    }while(0);


#define blr_postamble( fmt, ... )\
    do{ \
        _blr_log( "postamble", "<", fmt, ## __VA_ARGS__ ); \
        blr_stack_depth--; \
        snprintf( blr_new_depth_str, 21, "%i", blr_stack_depth ); \
        setenv( "BLR_STACK_DEPTH", blr_new_depth_str, 1 ); \
    }while(0);
#else  // BLR_LOG
#define blr_log( fmt, ... )
#define blr_err( fmt, ... )
#define blr_preamble( fmt, ... )
#define blr_postamble( fmt, ... )
#endif // BLR_LOG


#endif // __BLR_LOG_H

