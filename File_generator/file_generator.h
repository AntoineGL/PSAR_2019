#pragma once

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define PAGESIZE sysconf(_SC_PAGESIZE)

/**
 * randomly_fill_buffer - fill buffer with random bytes
 * @buffer:    buffer to fill
 * @size:      buffer length
 */
void randomly_fill_buffer(char *buffer,unsigned size);

/**
 * zero_fill_buffer - fill buffer with zeros
 * @buffer:    buffer to fill
 * @size:      buffer length
 */
void zero_fill_buffer(char *buffer,unsigned size);

/**
 * generate_file - create file filled with data depending on
 *                        filling function
 * @name:       file name
 * @size:       size in byte of the file
 * @fill_block: filling bufferfunction
 *
 * @return:     file descriptor if success or -1.
 * @error:      invalid path, file already exists, size too big.
 */
int generate_file(char *name,unsigned long size,\
			 void (*fill_block)(char *,unsigned));

