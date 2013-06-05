#include <stdio.h>
#include <stdlib.h>

int main()
{ FILE *p_file;
  char buf[1024]; 
    printf("TEST 1\n");
  p_file = fopen("/home/test.txt","r");  
  if(p_file){
    fgets(buf,1024,p_file);
    printf("%s\n",buf);
  }
  else
    printf("error opening /home/test.txt\n");
  close (p_file);  
  
  printf("TEST 2\n");
  
  p_file = fopen("/home/a.txt","a");  
  if(p_file){
    fputs("BLAHBLAHBLAH\n",p_file);
    fflush(p_file);
    fputs("BLAHBLAHBLAH\n",p_file);
    fflush(p_file);
    fputs("BLAHBLAHBLAH\n",p_file);
    fflush(p_file);
    fputs("BLAHBLAHBLAH\n",p_file);
    fflush(p_file);
  }
  else
    printf("error opening /home/a.txt\n");
  close (p_file);
  
  printf("TEST 3\n");
  p_file = fopen("/home/test2.txt","a+");  
  if(p_file){
    fgets(buf,1024,p_file);
    printf("%s\n",buf);
    fputs("BLAHBLAHBLAH\n",p_file);
  }
  else
    printf("error opening /home/test2.txt\n");
  close (p_file);
    
  return 0;   
}
