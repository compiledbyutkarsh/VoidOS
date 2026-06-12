#include "scheduler.h"
#include "ipc.h"
#include "../drivers/vga.h"

static task_t   task_pool[SCHED_MAX_TASKS];
static task_t  *current_task = NULL;
static task_t  *task_queue   = NULL;
static uint32_t next_pid     = 1;
static uint32_t task_count   = 0;

static void task_idle() {
    while (1) {
        __asm__ volatile("hlt");
    }
}

static task_t *alloc_task() {
    for (int i = 0; i < SCHED_MAX_TASKS; i++) {
        if (task_pool[i].state == TASK_DEAD) {
            return &task_pool[i];
        }
    }
    return NULL;
}

static void queue_push(task_t *task) {
    task->next = NULL;
    if (!task_queue) {
        task_queue = task;
        return;
    }
    task_t *cur = task_queue;
    while (cur->next) cur = cur->next;
    cur->next = task;
}

static __attribute__((unused)) task_t *queue_pop() {
    if (!task_queue) return NULL;
    task_t *task = task_queue;
    task_queue = task_queue->next;
    task->next = NULL;
    return task;
}

static task_t *queue_pop_by_priority() {
    if (!task_queue) return NULL;

    task_t *best      = task_queue;
    task_t *best_prev = NULL;
    task_t *cur       = task_queue->next;
    task_t *prev      = task_queue;

    while (cur) {
        if (cur->state == TASK_READY && cur->priority > best->priority) {
            best      = cur;
            best_prev = prev;
        }
        prev = cur;
        cur  = cur->next;
    }

    if (best_prev) {
        best_prev->next = best->next;
    } else {
        task_queue = best->next;
    }

    best->next = NULL;
    return best;
}

static void save_context(task_t *task) {
    __asm__ volatile(
        "mov %%eax, %0\n"
        "mov %%ebx, %1\n"
        "mov %%ecx, %2\n"
        "mov %%edx, %3\n"
        : "=m"(task->cpu.eax), "=m"(task->cpu.ebx),
          "=m"(task->cpu.ecx), "=m"(task->cpu.edx)
    );
    __asm__ volatile(
        "mov %%esp, %0\n"
        "mov %%ebp, %1\n"
        : "=m"(task->cpu.esp), "=m"(task->cpu.ebp)
    );
}

static void restore_context(task_t *task) {
    __asm__ volatile(
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        :
        : "m"(task->cpu.eax), "m"(task->cpu.ebx),
          "m"(task->cpu.ecx), "m"(task->cpu.edx)
    );
    __asm__ volatile(
        "mov %0, %%esp\n"
        "mov %1, %%ebp\n"
        :
        : "m"(task->cpu.esp), "m"(task->cpu.ebp)
    );
}

void scheduler_init() {
    for (int i = 0; i < SCHED_MAX_TASKS; i++) {
        task_pool[i].state = TASK_DEAD;
        task_pool[i].pid   = 0;
        task_pool[i].next  = NULL;
    }

    task_t *idle = scheduler_create_task("idle", task_idle, PRIORITY_IDLE);
    if (idle) {
        idle->state = TASK_READY;
    }

    vga_printf("[SCHED] Initialized | Max tasks: %d | Quantum: %d\n",
        SCHED_MAX_TASKS, SCHED_QUANTUM);
}

task_t *scheduler_create_task(const char *name, void (*entry)(), uint8_t priority) {
    if (task_count >= SCHED_MAX_TASKS) return NULL;

    task_t *task = alloc_task();
    if (!task) return NULL;

    task->pid        = next_pid++;
    task->ppid       = current_task ? current_task->pid : 0;
    task->state      = TASK_READY;
    task->priority   = priority;
    task->quantum    = SCHED_QUANTUM;
    task->sleep_ticks = 0;
    task->exit_code  = 0;
    task->next       = NULL;
    task->ipc_port   = ipc_create_port(task->pid);

    int i = 0;
    while (name[i] && i < 31) {
        task->name[i] = name[i];
        i++;
    }
    task->name[i] = '\0';

    uint32_t stack_top = (uint32_t)&task->stack[SCHED_STACK_SIZE / sizeof(uint32_t) - 1];
    stack_top &= ~0xF;

    task->cpu.esp    = stack_top;
    task->cpu.ebp    = stack_top;
    task->cpu.eip    = (uint32_t)entry;
    task->cpu.eflags = 0x202;

    queue_push(task);
    task_count++;

    vga_printf("[SCHED] Task created | PID: %d | Name: %s | Priority: %d\n",
        task->pid, task->name, task->priority);

    return task;
}

void scheduler_destroy_task(uint32_t pid) {
    for (int i = 0; i < SCHED_MAX_TASKS; i++) {
        if (task_pool[i].pid == pid) {
            task_pool[i].state = TASK_ZOMBIE;
            ipc_destroy_port(task_pool[i].ipc_port);
            task_count--;
            return;
        }
    }
}

void scheduler_schedule() {
    if (!task_queue) return;

    task_t *prev = current_task;

    if (prev && prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
        save_context(prev);
        queue_push(prev);
    }

    task_t *next = queue_pop_by_priority();
    if (!next) return;

    next->state   = TASK_RUNNING;
    next->quantum = SCHED_QUANTUM;
    current_task  = next;

    restore_context(next);
}

void scheduler_yield() {
    scheduler_schedule();
}

void scheduler_sleep(uint32_t ticks) {
    if (!current_task) return;
    current_task->state       = TASK_SLEEPING;
    current_task->sleep_ticks = ticks;
    scheduler_schedule();
}

void scheduler_block(uint32_t pid) {
    task_t *task = scheduler_get_task(pid);
    if (task) task->state = TASK_BLOCKED;
}

void scheduler_unblock(uint32_t pid) {
    task_t *task = scheduler_get_task(pid);
    if (task && task->state == TASK_BLOCKED) {
        task->state = TASK_READY;
        queue_push(task);
    }
}

task_t *scheduler_get_current() {
    return current_task;
}

task_t *scheduler_get_task(uint32_t pid) {
    for (int i = 0; i < SCHED_MAX_TASKS; i++) {
        if (task_pool[i].pid == pid && task_pool[i].state != TASK_DEAD) {
            return &task_pool[i];
        }
    }
    return NULL;
}

uint32_t scheduler_get_task_count() {
    return task_count;
}

void scheduler_dump_tasks() {
    vga_printf("[SCHED] Active tasks: %d\n", task_count);
    for (int i = 0; i < SCHED_MAX_TASKS; i++) {
        if (task_pool[i].state != TASK_DEAD) {
            vga_printf("  PID: %d | Name: %s | State: %d | Priority: %d\n",
                task_pool[i].pid,
                task_pool[i].name,
                task_pool[i].state,
                task_pool[i].priority
            );
        }
    }
}
