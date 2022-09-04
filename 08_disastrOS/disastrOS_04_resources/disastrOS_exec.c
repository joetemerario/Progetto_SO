#include <assert.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"

void internal_exec()
{
    printf("Sono dentro internal_exec\n");

    void *handle;
    void (*func)(void *);
    char *error;

    const char *shared_obj = (const char *)running->syscall_args[0];
    handle = dlopen(shared_obj, RTLD_LAZY);

    printf("Shared Object da aprire: %s\n", shared_obj);

    if (!handle)
    {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();

    const char *func_symbol = (const char *)running->syscall_args[1];
    func = (void (*)(void *))dlsym(handle, func_symbol);

    printf("Simbolo della funzione da eseguire: %s\nIndirizzo della funzione all'interno della memoria: %p\n", func_symbol, func);

    error = dlerror();

    if (error)
    {
        printf("%s\n", error);
        exit(EXIT_FAILURE);
    }

    static PCB *new_pcb;
    new_pcb = PCB_alloc();
    if (!new_pcb)
    {
        running->syscall_retvalue = DSOS_ESPAWN;
        return;
    }

    new_pcb->status = Ready;

    // sets the parent of the newly created process to the running process
    new_pcb->parent = running;

    // adds a pointer to the new process to the children list of running
    PCBPtr *new_pcb_ptr = PCBPtr_alloc(new_pcb);
    assert(new_pcb_ptr);
    List_insert(&running->children, running->children.last, (ListItem *)new_pcb_ptr);

    // adds the new process to the ready queue
    List_insert(&ready_list, ready_list.last, (ListItem *)new_pcb);

    // sets the retvalue for the caller to the new pid
    running->syscall_retvalue = new_pcb->pid;

    getcontext(&new_pcb->cpu_state);

    new_pcb->cpu_state.uc_stack.ss_sp = new_pcb->stack;
    new_pcb->cpu_state.uc_stack.ss_size = STACK_SIZE;
    new_pcb->cpu_state.uc_stack.ss_flags = 0;
    sigemptyset(&new_pcb->cpu_state.uc_sigmask);
    new_pcb->cpu_state.uc_link = &main_context;
    
    makecontext(&new_pcb->cpu_state, (void(*)()) func, 1, 0);
}