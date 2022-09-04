#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

int alive_children=0;

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());

  printf("PID: %d\n", disastrOS_getpid());

  printf("**********************************TEST disastrOS_exec()**********************************\n");

  const char* shared_obj = "libdisastrOS.so";
  const char* func_symbol1 = "polite";

  disastrOS_exec(shared_obj, func_symbol1);
  alive_children++;

  printf("**********************************TEST disastrOS_terminate()**********************************\n");

  const char* func_symbol2 = "infinite_count";

  int pid_ret2;
  pid_ret2 = disastrOS_exec(shared_obj, func_symbol2);
  alive_children++;

  disastrOS_terminate(pid_ret2); //Grazie a questa chiamata a disastrOS_terminate, il thread viene terminato dal processo figlio
                                 //ancora prima di far partire l'esecuzione della funzione infinite_count. Per provare
                                 //commentare la disastrOS_terminate e decommentare la disastrOS_exit nel file
                                 //disastrOS_polite_function.c

  int retval;
  int pid;

  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    --alive_children;
  }

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {

  disastrOS_printStatus();

  printf("hello, I am initFunction and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  printf("I feel like to spawn 1 nice thread\n");

  disastrOS_spawn(childFunction, 0);
  alive_children++;

  disastrOS_printStatus();

  int retval;
  int pid;

  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n", pid, retval, alive_children);
    --alive_children;
  }

  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;

  if (argc>1) {
    logfilename=argv[1];
  }

  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p\n", childFunction);

  // spawn an init process
  printf("start\n");

  disastrOS_start(initFunction, 0, logfilename);

  return 0;
}
