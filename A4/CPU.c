#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include "systemcall.h"

/*
This program does the following.
1) Creates handlers for two signals.
2) Create an idle process which will be executed when there is nothing
   else to do.
3) Create a send_signals process that sends a SIGALRM every so often.

The output looks approximately like:

[...]
Sending signal:  14 to process: 35050
Stopping: 35052
---- entering scheduler
Continuing idle: 35052
---- leaving scheduler
Sending signal:  14 to process: 35050
at the end of send_signals
Stopping: 35052
---- entering scheduler
Continuing idle: 35052
---- leaving scheduler
---- entering process_done
Timer died, cleaning up and killing everything
Terminated: 15

---------------------------------------------------------------------------
Add the following functionality.
1) Change the NUM_SECONDS to 20.
X

2) Take any number of arguments for executables and place each on the
   processes list with a state of NEW. The executables will not require
   arguments themselves.

3) When a SIGALRM arrives, scheduler() will be called; it currently simply
   restarts the idle process. Instead, do the following.
   a) Update the PCB for the process that was interrupted including the
      number of context switches and interrupts it had and changing its
      state from RUNNING to READY.
   b) If there are any NEW processes on processes list, change its state to
      RUNNING, and fork() and execl() it.
   c) If there are no NEW processes, round robin the processes in the
      processes queue that are READY. If no process is READY in the
      list, execute the idle process.

4) When a SIGCHLD arrives notifying that a child has exited, process_done() is
   called.
   a) Add the printing of the information in the PCB including the number
      of times it was interrupted, the number of times it was context
      switched (this may be fewer than the interrupts if a process
      becomes the only non-idle process in the ready queue), and the total
      system time the process took.
   b) Change the state to TERMINATED.
   c) Restart the idle process to use the rest of the time slice.
*/

#define NUM_SECONDS 20
#define ever ;;

enum STATE { NEW, RUNNING, WAITING, READY, TERMINATED, EMPTY };

struct PCB {
    enum STATE state;
    const char *name;   // name of the executable
    int pid;            // process id from fork();
    int ppid;           // parent process id
    int interrupts;     // number of times interrupted
    int switches;       // may be < interrupts
    int started;        // the time this process started
};

typedef enum { false, true } bool;

#define PROCESSTABLESIZE 10
struct PCB processes[PROCESSTABLESIZE];
struct PCB idle;
struct PCB *running;

int sys_time;
int timer;
struct sigaction alarm_handler;
struct sigaction child_handler;

void bad(int signum) {
    WRITESTRING("bad signal: ");
    WRITEINT(signum, 4);
    WRITESTRING("\n");
}


// cdecl> declare ISV as array 32 of pointer to function(int) returning void
void(*ISV[32])(int) = {
/*       00   01   02   03   04   05   06   07   08   09 */
/*  0 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 10 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 20 */ bad, bad, bad, bad, bad, bad, bad, bad, bad, bad,
/* 30 */ bad, bad
};

void ISR (int signum) {
    if (signum != SIGCHLD) {
        kill (running->pid, SIGSTOP);
        WRITESTRING("Stopping: ");
        WRITEINT(running->pid, 6);
        WRITESTRING("\n");
    }

    ISV[signum](signum);
}

void printPCB(struct PCB pcb){
  WRITESTRING(pcb.name);
  WRITESTRING("PID:");
  WRITEINT(pcb.pid,6);
  WRITESTRING("PPID:");
  WRITEINT(pcb.ppid,6);
  WRITESTRING("Interrupts:");
  WRITEINT(pcb.interrupts,6);
  WRITESTRING("Switches:");
  WRITEINT(pcb.switches,6);
}

void send_signals(int signal, int pid, int interval, int number) {
    for(int i = 1; i <= number; i++) {
        sleep(interval);
        WRITESTRING("Sending signal: ");
        WRITEINT(signal, 4);
        WRITESTRING(" to process: ");
        WRITEINT(pid, 6);
        WRITESTRING("\n");
        systemcall(kill(pid, signal));
    }

    WRITESTRING("At the end of send_signals\n");
}

void create_handler(int signum, struct sigaction action, void(*handler)(int)) {
    action.sa_handler = handler;

    if (signum == SIGCHLD) {
        action.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    } else {
        action.sa_flags =  SA_RESTART;
    }

    systemcall(sigemptyset(&action.sa_mask));
    systemcall(sigaction(signum, &action, NULL));
}

int getrunningindex(){
  struct PCB run = *running;
  for (int i =0; i<10; i++){
    if(processes[i].pid == run.pid) {
      return i;
    }
  }
  return 0;
}

void scheduler (int signum) {
    WRITESTRING("---- entering scheduler\n");
    assert(signum == SIGALRM);
    struct PCB run = *running;
    assert( kill(run.pid, SIGSTOP) == 0);
    run.interrupts += 1;
    run.state = READY;
    // If a new process exists make it and run it

    int offset = getrunningindex() % 10;
    // Find the next ready process and run it
    for( int i=1; i<10; i++){
      int index = i + offset % 10;
      if(processes[index].state == READY || processes[index].state == NEW){
        processes[index].state = RUNNING;
        processes[index].switches += 1;
        assert( kill(processes[index].pid, SIGCONT) == 0);
        running = &processes[index];
        break;
      } else if (index == offset && processes[index].state != READY){
        running = &idle;
      }
    }


    WRITESTRING("---- leaving scheduler\n");
}

void process_done (int signum) {
    WRITESTRING("---- entering process_done\n");
    assert (signum == SIGCHLD);

    WRITESTRING ("Timer died, cleaning up and killing everything\n");
    systemcall(kill(0, SIGTERM));

    WRITESTRING ("---- leaving process_done\n");
}

void boot()
{
    sys_time = 0;

    ISV[SIGALRM] = scheduler;
    ISV[SIGCHLD] = process_done;
    create_handler(SIGALRM, alarm_handler, ISR);
    create_handler(SIGCHLD, child_handler, ISR);

    assert((timer = fork()) != -1);
    if (timer == 0) {
        send_signals(SIGALRM, getppid(), 1, NUM_SECONDS);
        exit(0);
    }
}

void create_idle() {
    idle.state = READY;
    idle.name = "IDLE";
    idle.ppid = getpid();
    idle.interrupts = 0;
    idle.switches = 0;
    idle.started = sys_time;

    assert((idle.pid = fork()) != -1);
    if (idle.pid == 0) {
        systemcall(pause());
    }
}

int main(int argc, char **argv) {
  if(argc > 10){
    assert(printf("To many processes to run.\nSupplied %i arguements only 10 permitted.", argc) != 0);
  }
  assert(printf("Have %i processes to run.\n", argc-1) != 0);


  boot();
  create_idle();
  running = &idle;

  /// Add the processes to the PCB list
  for( int i=1; i<=argc; i++){
    processes[i].ppid = getpid();
    processes[i].name = argv[i];
    processes[i].interrupts = 0;
    processes[i].switches = 0;
    processes[i].started = sys_time;
    processes[i].state = NEW;
    assert((processes[i].pid = fork()) != -1);

    if (processes[i].pid == 0) {
      execl(argv[i], processes[i].name, NULL);
      WRITESTRING("EXECL FAILURE.\n")
      exit(-1);
    } else {
      assert(kill(processes[i].pid, SIGSTOP) == 0);
      running = &processes[i];
    }
  }

  for(ever){
      pause();
  }
}
