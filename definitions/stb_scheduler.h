
#include "functions.h"

#ifndef STB_SCHEDULER_H
#define STB_SCHEDULER_H


// Scheduler structure to manage tasks and execution
typedef struct {
    int tick_ms;                 // Scheduler tick duration in milliseconds
    Task *task_table;  // Array of tasks
    int max_tasks;               // Maximum number of tasks allowed
    int num_tasks;               // Current number of tasks
    int macro_cycle;              // Macrocycle duration (in ticks)
} STB_scheduler;

typedef struct{
    // int tick;               // current tick
    k_tid_t *task_id_list;  // ids of the tasks that will execute in this tick
    int num_tasks;          // number of tasks to execute in this tick
    int total_exec_time;    // total time it takes for the tasks to execute
}scheduler_table_entry;

void STBS_Init(int tick_ms, int max_tasks);
void STBS_AddTask(int ticks, k_tid_t task_id, int priority);
void STBS_Start();

#endif