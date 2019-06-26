#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
/* Main program based off of program in man strtol
Should be run as "counter num" */
int main(int argc, char *argv[]){
  char *endptr, *str;
  long val;
  int pid;

  assert(printf("Child PID: %i\n", getpid()) != 0);

  if (argc != 2) {
    assert(printf("Bad number of args, expecting 2 have: %i\n", argc) != 0);
    exit(EXIT_FAILURE);
  }

  str = argv[1];

  errno = 0;    /* To distinguish success/failure after call */
  val = strtol(str, &endptr, 10);

  /* Check for various possible errors */
  if (endptr == str || *endptr != '\0') {
    assert(fprintf(stderr, "Not a number\n") != 0);
    exit(EXIT_FAILURE);
  }

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0) || *endptr != '\0') {
    perror("strtol");
    exit(EXIT_FAILURE);
  }

  if (val <= 0 ) {
    assert(printf("Invalid Range") != 0);
    exit(EXIT_FAILURE);
  }

  /* If we got here, strtol() successfully parsed a number */
  pid = getpid();
  for(int i = 1; i<= val; i++){
    assert(printf("%i returned %i\n", pid, i) != 0);
  }

  return(val);
}
