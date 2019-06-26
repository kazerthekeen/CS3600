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
  int ppid = getppid();

  assert(printf("Child PID: %i\n", getpid()) != 0);

  if (argc != 1) {
    assert(printf("Bad number of args, expecting 1 have: %i\n", argc) != 0);
    exit(EXIT_FAILURE);
  }

  errno = 0;
  assert( kill(ppid, SIGUSR1 ) == 0 );
  assert( kill(ppid, SIGUSR2 ) == 0 );
  assert( kill(ppid, SIGALRM ) == 0 );
  assert( kill(ppid, SIGALRM ) == 0 );
  assert( kill(ppid, SIGALRM ) == 0 );


  exit(0);
}
