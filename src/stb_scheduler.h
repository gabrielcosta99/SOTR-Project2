#include "functions.h"


// Scheduler structure to manage tasks and execution
typedef struct {
    int tick_ms;                 // Scheduler tick duration in milliseconds
    Task *task_table;  // Array of tasks
    int max_tasks;               // Maximum number of tasks allowed
    int num_tasks;               // Current number of tasks
    int macroCycle;              // Macrocycle duration (in ticks)
} STB_scheduler;

static STB_scheduler stb; // Global scheduler instance

void STBS_Init(int tick_ms, int max_tasks) {
    stb.tick_ms = tick_ms;
    stb.max_tasks = max_tasks;
    stb.num_tasks = 0;
    stb.task_table = k_malloc(max_tasks * sizeof(Task));
    // Initialize task table
    for (int i = 0; i < max_tasks; i++) {
        stb.task_table[i].id = -1; // Mark as unused
        stb.task_table[i].ticks = 0;
        stb.task_table[i].next_activation = -1;
    }

    // Initialize the Zephyr timer
    // k_timer_init(&stb.scheduler_timer, scheduler_timer_handler, NULL);
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
    int64_t fin_time=0, release_time=0;     /* Timing variables to control task periodicity */

    // Calculate macrocycle as the LCM of all task periods
    int task_ticks[stb.max_tasks];
    for (int i = 0; i < stb.num_tasks; i++) {
        task_ticks[i] = stb.task_table[i].ticks;
        printk("Thread id: %d\n", stb.task_table[i].id);
    }
    stb.macroCycle = lcm_array(task_ticks, stb.num_tasks);
    printk("Macrocycle calculated: %d ticks\n", stb.macroCycle);

    printk("Starting STBS\n");
    release_time = k_uptime_get() + stb.tick_ms;
    int current_tick = 0; // Keeps track of the current tick count
    while(1){
        current_tick++;

        // Log when a macrocycle completes
        // if (current_tick % stb.macroCycle == 0) {
        //     printk("End of macrocycle at tick %d\n", current_tick);
        // }

        // Check each task for activation
        for (int i = 0; i < stb.num_tasks; i++) {
            if (current_tick == stb.task_table[i].next_activation) {
                printk("Activating Task %d\n", stb.task_table[i].id);
                stb.task_table[i].next_activation += stb.task_table[i].ticks; // Schedule next activation
                k_thread_resume(stb.task_table[i].id);
                
            }
        }


        fin_time = k_uptime_get();
        if( fin_time < release_time) {
            k_msleep(release_time - fin_time);
            release_time += stb.tick_ms;

        }
    }
    timing_stop();
    // k_timer_start(&stb.scheduler_timer, K_MSEC(stb.tick_ms), K_MSEC(stb.tick_ms));
    // thread id
    
}