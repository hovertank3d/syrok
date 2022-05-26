/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <cyb3r@nigge.rs> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Mykhailo Chernysh
 * ----------------------------------------------------------------------------
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"

/* TODO: thread safe errors */
static syrok_error *syrok_get_error_buffer()
{
    static syrok_error errbuf;
    return &errbuf;
}

void syrok_clear_error()
{
    syrok_get_error_buffer()->error = 0;
}

void syrok_set_error(const char *fmt, ...)
{
    syrok_error *err;
    va_list args;

    err = syrok_get_error_buffer();
    
    va_start(args, fmt);
    vsnprintf(err->message, ERRBUFLEN, fmt, args);
    va_end(args);

    err->error = 1;
}

const char *syrok_get_error()
{
    syrok_error *err = syrok_get_error_buffer();
    return err->error ? err->message : "";
}