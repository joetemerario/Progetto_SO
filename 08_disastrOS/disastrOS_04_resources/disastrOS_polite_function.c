#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include <stdio.h>

void polite(void* args) {
    printf("******L'esecuzione della funzione potrebbe essere successiva alla terminazione della funzione disastrOS_exec()******\n");

    for (int i = 0; i < 50; i++)
    {
        printf("Hello, this function was called by a thread created with a syscall. Iteration: %d\n", i);
    }

    disastrOS_exit(disastrOS_getpid() + 1);
}

void infinite_count(void* args) {
    printf("******Se leggi questo print vuol dire che la disastrOS_terminate() non è in funzione******\n");

    for (int i = 0; i < 5000; i++)
    {
        printf("Questa print andrà avanti finchè il processo non verrà terminato\n");
    }

    //disastrOS_exit(disastrOS_getpid() + 1);   Se disattivo disastrOS_terminate e attivo questa chiamata, la funzione viene eseguita e, alla sua terminazione, viene eliminato il thread corrispondente
}