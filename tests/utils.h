/**
 * @file utils.h
 * Betriebsysteme popen Modul.
 * Beispiel 1.2
 *
 * @author Thomas M. Galla <galla@technikum-wien.at>
 * @date 2006/03/18
 *
 * @version $Revision: 1354 $
 *
 * @todo Nothing to do. - Everything perfect! ;-)
 *
 * URL: $HeadURL: https://svn.petrovitsch.priv.at/ICSS-BES/trunk/2015/src/mypopen/utils.h $
 *
 * Last Modified: $Author: bernd $
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * --------------------------------------------------------------- defines --
 */

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define VERIFY_CLOSED_NON_STD_FDS() \
    verify_closed_file_desc(std_fds_allowed_open, num_std_fds_allowed_open)

#define TRACE0(fmt,...)                                 \
    trace(                                              \
        0,                                              \
        "%s [%s, %s(), line %d]: " fmt,                 \
        cmd,                                            \
        __FILE__,                                       \
        __func__,                                       \
        __LINE__,                                       \
        ##__VA_ARGS__                                   \
        )
#define TRACE(fmt,...)                                  \
    trace(                                              \
        1,                                              \
        "%s [%s, %s(), line %d]: " fmt,                 \
        cmd,                                            \
        __FILE__,                                       \
        __func__,                                       \
        __LINE__,                                       \
        ##__VA_ARGS__                                   \
        )
#define TRACE2(fmt,...)                                 \
    trace(                                              \
        2,                                              \
        "%s [%s, %s(), line %d]: " fmt,                 \
        cmd,                                            \
        __FILE__,                                       \
        __func__,                                       \
        __LINE__,                                       \
        ##__VA_ARGS__                                   \
        )


#define ENTER() TRACE("entering ...\n")

#define EXIT() TRACE("exiting ...\n")

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

extern const int std_fds_allowed_open[];

extern const size_t num_std_fds_allowed_open;

/*
 * ------------------------------------------------------------- externals --
 */

extern const char *cmd;

extern void freeresources(
        void
);

/*
 * ------------------------------------------------------------- functions --
 */

extern void setVerbose(
        int value
);

extern int getVerbose(
        void
);

extern void trace(
        const int level,
        const char *fmt,
        ...
) __attribute__((format(printf,2,3)));
/**
 * \brief terminate program with error message
 *
 * This function deallocates all resources by calling \a freeresources(),
 * prints the (printf(3) like) message \a fmt and terminates the program with exits status
 * EXIT_FAILURE
 *
 * \param fmt printf(3) like format string
 *
 */
extern void bailout(
        const char *fmt,
        ...
) __attribute__((format(printf,1,2)));

/**
 * verify that all file descriptors except those provided as arguments are
 * closed.
 *
 * note: FD_SETSIZE (see select()) is used as maximum number of
 * open file descriptors even the system may support an higher number
 * of open files.
 *
 * \param fds_allowed_open array of file descriptors that are allowed to be open
 * \param num_fds number of elements in the \a fds_allowed_open array
 */
extern int verify_closed_file_desc(
        const int *fds_allowed_open,
        size_t num_fds
);

#endif /* _UTILS_H_ */

/*
 * =================================================================== eof ==
 */
