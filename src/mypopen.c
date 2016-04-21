#include "mypopen.h"

/**
 * a global variable containing process id returned by fork
 */
static pid_t pid = -1;

/**
 * a global variable containing a pointer of a pipe stream initiated by mypopen
 */
static FILE *global_stream = NULL;

/**
 * @brief initiate a pipe stream to or from a process
 *
 * @param command the command to be executed
 * @param type the I/O mode (r/w)
 *
 * @returns a file pointer or NULL in case of error
 */
FILE *mypopen(const char *command, const char *type) {
  int pipe_ends[2];
  int parent, child;

  /* check if already open */
  if (global_stream != NULL) {
    errno = EAGAIN;
    return NULL;
  }

  /* check the command input */
  if (command == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* check the type input */
  if (strcmp(type, "r") == 0) {
    parent = READ;
    child = WRITE;
  } else if (strcmp(type, "w") == 0) {
    parent = WRITE;
    child = READ;
  } else {
    errno = EINVAL;
    return NULL;
  }

  /* create a pipe */
  if (pipe(pipe_ends) == -1) {
    return NULL;
  }

  /* create a child process */
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
        exit(EXIT_FAILURE);
      }
      close(pipe_ends[child]);
    }
    execl("/bin/sh", "sh", "-c", command, NULL);
    /* reached only if execl failed */
    exit(EXIT_FAILURE);
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

/**
 * @brief close a pipe stream to or from a process
 *
 * @param stream the stream to be closed
 *
 * @returns the exit status of the process or -1 in case of error
 */
int mypclose(FILE *stream) {
  pid_t wait_pid;
  int status;

  /* check if mypopen was previously run */
  if (global_stream == NULL) {
    errno = ECHILD;
    return -1;
  }

  /* check if we are closing the correct stream */
  if (global_stream != stream) {
    errno = EINVAL;
    return -1;
  }

  /* close the stream */
  if (fclose(stream) == EOF) {
    pid = -1;
    global_stream = NULL;
    errno = ECHILD;
    return -1;
  }

  /* wait for a child process to terminate */
  while ((wait_pid = waitpid(pid, &status, 0)) != pid) {
    if (wait_pid == -1) {
      if (errno == EINTR) {
        continue;
      }
    }

    /* reached only if waitpid returned an unexpected value or errno */
    pid = -1;
    global_stream = NULL;
    errno = ECHILD;
    return -1;
  }

  pid = -1;
  global_stream = NULL;

  /* check if the child process terminated normally */
  if (WIFEXITED(status) != 0) {
    return WEXITSTATUS(status);
  }

  errno = ECHILD;
  return -1;
}
