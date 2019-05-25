#include "benchmark.h"
#include "logger.h"

static struct timespec begin, end;

static int clear_page_cache()
{
	logger(2,"Page cache cleared\n");
	sync();
	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	if(fd < 0){
		perror("Opening /proc/sys/vm/drop_caches");
		return -1;
	}
	if(write(fd, "1", 1) < 0){
		perror("Write error: /proc/sys/vm/drop_caches");
		return -1;
	}
	close(fd);
	return 0;
}

static inline int write_page(int fd)
{
	char a = '\0';
	int ret = write(fd, &a, 1);
	if(ret<0) return ret;
	lseek(fd, PAGESIZE - 1, SEEK_CUR);
	return ret;
}

static inline int read_page(int fd)
{
	char a;
	int ret = read(fd, &a, 1);
	if(ret<0) return ret;
	lseek(fd, PAGESIZE - 1, SEEK_CUR);
	return ret;
}

static inline int write_page_no(int fd,unsigned long page_no)
{
	lseek(fd,PAGESIZE * page_no,SEEK_SET);
	return write_page(fd);
}

static inline int skip_pages(int fd,unsigned long nb_pages)
{
	return lseek(fd, nb_pages * PAGESIZE, SEEK_CUR);
}

int sequential_page_write(int fd,unsigned long nb_pages,int preread,unsigned long offset)
{
	long position = lseek(fd, 0, SEEK_END);
	if((nb_pages+offset)*PAGESIZE > (unsigned long)position){ /* File too small */
		fprintf(stderr,"sequential_page_write: File is too small for %lu pages\n",nb_pages);
		return -1;
	}

	if(preread!=0){
		lseek(fd,offset*PAGESIZE,SEEK_SET);
		unsigned long readen;
		for(readen=0;readen<nb_pages && read_page(fd)>0 ;readen++);
	}

	logger(2,"sequential_page_write: will write %lu pages\n",nb_pages);
	lseek(fd,offset*PAGESIZE,SEEK_SET);

	unsigned long written;

	clock_gettime(CLOCK_REALTIME, &begin);
  
	for(written=0;written<nb_pages && write_page(fd)>0 ;written++);

	clock_gettime(CLOCK_REALTIME, &end);

	if(written<nb_pages)
		fprintf(stderr,"sequential_page_write: Warning: Only %lu pages have been "
			"written over %lu\n",written,nb_pages);

	return written;
}

void shuffle_array(unsigned long *array,unsigned long n)
{
	srand(time(NULL));
	if (n > 1){
		unsigned long i;
		for (i = 0; i < n - 1; i++) {
			unsigned long j = i + rand() / (RAND_MAX / (n - i) + 1);
			unsigned long t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

int random_page_write(int fd,unsigned long nb_pages,int preread,unsigned long offset)
{
	long position = lseek(fd, 0, SEEK_END);
	if((nb_pages+offset)*PAGESIZE > (unsigned long)position){ /* File too small */
		fprintf(stderr,"random_page_write: File is too small for %lu pages\n",nb_pages);
		return -1;
	}

	if(preread!=0){
		lseek(fd,offset*PAGESIZE,SEEK_SET);
		unsigned long readen;
		for(readen=0;readen<nb_pages && read_page(fd)>0 ;readen++);
	}

	logger(2,"random_page_write: will write %lu pages\n",nb_pages);
	lseek(fd,offset*PAGESIZE,SEEK_SET);

	unsigned long page_tab[nb_pages];

	for(unsigned long i=0;i<nb_pages;i++)
		page_tab[i]=i+offset;

	shuffle_array(page_tab,nb_pages);

	unsigned long written;

	clock_gettime(CLOCK_REALTIME, &begin);
  
	for(written=0;written<nb_pages && \
		    write_page_no(fd,page_tab[written])>0;written++);

	clock_gettime(CLOCK_REALTIME, &end);

	if(written<nb_pages)
		fprintf(stderr,"random_page_write: Warning: Only %lu pages have been "
			"written over %lu\n",written,nb_pages);
  
	return written;

}

int gap_page_write(int fd, unsigned long nb_pages,unsigned long gap_size,\
		   int preread,unsigned long offset)
{
	if(gap_size == 0){
		return sequential_page_write(fd,nb_pages,preread,offset);
	}
	long position = lseek(fd, 0, SEEK_END);
	if((nb_pages+offset) * PAGESIZE + gap_size * nb_pages >	\
	   (unsigned long)position){ /* File too small */
		fprintf(stderr,"gap_page_write: File is too small for %lu pages\n",nb_pages);
		return -1;
	}
  
	unsigned long readen;
	switch(preread){
	case 1:
		lseek(fd,offset*PAGESIZE,SEEK_SET);
		for(readen=0;readen<nb_pages && read_page(fd)>0 ;readen++){
			skip_pages(fd,gap_size);
		}
		break;
	case 2:
		lseek(fd,offset*PAGESIZE,SEEK_SET);
		for(readen=0;readen<(nb_pages + (nb_pages - 1) * gap_size) && \
			    read_page(fd)>0 ;readen++);
		break;
	default:;
	}

	logger(2,"gap_page_write: will write %lu pages with %lu gap size\n",nb_pages,gap_size);

	lseek(fd,offset*PAGESIZE,SEEK_SET);

	unsigned long written;

	clock_gettime(CLOCK_REALTIME, &begin);
  
	for(written=0;written<nb_pages && write_page(fd)>0 ;written++)
		skip_pages(fd,gap_size);

	clock_gettime(CLOCK_REALTIME, &end);

	if(written<nb_pages)
		fprintf(stderr,"gap_page_write: Warning: Only %lu pages have been "
			"written over %lu\n",written,nb_pages);

	return written;
}

/**
 * 
 * @return: 0 success
 *          1 wrong arguments
 *          2 error opening file
 *          3 no write happened
 *          4 writting has started but stopped before 
 *            being completed (because of a write error)
 *          5 fsync failed
 *          6 page cache clear failed
 */
int main(int argc,char **argv)
{
	enum {SEQUENTIAL,GAP,RANDOM};

	if(argc < 2){
		printf("Usage: ./benchmark [OPTION]...  <path to file>\n(Use -h for options)\n");
		return 1;
	}

	/* default values */
	int preread            = 0;
	int verbosity          = 0;
	unsigned long nb_pages = 1;
	int sync               = 0;
	int method             = SEQUENTIAL;
	int gap_size           = 0;
	int clear_cache        = 0;
	unsigned long offset   = 0;
	
	int c,ret;
	opterr = 0;

	while ((c = getopt (argc, argv, "o:v:m:p:r:shc")) != -1)
		switch (c){
		case 'o':
			offset = strtoul(optarg,NULL,10);
			break;
		case 'v':
			verbosity = atoi(optarg);
			break;
		case 'm':
			if(strcmp("sequential",optarg)==0){
				method = SEQUENTIAL;
			}else if(strcmp("random",optarg)==0){
				method = RANDOM;
			}else{
				if(strlen(optarg) <= 4)
					goto err;
				method = GAP;
				char dest[5];
				dest[4]='\0';
				memcpy(dest,optarg,4);
				if(strcmp("gap=",dest)==0){
					gap_size=strtoul(optarg+4,NULL,10);
				}else{
					goto err;
				}
			}
			break;
		err:
			fprintf(stderr,"Wrong method: '%s' not a method\n",optarg);
			return 1;
		case 'p':
			nb_pages = strtoul(optarg,NULL,10);
			if(nb_pages == 0){
				fprintf(stderr,"'how many' parameter is invalid\n");
				return 1;
			}
			break;
		case 'r':
			preread = atoi(optarg);
			if(preread < 0 || preread > 2){
				fprintf(stderr,"'preread' parameter is invalid\n");
				return 1;
			}
			break;
		case 'c':
			clear_cache = 1;
			break;
		case 's':
			sync = 1;
			break;
		case 'h':
			printf("Usage: ./benchmark [OPTION]...  <path to file>\n"
			"OPTIONS:\n"
			       "\t-v <INT> verbosity level, default is 0.\n"
			       "\t-o <INT> offset (in number of pages), default is 0.\n"
			       "\t-m <sequential | random | gap=INT> writting pattern, default is sequential.\n"
			       "\t-p <INT> number of pages effectively written, default is 1.\n"
			       "\t-r <INT: 0..2> preread, if 1 written pages will be read first,\n"
			       "\t   if 2, all pages (including gap) will be read first, default is 0.\n"
			       "\t-c clear page cache before benchmarking.\n"
			       "\t-s call fsync before exiting.\n"
			       "\t-h display help (but you know that).\n");
			return 0;
		case '?':
			fprintf (stderr,"Unknown option or missing argument: -%c (use -h for help)\n",optopt);
			return 1;
		default:
			abort ();
	}
	char * path = argv[argc - 1];

	setVerbosityLevel(verbosity);

	/* Cleaning page cache for benchmark to be fair*/
	if(clear_cache && clear_page_cache()<0)
		return 6;
  
	int fd = open(path,preread ? O_RDWR : O_WRONLY);
	if(fd < 0){
		perror("Opening file");
		return 2;
	}

	switch(method){
	case SEQUENTIAL:
		ret = sequential_page_write(fd,nb_pages,preread,offset);
		break;
	case RANDOM:
		ret = random_page_write(fd,nb_pages,preread,offset);
		break;
	case GAP:
		ret = gap_page_write(fd,nb_pages,gap_size,preread,offset);
		break;
	default:
		abort();
	}
	if(ret == -1)
		return 3;
	if(ret<nb_pages)
		return 4;
	
	if(sync && fsync(fd)<0){
		perror("fsync failed");
		return 5;
	}
  
	logger(0,"%f\n",(end.tv_sec - begin.tv_sec) +	\
	       (double)(end.tv_nsec - begin.tv_nsec) / 1000000000);
  
	return 0;
}
