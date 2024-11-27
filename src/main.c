/**
 * @file
 * @brief Static Table-Based Scheduler (STBS) Example
 *
 * This file demonstrates a simple STBS implementation for Zephyr.
 * The scheduler uses a static table to manage tasks and activates them periodically.
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include "zephyr/kernel/thread.h"
#include <zephyr/timing/timing.h>   /* for timing services */
#include "stb_scheduler.h"

// Configuration constants
#define TICK_MS 200          // Scheduler tick period in milliseconds
#define MAX_TASKS 15

extern const k_tid_t thread0,thread1,thread2,thread3;

void task0(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread0);
        printk("Task0 executing %d\n",thread0); // Simulate task behavior
    }
}

void task1(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread1);
        printk("Task1 executing %d\n",thread1); // Simulate task behavior
        // k_msleep(TICK_MS); // Simulate work
    }
}

void task2(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread2);
        printk("Task2 executing %d\n",thread2); // Simulate task behavior
        // k_msleep(TICK_MS); // Simulate work
    }
}

void task3(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread3);
        printk("Task3 executing %d\n",thread3); // Simulate task behavior
        // k_msleep(TICK_MS); // Simulate work
    }
}

/**
 * Timer handler for the scheduler.
 * Triggers every scheduler tick and evaluates which tasks need activation.
 */
// void scheduler_timer_handler(struct k_timer *timer) {
//     static int current_tick = 0; // Keeps track of the current tick count
//     current_tick++;

//     // Log when a macrocycle completes
//     if (current_tick % stb.macroCycle == 0) {
//         printk("End of macrocycle at tick %d\n", current_tick);
//     }

//     // Check each task for activation
//     for (int i = 0; i < stb.num_tasks; i++) {
//         if (current_tick == stb.task_table[i].next_activation) {
//             printk("Activating Task %d\n", stb.task_table[i].id);
//             stb.task_table[i].next_activation += stb.task_table[i].ticks; // Schedule next activation
//             k_thread_resume(stb.task_table[i].id);
            
//         }
//     }
// }

/**
 * Initializes the Static Table-Based Scheduler (STBS).
 */


/*
 * defining threads
*/
K_THREAD_DEFINE(thread0 , 512, task0, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread1, 512, task1, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread2, 512, task2, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread3, 512, task3, NULL, NULL, NULL,5,0,0);

/**
 * Main function demonstrating the Static Table-Based Scheduler (STBS).
 */
void main(void) {
    printk("Zephyr STBS Example\n");

    // Initialize the scheduler
    STBS_Init(TICK_MS,MAX_TASKS);

    // Add tasks with different periods
    // STBS_AddTask(1, thread0, 1,40); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 1,120); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 1,160); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 1,40); // Task 2: Period = 2 tick

    // STBS_AddTask(1, thread0, 1,20); // Task 1: Period = 1 ticks
    // STBS_AddTask(2, thread1, 1,20); // Task 2: Period = 2 tick
    // STBS_AddTask(3, thread2, 1,20); // Task 3: Period = 3 ticks

    STBS_AddTask(1, thread0, 10,40); // Task 1: Period = 1 ticks
    STBS_AddTask(3, thread2, 5,50); // Task 3: Period = 3 ticks
    STBS_AddTask(2, thread1, 7,60); // Task 2: Period = 2 tick
    STBS_AddTask(2, thread3, 2,30); // Task 2: Period = 2 tick

    // Start the scheduler
    STBS_Start();
}
