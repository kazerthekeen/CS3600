#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
/*Recieved some assistance from the following
  - This stack post for how to set up a handler
    https://stackoverflow.com/questions/49593009/sigaction-structure
  - Talked extensivly to Andrew Pena
 */
void handler (int sig){
  switch(sig){
    case SIGUSR1 :
      assert( write(1, "Recieved User Signal 1\n", 23) > 0);
      break;
    case SIGUSR2 :
      assert( write(1, "Recieved User Signal 2\n", 23) > 0);
      break;
    case SIGALRM :
      assert( write(1, "Recieved alarm signal, is it time to do something else?\n", 56) > 0);
      break;
    default :
      assert( write(1, "Recieved unknown signal number.\n", 32) > 0);
      break;
  }


}

int main(int argc, char const *argv[]) {
  int childpid;
  errno = 0;

  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = SA_RESTART;
  assert (sigaction (SIGUSR1, &action, NULL) == 0);
  assert (sigaction (SIGUSR2, &action, NULL) == 0);
  assert (sigaction (SIGALRM, &action, NULL) == 0);

  /*Fork*/
  childpid = fork();
  if(childpid < 0){
    perror("Bad Fork");
    exit(EXIT_FAILURE);
  }

  /*child process */
  if (childpid ==0){
    int e;
    e = execl("./child", "child", NULL);
    if (e ==-1){
      perror("execl");
      exit(EXIT_FAILURE);
    }
  }

  /*Parent process*/
  if (childpid > 0){
    assert( kill(childpid, SIGUSR1 ) == 0 );
    int status;
    int e;
    assert(printf("Parent PID: %i\n", getpid()) != 0);
    e = wait(&status);

    if (e != 0 && WIFEXITED(status)== 0){
      perror("waitpid");
      exit(EXIT_FAILURE);
    } else{
      assert(printf("Child %i returned successfully.\n", childpid) != 0);
    }
  }

  return(0);
}
