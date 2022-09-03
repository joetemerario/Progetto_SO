#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include <stdio.h>

void polite(void* args) {
    for (int i = 0; i < 50; i++)
    {
        printf("Hello, this function was called by a thread created with a syscall. Iteration: %d\n", i);
    }

    disastrOS_exit(disastrOS_getpid() + 1);
}