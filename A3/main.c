#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>



/*Recieved some assistance from the following
  - Talked with Andrew Pena
  - eyetoeh was copied from program becouse I was in part to lazy to figure out
  how to get the information gathered as is.
 */

#define syscall(x) { \
     (x); \
     int err = errno; \
     if (err) { \
         fprintf(stderr, "In file %s at line %d: ", __FILE__, __LINE__); \
         perror(#x); \
         exit(err);} \
     }

#define WRITESTRING(STRING) \
     syscall(write(STDOUT_FILENO, STRING, strlen(STRING)));

#define WRITEINT(INT, LEN) { char buf[LEN]; \
     syscall(eye2eh(INT, buf, LEN, 10)); WRITESTRING(buf); }

int eye2eh(int i, char *buffer, int buffersize, int base) {
    if (i < 0 || buffersize <= 1 || base < 2 || base > 16) {
        errno = EINVAL;
        return -1;
    }

    buffer[buffersize-1] = '\0';

    int count = 0;
    const char *digits = "0123456789ABCDEF";
    for (int j = buffersize-2; j >= 0; j--) {
        if (i == 0 && count != 0) {
            buffer[j] = ' ';
        }
        else {
            buffer[j] = digits[i%base];
            i = i/base;
            count++;
        }
    }

    if (i != 0) {
        errno = EINVAL;
        return -1;
    }

    return count;
}
void handler (int sig){
  switch(sig){
    case SIGCHLD:
      assert( write(1, "Recieved SIGCHLD\n", 17) > 0);
      int status;
      int e;
      int result = -1;
      e = wait(&status);

      if (e < 0){
        assert(write(1, "return failed\n", 14) != 0);
        exit(EXIT_FAILURE);
      }else if(WIFSIGNALED(status)==0){
        assert(write(1, "No returned status.\n", 18) != 0);
      } else{
        assert(write(1, "Return status:\n", 14) != 0);
        result = WTERMSIG(status);
        WRITEINT(result, 3);
      }
      exit(1);
      break;

    default :
      assert( write(1, "Unknown signal recieved\n", 17) > 0);
      break;
  }


}

int main(int argc, char const *argv[]) {
  int childpid;
  errno = 0;

  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  assert (sigaction (SIGINT, &action, NULL) == 0);
  assert (sigaction (SIGCHLD, &action, NULL) == 0);

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

    for( int i = 0; i<5; i++){
      assert( printf("Parent is sending SIGSTOP\n") != 0 );
      assert( kill(childpid, SIGSTOP ) == 0 );
      assert( sleep(2) == 0 );
      assert( printf("Parent is sending SIGCONT\n") != 0 );
      assert( kill(childpid, SIGCONT ) == 0 );
      assert( sleep(2) == 0 );
    }
    assert( printf("Parent is sending SIGINT\n") != 0 );
    assert( kill(childpid, SIGINT ) == 0 );

    pause();
    perror("pause");
    exit(EXIT_FAILURE);
  }

  return(0);
}
