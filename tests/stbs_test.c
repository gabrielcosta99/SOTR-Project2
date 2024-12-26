/**
 * @file
 * @brief Static Table-Based Scheduler (STBS) Example
 *
 * This file demonstrates a simple STBS implementation for Zephyr.
 * The scheduler uses a static table to manage tasks and activates them periodically.
 */

#include <sys/_intsup.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "zephyr/kernel/thread.h"
#include <zephyr/timing/timing.h>   /* for timing services */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>


// helper functions
#include "../include/functions.h"
#include "../include/stb_scheduler.h"
#include "../include/frames.h"
#include "../include/rtdb.h"
#include "zephyr/sys/sys_io.h"

// GLOBAL

RT_db rtdb;

/************************************  UART  ***********************************/
#define SLEEP_TIME_MS 1000
#define RECEIVE_BUFF_SIZE 10
#define RECEIVE_TIMEOUT 100
#define INPUT_BUFFER_SIZE 20

/************************** THREADS ******************************/

// Configuration constants
#define TICK_MS 50          // Scheduler tick period in milliseconds
#define MAX_TASKS 15

extern const k_tid_t thread0,thread1,thread2,thread3;


/**
 * Task 0: Periodic task with period 1 tick
 * this task is responsible for updating the RTDB with the button states
 */
void task0(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        
        k_thread_suspend(thread0);
        
    }
}


/**
 * Task 1: Periodic task with period 2 ticks
 * this task is responsible for updating the led states based on the button states from the RTDB
 */
void task1(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    
    while (1) {
        k_thread_suspend(thread1);
        
    }
}

// Validate rtdb entries and reset them if they are corrupted
void task2(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread2);

    }
}

void task3(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread3);
    }
}

/*
 * defining threads
*/
K_THREAD_DEFINE(thread0 , 512, task0, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread1, 512, task1, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread2, 512, task2, NULL, NULL, NULL,1,0,0);
K_THREAD_DEFINE(thread3, 512, task3, NULL, NULL, NULL,5,0,0);

/**
 * Main function demonstrating the Static Table-Based Scheduler (STBS).
 */
int main(void) {
   
    RT_db_init(&rtdb);

    // Initialize the scheduler
    STBS_Init(TICK_MS,MAX_TASKS);

    // Add tasks with different periods
    STBS_AddTask(1, thread0, 1,10,"thread0"); // Task 1: Period = 1 ticks
    STBS_AddTask(3, thread2, 1,30,"thread2"); // Task 3: Period = 3 ticks
    STBS_AddTask(2, thread1, 1,40,"thread1"); // Task 2: Period = 2 tick
    STBS_AddTask(2, thread3, 1,10,"thread3"); // Task 2: Period = 2 tick

    // SCHEDULABLE
    // STBS_AddTask(1, thread0, 10,10,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 5,12,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 7,15,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 2,7,"thread3"); // Task 2: Period = 2 tick

    // NOT SCHEDULABLE
    // STBS_AddTask(1, thread0, 10,20,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 5,25,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 7,30,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 2,15,"thread3"); // Task 2: Period = 2 tick

    // NOT SCHEDULABLE
    // STBS_AddTask(1, thread0, 1,20,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 5,25,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 7,30,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 2,15,"thread3"); // Task 2: Period = 2 tick


    // STBS_AddTask(2, thread0, 1,50,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(4, thread1, 2,50,"thread1"); // Task 3: Period = 2 ticks
    // STBS_AddTask(6, thread2, 3,50,"thread2"); // Task 2: Period = 3 tick

    

    // Start the scheduler
    STBS_Start();
    return 0;
}
