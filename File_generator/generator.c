#include "file_generator.h"

int main(int argc, char **argv){
  if(argc!=4){
    printf("Usage: ./generate <Type of data (zero | random)> \n"
	   "                  <unsigned long:how many pages to write> \n"
	   "                  <file name>\n"
	   "(Actual system page size: %ld bytes)\n",PAGESIZE);
    return 1;
  }
  unsigned long size=strtoul(argv[2],NULL,10);
  
  if(size == 0){
    printf("Invalid size\n");
    return 1;
  }

  if(strcmp("zero",argv[1])){
    if(generate_file(argv[3],size,&zero_fill_buffer)<0)
      return 2;
  }else if(strcmp("random",argv[1])){
    if(generate_file(argv[3],size,&randomly_fill_buffer)<0)
      return 2;
  }else{
    printf("Wrong first argument: '%s' not an option\n",argv[1]);
    return 1;
  }
  
  
  return 0;
}
