#include <stdio.h>
#include <linux/limits.h>
#include "../../src/mypopen.h"

int main(int argc, char *argv[]) {
    FILE *fp;
    int status;
    char path[PATH_MAX];

    fp = mypopen("ls -l", "r");
    if (fp == NULL) {
        return errno;
    }

    while (fgets(path, PATH_MAX, fp) != NULL) {
        printf("%s", path);
    }

    status = mypclose(fp);

    return errno;
}