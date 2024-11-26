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
#include "test_functions.h" // Include utility functions
#include "zephyr/kernel/thread.h"

// Configuration constants
#define MAX_TASKS 15        // Maximum number of tasks the scheduler can manage
#define TICK_MS 20          // Scheduler tick period in milliseconds
//#define K_THREAD_STACK_SIZEOF(sym) (sizeof(sym) - K_THREAD_STACK_RESERVED)

// Scheduler structure to manage tasks and execution
typedef struct {
    int tick_ms;                 // Scheduler tick duration in milliseconds
    Task task_table[MAX_TASKS];  // Array of tasks
    int max_tasks;               // Maximum number of tasks allowed
    int num_tasks;               // Current number of tasks
    int macroCycle;              // Macrocycle duration (in ticks)
    struct k_timer scheduler_timer; // Zephyr timer for periodic scheduling
} STB_scheduler;

static STB_scheduler stb; // Global scheduler instance

/**
 * Function executed by each task thread.
 * Simulates task execution by printing the task's ID.
 */
void task_function(void *id_ptr, void *unused1, void *unused2) {
    k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        printk("Task %d executing\n", task_id); // Simulate task behavior
        k_msleep(TICK_MS); // Simulate work
    }
}

/**
 * Timer handler for the scheduler.
 * Triggers every scheduler tick and evaluates which tasks need activation.
 */
void scheduler_timer_handler(struct k_timer *timer) {
    static int current_tick = 0; // Keeps track of the current tick count
    current_tick++;

    // Log when a macrocycle completes
    if (current_tick % stb.macroCycle == 0) {
        printk("End of macrocycle at tick %d\n", current_tick);
    }

    // Check each task for activation
    for (int i = 0; i < stb.num_tasks; i++) {
        if (current_tick == stb.task_table[i].next_activation) {
            printk("Activating Task %d\n", stb.task_table[i].id);
            stb.task_table[i].next_activation += stb.task_table[i].ticks; // Schedule next activation
        }
    }
}

/**
 * Initializes the Static Table-Based Scheduler (STBS).
 */
void STBS_Init(int tick_ms, int max_tasks) {
    stb.tick_ms = tick_ms;
    stb.max_tasks = max_tasks;
    stb.num_tasks = 0;

    // Initialize task table
    for (int i = 0; i < max_tasks; i++) {
        stb.task_table[i].id = -1; // Mark as unused
        stb.task_table[i].ticks = 0;
        stb.task_table[i].next_activation = -1;
    }

    // Initialize the Zephyr timer
    k_timer_init(&stb.scheduler_timer, scheduler_timer_handler, NULL);
    printk("STBS Initialized\n");
}

/**
 * Adds a new task to the scheduler.
 */
void STBS_AddTask(int ticks, k_tid_t task_id, int priority) {
    if (stb.num_tasks >= stb.max_tasks) {
        printk("Error: Maximum task limit reached\n");
        return;
    }

    // Find an empty slot in the task table
    for (int i = 0; i < stb.max_tasks; i++) {
        if (stb.task_table[i].id == -1) {
            stb.task_table[i].ticks = ticks;
            stb.task_table[i].next_activation = ticks; // Set initial activation
            stb.task_table[i].priority = priority;
            stb.task_table[i].id = task_id;

            // Allocate memory for thread stack
            //char *stack_area = k_malloc(K_THREAD_STACK_SIZEOF(512));
            //if (stack_area == NULL) {
            //    printk("Error: Failed to allocate thread stack\n");
            //    return;
            //}

            // Create and start the thread
            //stb.task_table[i].thread_id = k_thread_create(
            //    &stb.task_table[i].thread,
            //    stack_area, 512,
            //    task_function,
            //    &stb.task_table[i].id, NULL, NULL,
            //    priority,
            //    0, K_NO_WAIT);
           
            

            stb.num_tasks++;
            printk("Added Task %d with period %d ticks\n", task_id, ticks);
            break;
        }
    }
}

/**
 * Starts the scheduler by activating the timer and calculating the macrocycle.
 */
void STBS_Start() {
    // Calculate macrocycle as the LCM of all task periods
    int task_ticks[MAX_TASKS];
    for (int i = 0; i < stb.num_tasks; i++) {
        task_ticks[i] = stb.task_table[i].ticks;
        printk("Thread id: %d\n", stb.task_table[i].thread_id);
    }
    stb.macroCycle = lcm_array(task_ticks, stb.num_tasks);
    printk("Macrocycle calculated: %d ticks\n", stb.macroCycle);

    printk("Starting STBS\n");
    k_timer_start(&stb.scheduler_timer, K_MSEC(stb.tick_ms), K_MSEC(stb.tick_ms));
    // thread id
    
}

/*
 * defining threads
*/
K_THREAD_DEFINE(thread0 , 512, task_function, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread1, 512, task_function, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread2, 512, task_function, NULL, NULL, NULL,5,0,0);

/**
 * Main function demonstrating the Static Table-Based Scheduler (STBS).
 */
void main(void) {
    printk("Zephyr STBS Example\n");

    // Initialize the scheduler
    STBS_Init(TICK_MS, MAX_TASKS);

    // Add tasks with different periods
    STBS_AddTask(2, thread0, 1); // Task 1: Period = 2 ticks
    STBS_AddTask(1, thread1, 2); // Task 2: Period = 1 tick
    STBS_AddTask(3, thread2, 3); // Task 3: Period = 3 ticks

    

    // Start the scheduler
    STBS_Start();
}
