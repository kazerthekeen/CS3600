#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

/* Main program based off of program in man strtol
Should be run as "counter num" */
int main(int argc, char *argv[]){

  ///Was having an issue with sleep preventing printf from working so used this implimentation of print
  char str[40];
  int len = sprintf(str, "Child Process %i running.\n", getpid());

  assert(len < 40);
  while(1){
    assert( write(1, str, len) != 0 );
    assert( sleep(1) ==0 );
  }
  exit(0);
}
