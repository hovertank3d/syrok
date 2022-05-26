/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <cyb3r@nigge.rs> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Mykhailo Chernysh
 * ----------------------------------------------------------------------------
 */


#ifndef __SYROK_H__
#define __SYROK_H__

#include <stdint.h>
#include <stddef.h>

enum {
	SYROK_MONOCHROME = 0,
	SYROK_COLORED,	
	SYROK_COLORED_XOR,	
	SYROK_COLORED_AND,	
	SYROK_COLORED_OR,	
	SYROK_MODES_COUNT,
};

uint8_t *syrok_read_file(const char *filename, int *retsize, int mode);
uint8_t *syrok(const uint8_t *data, size_t filesize, int *retsize, int mode);
const char *syrok_get_error();

#endif //__SYROK_H__