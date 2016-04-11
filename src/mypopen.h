#ifndef _MYPOPEN_H_
#define _MYPOPEN_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define READ 0
#define WRITE 1

FILE *mypopen(const char *command, const char *type);
int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */
