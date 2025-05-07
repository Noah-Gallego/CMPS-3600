/* volatile.c: compile the program below with and without
  optimization and see what effect volatile and register 
  have, i.e. compare:

     $ gcc -g volatile.c -o noop 
     $ ./noop
         var=4 war=4 xar=4 
  to
     $ gcc -g -O2 volatile.c -o withop      
     $./withop 
         var=3 war=4 xar=3 

Does the differing output make sense?  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>

int main(int argc, char *argv[], char *envp[]) {
  register int var = 3;
  volatile int war = 3;
  int xar = 3;
  jmp_buf context;

  if (setjmp(context) != 0) {
    printf("\n var=%d war=%d xar=%d \n\n", var, war, xar);
    return(0);
  }

  var = 4;
  war = 4;
  xar = 4;
  longjmp(context, 1);

  printf("\n\n");
  return(0);
}
