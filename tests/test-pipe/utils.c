/**
 * @file utils.c
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
 * URL: $HeadURL: https://svn.petrovitsch.priv.at/ICSS-BES/trunk/2015/src/mypopen/utils.c $
 *
 * Last Modified: $Author: bernd $
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

/*
 * --------------------------------------------------------------- defines --
 */

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

const int std_fds_allowed_open[] =
        {
                STDIN_FILENO,
                STDOUT_FILENO,
                STDERR_FILENO
        };

const size_t num_std_fds_allowed_open = ARRAY_SIZE(std_fds_allowed_open);

static int verbose = -1;

/*
 * ------------------------------------------------------------- functions --
 */

void setVerbose(
        int value
)
{
    verbose = value;
}

int getVerbose(
        void
)
{
    return verbose;
}

void trace(
        const int level,
        const char *fmt,
        ...
)
{
    if (verbose >= level)
    {
        va_list ap;
        va_start(ap, fmt);
        if (vprintf(fmt, ap) < 0)
        {
            bailout("Cannot write to stdout");
        }
        va_end(ap);
    }

    if (fflush(stdout) == EOF)
    {
        bailout("Cannot flush stdout");
    }

}


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
void bailout(
        const char *fmt,
        ...
)
{
    va_list ap;

    va_start(ap, fmt);

    if (errno != 0)
    {
        (void) fprintf(
                stderr,
                "%s: %s - ",
                cmd,
                strerror(errno)
        );
    }
    else
    {
        (void) fprintf(
                stderr,
                "%s: ",
                cmd
        );
    }

    (void) vfprintf(
            stderr,
            fmt,
            ap
    );

    (void) fputc('\n', stderr);

    freeresources();

    exit(EXIT_FAILURE);
}

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
int verify_closed_file_desc(
        const int *fds_allowed_open,
        size_t num_fds
)
{
    int i;
    struct stat statbuf;

    for (i = 0; i < FD_SETSIZE; ++i)
    {
        int allowed_open = 0;
        int j;

        for (j = 0; j < (int) num_fds; ++j)
        {
            if (i == fds_allowed_open[j])
            {
                allowed_open = 1;
                break;
            }
        }

        errno = 0;

        if (!allowed_open)
        {
            TRACE2("Checking if file descriptor %d is open ...\n", i);

            if ((fstat(i, &statbuf) == 0) || (errno != EBADF))
            {

                TRACE("File descriptor %d is open but should be closed\n", i);

                return 0;
            }
        }

        errno = 0;
    }

    return 1;
}

/*
 * =================================================================== eof ==
 */

/*
 * Local Variables:
 * c-basic-offset: 4
 * End:
 */
