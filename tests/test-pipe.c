/**
 * @file test-pipe.c
 * Interactive test suite for Betriebsysteme mypopen()/mypclose() module
 * Beispiel 2
 *
 * @author Thomas M. Galla <galla@technikum-wien.at>
 * @date 2006/03/18
 *
 * @version $Revision: 965 $
 *
 * URL: $HeadURL: https://svn.petrovitsch.priv.at/ICSS-BES/trunk/2014/src/mypopen/test-pipe.c $
 *
 * Last Modified: $Author: bernd $
 */

/**
 * \mainpage Interactive test suite for the Betriebsystem mypopen()/mypclose module
 *
 * This library called \c libtest-pipe.a provides an interactive test suite for the
 * \c mypopen()/mypclose() module. The library implements provides
 * a \a main() function that prompts input from the user and calls \c mypopen()/mypclose()
 * accordingly.
 *
 * The library assumes that your versions of \c popen()/\c pclose() are
 * called \c mypopen() and \c mypclose() respectively.
 *
 * The resulting executable (\c test-pipe) first propts the user for
 * the input of a shell command and afterwards for a type ("r" or "w")
 * depending on whether the \c test-pipe shall read data from the pipe or
 * whether \c test-pipe shall write data into the pipe. In case the pipe is
 * opened for reading ("r"), the data read from the pipe is written to
 * \c stdout. In case the pipe is opened for writing ("w"), data is read
 * from \c stdin and written to the pipe.  In both cases the number of characters
 * transferred via the pipe is written to \c stdout. Any errors during
 * execution are printed to \c stderr. - See below for an example output:
 *
 \verbatim
 $ ./test-pipe
 Command> ls -al
 Type (r/w)> r
 total 52
 drwxrwxrwx+   3 tom      Domain U        0 Mar 19 00:26 .
 drwxrwxrwx+   8 Administ Domain U        0 Mar 18 09:46 ..
 -rwxrwxrwx    1 tom      Domain U     1863 Mar 19 00:17 Makefile
 drwxrwxrwx+   4 tom      Domain U        0 Mar 19 00:26 doc
 -rwxrwxrwx    1 tom      Domain U     8675 Mar 19 00:25 doxygen.dcf
 -rwxrwxrwx    1 tom      Domain U     5733 Mar 18 23:46 main.c
 -rw-r--r--    1 tom      Domain U     2996 Mar 19 00:17 main.o
 -rwxrwxrwx    1 tom      Domain U     4194 Mar 18 11:26 mypopen.c
 -rwxrwxrwx    1 tom      Domain U     1888 Mar 18 10:44 mypopen.h
 -rw-r--r--    1 tom      Domain U     1632 Mar 19 00:17 mypopen.o
 -rwxr-xr-x    1 tom      Domain U    18051 Mar 19 00:17 test-pipe.exe
 -rwxrwxrwx    1 tom      Domain U     1596 Mar 19 00:00 utils.c
 -rwxrwxrwx    1 tom      Domain U     1389 Mar 19 00:00 utils.h
 -rw-r--r--    1 tom      Domain U      793 Mar 19 00:17 utils.o
 905 characters read.

 $ ./test-pipe
 Command> grep foo
 Type (r/w)> w
 Die Betriebssysteme Uebung ist total genial.
 Der Vortragende ist ein foo.
 Der Vortragende ist ein foo.
 Abbrechen mit Control D.
 99 characters written.
 \endverbatim
 *
 * To link your \c mypopen()/mypclose() module (name \c mypopen.o is
 * assumed in the command line below) with the library, you have to issue
 * the following command in for the final linking step:
 *
 \verbatim
 gcc -Wall -Wextra -Werror -pedantic -o test-pipe mypopen.o -ltest-pipe
 \endverbatim
*/


/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "utils.h"
#include "../src/mypopen.h"

/*
 * --------------------------------------------------------------- defines --
 */

#define CMDLEN 256
#define LINELEN 256
#define TYPELEN 2

/*
 * -------------------------------------------------------------- typedefs --
 */

typedef enum
{
    DIR_READ,
    DIR_WRITE,
    DIR_NONE
} flowdir_t;

/*
 * --------------------------------------------------------------- globals --
 */

const char *cmd = "<not yet set>";

static FILE *fp = NULL;

static int stdoutflushed = 0;

/*
 * ------------------------------------------------------------- functions --
 */

/**
 * \brief Free all allocated resources
 *
 * This function frees all allocated ressources by calling \a PCLOSE()
 * and afterwards flushes stdout.
 *
 */
void freeresources(
        void
)
{
    if (fp != NULL)
    {
        if (mypclose(fp) == -1)
        {
            fp = NULL;

            bailout("Cannot mypclose() pipe");
        }

        fp = NULL;
    }

    if (stdoutflushed)
    {
        if (fflush(stdout))
        {
            stdoutflushed = 1;
            bailout("Cannot flush stdout");
        }

        stdoutflushed = 1;
    }
}


/**
 * \brief Print usage messaf
 *
 * This function prints a usage message to stderr and afterwards
 * terminates the program with status EXIT_FAILURE.
 *
 */
static void usage(
        void
)
{
    (void) fprintf(
            stderr,
            "USAGE: %s\n",
            cmd
    );

    exit(EXIT_FAILURE);
}


/**
 * \brief Main function
 *
 * This function is the main entry point of the program. The function
 * prompts the user to enter a command and a type ("r" or "w") which
 * are then passed onward to \a POPEN(). - Afterwards data is read from
 * either stdin or the pipe and written to either the pipe or to stdout.
 * Once EOF is read, \a freeresources() is called.
 *
 * \param argc number of arguments passed to the program
 * \param argv vector of arguments passed to the program.
 *
 * \return exit status of program
 * \retval EXIT_FAILURE Program terminated due to a failure.
 * \retval EXIT_SUCCESS Program terminated successfully.
 *
 */
int main(
        int argc,
        char **argv
)
{
    char command[CMDLEN];
    char type[TYPELEN];
    char line[LINELEN];
    flowdir_t flowdir = DIR_NONE;
    unsigned long countchars = 0;
    FILE *input, *output;
    const char *action;

    cmd = argv[0];

    /*
     * wrong number of arguments?
     */
    if (argc != 1)
    {
        usage(); /* show usage to user */
    }

    if (printf("Command> ") < 0)
    {
        bailout("Cannot write to stdout");
    }

    if (fgets(command, sizeof(command), stdin) == NULL)
    {
        bailout("Cannot read from stdin");
    }

    do
    {
        if (printf("Type (r/w)> ") < 0)
        {
            bailout("Cannot write to stdout");
        }

        if (fgets(type, sizeof(type), stdin) == NULL)
        {
            bailout("Cannot read from stdin");
        }

        if (strcmp(type, "w") == 0)
        {
            flowdir = DIR_WRITE;
        }
        else if (strcmp(type, "r") == 0)
        {
            flowdir = DIR_READ;
        }
        else
        {
            flowdir = DIR_NONE;
        }
    } while (flowdir == DIR_NONE);

    if ((fp = mypopen(command, type)) == NULL)
    {
        bailout("Cannot mypopen()");
    }

    switch (flowdir)
    {
        case DIR_READ:
            input = fp;
            output = stdout;
            action = "read";
            break;
        case DIR_WRITE:
            input = stdin;
            output = fp;
            action = "written";
            break;
        default:
            assert(0);
    }

    while (fgets(line, sizeof(line), input) != NULL)
    {
        if (fputs(line, output) == EOF)
        {
            bailout("Cannot write to output stream");
        }

        if (fflush(output) == EOF)
        {
            bailout("Cannot flush output stream");
        }

        countchars += strlen(line);
    }

    if (ferror(fp))
    {
        bailout("Cannot read from input stream");
    }

    if (printf("%lu characters %s.\n", countchars, action) < 0)
    {
        bailout("Cannot write to stdout");
    }

    freeresources();

    exit(EXIT_SUCCESS);
}

/*
 * =================================================================== eof ==
 */

/*
 * Local Variables:
 * c-basic-offset: 4
 * End:
 */
