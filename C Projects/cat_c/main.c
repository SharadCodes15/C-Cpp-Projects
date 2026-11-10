#include "stdio.h"

#define BUFFER_SIZE 1024


int main(int argc,char *argv[]){
  if(argc == 1){
    printf("Usage : %s <filename>.. \n",argv[0]);
    return 0;
  }
  for (int i= 1;i<argc;i++){
    const char *path = argv[i];
    FILE *fptr = fopen(path,"r");
    if (fptr == NULL){
      perror(path);
      continue;
    }
    char buffers[BUFFER_SIZE];
    size_t nitems; // num of bytes to read 
    
    while((nitems = fread(buffers,1,BUFFER_SIZE,fptr)))
      fwrite(buffers,1,nitems,stdout);


    // You can add error checks

    int c;
    while((c = fgetc(fptr)) != EOF)
    {
      putchar(c);
    }

    fclose(fptr);
  }
}
