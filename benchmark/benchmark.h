#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define PAGESIZE sysconf(_SC_PAGESIZE)

/**
 * clear_page_cache - Empty the Page Cache
 *
 * @return: -1 if error, 0 otherwise.
 */
static int clear_page_cache();

/**
 * write_page - Write 1 byte (of the page) and increment fd offset 
 *              by the size of a page
 * @fd:     file descriptor
 *
 * @return: write return value
 */
static inline int write_page(int fd);

/**
 * write_page - Read 1 byte (of the page) and increment fd offset 
 *              by the size of a page
 * @fd:     file descriptor
 *
 * @return: read return value
 */
static inline int read_page(int fd);

/**
 * write_page - Move offset to page_no and call write_page
 * @fd:     file descriptor
 *
 * @return: write_page return value
 */
static inline int write_page_no(int fd,unsigned long page_no);

/**
 * skip_pages - Move fd offset by nb_pages * page size
 *
 * @fd:       file descriptor
 * @nb_pages: number of pages to skip
 *
 * @return:   lseek value
 */
static inline int skip_pages(int fd,unsigned long nb_pages);

/**
 * sequential_page_write - Sequentially write the nb_pages from the beginning of
 *                         the file
 * @fd:       file descriptor of the file to be written
 *            file must be large enough
 * @nb_pages: number of pages which will be written

 * @return:   -1 if file too small
 *            the number of pages written
 */
int sequential_page_write(int fd,unsigned long nb_pages,\
			  int preread,unsigned long offset);

/**
 * shuffle_array - shuffles array
 *
 * @array: ptr to array
 * @b:     array size
 */
void shuffle_array(unsigned long *array,unsigned long n);

/**
 * random_page_write - Randomly write the nb_pages from the beginning of 
 *                     the file (no page will be written twice)
 * @fd:       file descriptor of the file to be written
 *            file must be large enough
 * @nb_pages: number of pages which will be written

 * @return:   -1 if file too small
 *            the number of pages written
 */
int random_page_write(int fd,unsigned long nb_pages,int preread,\
		      unsigned long offset);

/**
 * gap_page_write - Write nb_pages with gap_size pages space between
 *                  writes 
 *                         the file
 * @fd:       file descriptor of the file to be written
 *            file must be large enough
 * @nb_pages: number of pages which will be written
 * @gap_size: the size (in pages) of the space between writes
 *
 * @return:   -1 if file too small
 *            the number of pages written
 */
int gap_page_write(int fd, unsigned long nb_pages,unsigned long gap_size,\
		   int preread,unsigned long offset);
