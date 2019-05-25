#include "file_generator.h"


void randomly_fill_buffer(char *buffer,unsigned size)
{
  srand(time(NULL));
  for(unsigned i=0;i<size;i++)
    buffer[i]=random();
}

void zero_fill_buffer(char *buffer,unsigned size)
{
  memset(buffer,'\0',size);
}

int generate_file(char *name,unsigned long nb_pages,\
		  void (*fill_block)(char *,unsigned))
{  
  int fd = open(name, O_CREAT | O_WRONLY | O_EXCL , 0660);
  if(fd<0){
    perror("Open file error");
    return -1;
  }
  
  char buffer[PAGESIZE];

  printf("Starting writting %lu pages\n",nb_pages);

  for(unsigned long i=0;i<nb_pages;i++){
    fill_block(buffer,PAGESIZE);
    if(write(fd,buffer,PAGESIZE)<PAGESIZE){
      perror("Error while writting file\n");
      printf("File Size= %lu KB\n",(i*PAGESIZE)/1024);
      close(fd);
      return -1;
    }
  }

  printf("File Size= %lu KB\n",(PAGESIZE*nb_pages)/1024);

  if(fsync(fd)<0){
    perror("fsync failed\n");
    return -1;
  }
  
  close(fd);   
  return 0;
}
