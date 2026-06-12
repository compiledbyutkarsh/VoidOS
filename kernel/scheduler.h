#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../lib/types.h"

#define SCHED_MAX_TASKS     64
#define SCHED_STACK_SIZE    8192
#define SCHED_QUANTUM       10

#define TASK_RUNNING        0
#define TASK_READY          1
#define TASK_BLOCKED        2
#define TASK_SLEEPING       3
#define TASK_ZOMBIE         4
#define TASK_DEAD           5

#define PRIORITY_IDLE       0
#define PRIORITY_LOW        1
#define PRIORITY_NORMAL     2
#define PRIORITY_HIGH       3
#define PRIORITY_REALTIME   4

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, ss;
} PACKED cpu_state_t;

typedef struct task {
    uint32_t     pid;
    uint32_t     ppid;
    char         name[32];
    uint8_t      state;
    uint8_t      priority;
    uint32_t     quantum;
    uint32_t     sleep_ticks;
    cpu_state_t  cpu;
    uint32_t     stack[SCHED_STACK_SIZE / sizeof(uint32_t)];
    uint32_t     ipc_port;
    uint32_t     exit_code;
    struct task *next;
} task_t;

void    scheduler_init();
task_t *scheduler_create_task(const char *name, void (*entry)(), uint8_t priority);
void    scheduler_destroy_task(uint32_t pid);
void    scheduler_schedule();
void    scheduler_yield();
void    scheduler_sleep(uint32_t ticks);
void    scheduler_block(uint32_t pid);
void    scheduler_unblock(uint32_t pid);
task_t *scheduler_get_current();
task_t *scheduler_get_task(uint32_t pid);
void    scheduler_dump_tasks();
uint32_t scheduler_get_task_count();

#endif
