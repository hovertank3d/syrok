/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <cyb3r@nigge.rs> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Mykhailo Chernysh
 * ----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "syrok.h"

int main(int argc, char **argv)
{
    char *in_file_path;
    char *out_file_path;
    int mode;

    char *file;
    int filesize;

    if (argc != 4) {
        printf("requires 3 arguments!\n");
        return 1;
    }
    
    in_file_path = argv[1];
    out_file_path = argv[2];
    mode = atoi(argv[3]);

    file = syrok_read_file(in_file_path, &filesize, mode);

    if (file == NULL) {
        fprintf(stderr, "err: %s\n", syrok_get_error());
    }

    FILE *f = fopen(out_file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "err: %s\n", strerror(errno));
    }

    fwrite(file, 1, filesize, f);
    fclose(f);
}