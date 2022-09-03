#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_descriptor.h"

void internal_terminate()
{
    printf("Sono dentro internal_terminate\n");

    running->return_value = 0;

    int target_pid = running->syscall_args[0];

    PCB *target = PCB_byPID(&ready_list, target_pid);

    if (!target)
    {
        target = PCB_byPID(&waiting_list, target_pid);
    }

    assert(init_pcb);
    while (target->children.first)
    {

        // detach from current list
        ListItem *item = List_detach(&target->children, target->children.first);
        assert(item);

        // attach to init's children list
        List_insert(&init_pcb->children, init_pcb->children.last, item);

        // send SIGHUP
        PCBPtr *pcb_ptr = (PCBPtr *)item;
        PCB *pcb = pcb_ptr->pcb;
        pcb->signals |= (DSOS_SIGHUP & pcb->signals_mask);
    }

    target->status = Zombie;
    List_insert(&zombie_list, zombie_list.last, (ListItem *)target);
    target->parent->signals |= (DSOS_SIGCHLD & target->parent->signals_mask);

    // if the parent was waiting for this process to die
    if (target->parent->status == Waiting
        // since he called  wait or waitpid, we wake him up
        && target->parent->syscall_num == DSOS_CALL_WAIT && (target->parent->syscall_args[0] == 0 || target->parent->syscall_args[0] == target->pid))
    {
        PCB *parent = (PCB *)List_detach(&waiting_list, (ListItem *)target->parent);
        assert(parent);
        parent->status = Running;

        // we remove the process from the parent's children list
        PCBPtr *self_in_parent = PCBPtr_byPID(&parent->children, target->pid);
        List_detach(&parent->children, (ListItem *)self_in_parent);

        // hack the PCB of the parent to put in return value
        parent->syscall_retvalue = target->pid;
        int *result = (int *)parent->syscall_args[1];
        if (result)
            *result = target->return_value;

        // we release all timers put by this process from the timer list, in case there was someone
        // hanging
        // WARNING: untested
        ListItem *aux = timer_list.first;
        while (aux)
        {
            TimerItem *timer = (TimerItem *)aux;
            aux = aux->next;
            if (timer->pcb == target)
            {
                ListItem *detach_result = List_detach(&timer_list, (ListItem *)timer);
                assert(detach_result);
                TimerItem_free(timer);
            }
        }

        // we release all resources of a process upon termination
        while (running->descriptors.first)
        {
            Descriptor *des = (Descriptor *)target->descriptors.first;
            List_detach(&target->descriptors, (ListItem *)des);
            Resource *res = des->resource;
            List_detach(&res->descriptors_ptrs, (ListItem *)des->ptr);
            DescriptorPtr_free(des->ptr);
            Descriptor_free(des);
        }

        // the process finally dies
        ListItem *suppressed_item = List_detach(&zombie_list, (ListItem *)target);
        PCB_free((PCB *)suppressed_item);
        target = parent;
    }
    else
    {
        // we put the first ready process in running
        PCB *next_running = (PCB *)List_detach(&ready_list, ready_list.first);
        assert(next_running);
        next_running->status = Running;
        target = next_running;
    }
}