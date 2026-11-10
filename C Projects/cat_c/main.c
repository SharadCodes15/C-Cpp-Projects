#include "stdio.h"
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


    int c;
    while((c = fgetc(fptr)) != EOF)
    {
      putchar(c);
    }

    fclose(fptr);
  }
}
