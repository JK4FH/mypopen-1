#include "mypopen.h"

static FILE *global_stream = NULL;
static pid_t pid = -1;

FILE *mypopen(const char *command, const char *type) {
  int parent, child;
  int pipe_ends[2];

  /* check if already open */
  if (global_stream != NULL) {
    errno = EAGAIN;
    return NULL;
  }

  if (command == NULL) {
    errno = EINVAL;
    return NULL;
  }

  switch (*type) {
  case 'r':
    parent = READ;
    child = WRITE;
    break;
  case 'w':
    parent = WRITE;
    child = READ;
    break;
  default:
    errno = EINVAL;
    return NULL;
  }

  if (pipe(pipe_ends) == -1) {
    return NULL;
  }

  switch (pid = fork()) {
  /* error */
  case -1:
    close(pipe_ends[parent]);
    close(pipe_ends[child]);
    return NULL;
  /* child */
  case 0:
    close(pipe_ends[parent]);
    if (pipe_ends[child] != child) {
      if (dup2(pipe_ends[child], child) == -1) {
        close(pipe_ends[child]);
        _exit(127);
      }
      close(pipe_ends[child]);
    }
    execl("/bin/sh", "sh", "-c", command, NULL);
    /* reached only if execl failed */
    _exit(127);
  /* parent */
  default:
    close(pipe_ends[child]);
    if ((global_stream = fdopen(pipe_ends[parent], type)) == NULL) {
      close(pipe_ends[parent]);
      return NULL;
    }
  }

  return global_stream;
}

int mypclose(FILE *stream) {
  int status;

  if (global_stream == NULL) {
    errno = ECHILD;
    return -1;
  }

  if (global_stream != stream) {
    errno = EINVAL;
    return -1;
  }

  if (fclose(stream) == EOF) {
    pid = -1;
    global_stream = NULL;
    errno = ECHILD;
    return -1;
  }

  do {
    wait(&status);
  } while (errno == EINTR);

  pid = -1;
  global_stream = NULL;

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }

  errno = ECHILD;
  return -1;
}
