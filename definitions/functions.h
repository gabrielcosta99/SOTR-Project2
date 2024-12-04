#include "zephyr/kernel/thread.h"
#include <zephyr/kernel.h>

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define NS_IN_SEC 1000000000L



typedef struct {
    k_tid_t id;                      // Unique identifier for the task
    int ticks;                   // Task's period in ticks (relative to the scheduler's tick)
    int next_activation;         // Tick count for the next activation of the task
    int priority;                // Priority level of the thread (lower values = higher priority in Zephyr)
    int exec_time;              // execution time in ms
    int to_be_executed;         // flag that says if the task was supposed to be executed in a previous tick, but it didnt have space available
} Task;


int compare_tasks(const void *a,const void *b);
int gcd(int a, int b);
int lcm(int a, int b);
int lcm_array(int arr[], int n);

#endif