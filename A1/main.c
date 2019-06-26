#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <assert.h>

int main(int argc, char const *argv[]) {

  int childpid;
  errno = 0;

  childpid = fork();
  if(childpid < 0){
    perror("Bad Fork");
    exit(EXIT_FAILURE);
  }

  /*child process */
  if (childpid ==0){
    int e;
    e = execl("./counter", "counter", "5", NULL);
    if (e ==-1){
      perror("execl");
      exit(EXIT_FAILURE);
    }
  }

  /*Parent process*/
  if (childpid > 0){
    int status;
    int e;
    int result = -1;
    assert(printf("Parent PID: %i\n", getpid()) != 0);
    e = wait(&status);

    if (e < 0){
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    if(WIFEXITED(status)== 0){
      perror("waitpid");
      exit(EXIT_FAILURE);
    } else{
      result = WEXITSTATUS(status);
      assert(printf("Child %i returned with status %i \n", childpid, result) != 0);
    }
  }

  exit(1);
}
