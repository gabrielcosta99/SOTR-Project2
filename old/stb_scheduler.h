#include "functions.h"


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
    Task *tasks;  // ids of the tasks that will execute in this tick
    int num_tasks;          // number of tasks to execute in this tick
    int total_exec_time;    // total time it takes for the tasks to execute
}scheduler_table_entry;


static STB_scheduler stbs; // Global scheduler instance
static scheduler_table_entry *entry;


void STBS_Init(int tick_ms, int max_tasks) {
    stbs.tick_ms = tick_ms;
    stbs.max_tasks = max_tasks;
    stbs.num_tasks = 0;
    stbs.task_table = k_malloc(max_tasks * sizeof(Task));
    // Initialize task table
    for (int i = 0; i < max_tasks; i++) {
        stbs.task_table[i].id = -1; // Mark as unused
        stbs.task_table[i].ticks = 0;
        stbs.task_table[i].next_activation = -1;
    }

    // Initialize the Zephyr timer
    // k_timer_init(&stbs.scheduler_timer, scheduler_timer_handler, NULL);
    printk("STBS Initialized\n");
}

/**
 * Adds a new task to the scheduler.
 */
void STBS_AddTask(int ticks, k_tid_t task_id, int priority, int execution_time, char *name) {
    if (stbs.num_tasks >= stbs.max_tasks) {
        printk("Error: Maximum task limit reached\n");
        return;
    }

    // Find an empty slot in the task table
    for (int i = 0; i < stbs.max_tasks; i++) {
        if (stbs.task_table[i].id == -1) {
            stbs.task_table[i].ticks = ticks;
            stbs.task_table[i].next_activation = ticks; // Set initial activation
            stbs.task_table[i].priority = priority;
            stbs.task_table[i].id = task_id;
            stbs.task_table[i].exec_time = execution_time;
            stbs.task_table[i].to_be_executed = 0;
            stbs.task_table[i].name = name;

            // Allocate memory for thread stack
            //char *stack_area = k_malloc(K_THREAD_STACK_SIZEOF(512));
            //if (stack_area == NULL) {
            //    printk("Error: Failed to allocate thread stack\n");
            //    return;
            //}

            // Create and start the thread
            //stbs.task_table[i].thread_id = k_thread_create(
            //    &stbs.task_table[i].thread,
            //    stack_area, 512,
            //    task_function,
            //    &stbs.task_table[i].id, NULL, NULL,
            //    priority,
            //    0, K_NO_WAIT);
           
            

            stbs.num_tasks++;
            printk("Added Task %d with period %d ticks\n", task_id, ticks);
            break;
        }
    }
}

/**
 * Starts the scheduler by activating the timer and calculating the macrocycle.
 */
// void STBS_Start() {
//     int64_t fin_time=0, release_time=0;     /* Timing variables to control task periodicity */

//     // Calculate macrocycle as the LCM of all task periods
//     int task_ticks[stbs.max_tasks];
//     for (int i = 0; i < stbs.num_tasks; i++) {
//         task_ticks[i] = stbs.task_table[i].ticks;
//         printk("Thread id: %d\n", stbs.task_table[i].id);
//     }
//     stbs.macro_cycle = lcm_array(task_ticks, stbs.num_tasks);
//     printk("Macrocycle calculated: %d ticks\n", stbs.macro_cycle);

//     printk("Starting STBS\n");
//     release_time = k_uptime_get() + stbs.tick_ms;
//     int current_tick = 0; // Keeps track of the current tick count
//     while(1){
//         current_tick++;

//         // Log when a macrocycle completes
//         // if (current_tick % stbs.macroCycle == 0) {
//         //     printk("End of macrocycle at tick %d\n", current_tick);
//         // }

//         // Check each task for activation
//         for (int i = 0; i < stbs.num_tasks; i++) {
//             if (current_tick == stbs.task_table[i].next_activation) {
//                 printk("Activating Task %d\n", stbs.task_table[i].id);
//                 stbs.task_table[i].next_activation += stbs.task_table[i].ticks; // Schedule next activation
//                 k_thread_resume(stbs.task_table[i].id);
                
//             }
//         }


//         fin_time = k_uptime_get();
//         if( fin_time < release_time) {
//             k_msleep(release_time - fin_time);
//             release_time += stbs.tick_ms;

//         }
//     }
//     timing_stop();
// }

void STBS_Start() {
    int64_t fin_time=0, release_time=0;     /* Timing variables to control task periodicity */

    // Calculate macrocycle as the LCM of all task periods
    int task_ticks[stbs.max_tasks];
    for (int i = 0; i < stbs.num_tasks; i++) {
        task_ticks[i] = stbs.task_table[i].ticks;
        printk("Thread id: %d\n", stbs.task_table[i].id);
    }
    stbs.macro_cycle = lcm_array(task_ticks, stbs.num_tasks);
    printk("Macrocycle calculated: %d ticks\n", stbs.macro_cycle);
    

    // Calculate the ticks in which each task will execute
    qsort(stbs.task_table,stbs.num_tasks,sizeof(Task),compare_tasks);

    // debug
    for (int i = 0; i < stbs.num_tasks; i++) {
        printk("prio: %d, ticks: %d\n", stbs.task_table[i].priority, stbs.task_table[i].ticks);
    }

    // create the table with the times in which each task will execute
    // in the first tick, all taks are ready
    // entry = k_malloc(stbs.macro_cycle * sizeof(scheduler_table_entry));
    entry = k_malloc(stbs.macro_cycle * sizeof(scheduler_table_entry));
    if (!entry) {
        printk("Failed to allocate scheduler table\n");
        return;
    }

    for(int j = 0; j< stbs.macro_cycle;j++){
        entry[j].num_tasks = 0;
        // entry[j].tick = j;
        entry[j].tasks = k_malloc(stbs.num_tasks*sizeof(Task));
        if (!entry[j].tasks) {
            printk("Failed to allocate memory for tasks at tick %d\n", j);
            // Free previously allocated entries to prevent memory leaks
            for (int k = 0; k < j; k++) {
                k_free(entry[k].tasks);
            }
            k_free(entry);
            return;
        }
        entry[j].total_exec_time = 0;

    }
    // for(int j = 0; j<stbs.num_tasks; j++){
    //     if(stbs.task_table[j].exec_time+entry[0].total_exec_time <= stbs.tick_ms){
    //         entry[0].task_id_list[entry[0].num_tasks] = stbs.task_table[j].id;
    //         entry[0].total_exec_time += stbs.task_table[j].exec_time;
    //         entry[0].num_tasks++;
    //     }
    //     else{
    //         entry[0+1].task_id_list[entry[0+1].num_tasks] = stbs.task_table[j].id;
    //         entry[0+1].total_exec_time += stbs.task_table[j].exec_time;
    //         entry[0+1].num_tasks;
    //     }
            
    // }
    printk("Starting table computation\n");
    // compute the next ticks
    for(int tick = 0; tick < stbs.macro_cycle;tick++){
        // printk("\ncurrent_tick: %d\n",tick);

        for(int task_idx = 0; task_idx < stbs.num_tasks; task_idx++){
            if(tick == 0 || tick % stbs.task_table[task_idx].ticks == 0 || stbs.task_table[task_idx].to_be_executed){
                // printk("total_exec_time: %d, task_exec_time: %d\n",entry[tick].total_exec_time,stbs.task_table[task_idx].exec_time);
                if(stbs.task_table[task_idx].exec_time + entry[tick].total_exec_time <= stbs.tick_ms){
                    entry[tick].tasks[entry[tick].num_tasks] = stbs.task_table[task_idx];
                    entry[tick].total_exec_time += stbs.task_table[task_idx].exec_time;
                    entry[tick].num_tasks++;
                    stbs.task_table[task_idx].to_be_executed = 0;
                    // printk("Added '%s' to tick: %d\n",stbs.task_table[task_idx].name,tick);
                }
                else{
                    if(tick+1 > stbs.macro_cycle){
                        printk("System not schedulable\n");
                        for (int i = 0; i < stbs.macro_cycle; i++) {
                            k_free(entry[i].tasks);
                        }
                        k_free(entry);
                        return;
                    }
                    stbs.task_table[task_idx].to_be_executed = 1;
                    // entry[tick+1].task_id_list[entry[tick+1].num_tasks] = stbs.task_table[task_idx].id;
                    // entry[tick+1].total_exec_time += stbs.task_table[task_idx].exec_time;
                    // entry[tick+1].num_tasks++;
                    // printk("Added task: %d to tick: %d\n",task_idx,tick+1);

                }
            }
            
            
        }
    }
    STBS_print_content();

    // for (int i = 0; i < stbs.num_tasks; i++) {
    //     entry[0].task_id_list
    //     for(int j = 1; j<stbs.macro_cycle;j++){
    //         if(j%)
    //     }

    // }

    printk("Starting STBS\n");
    int current_tick = 0; // Keeps track of the current tick count
    int count = 0; // Keeps track of the current tick count
    k_msleep(20); // let the tasks arrive at the point where they suspend themselves
    while(1){

        // Log when a macrocycle completes
        // if (current_tick % stbs.macroCycle == 0) {
        //     printk("End of macrocycle at tick %d\n", current_tick);
        // }
        release_time = k_uptime_get() + stbs.tick_ms;
        for(int i = 0; i <stbs.macro_cycle;i++){
            // Check each task for activation
            current_tick++;
            // printk("i: %d\n",i);
            for(int task_idx = 0; task_idx < entry[i].num_tasks; task_idx++){
                k_tid_t task_id = entry[i].tasks[task_idx].id;
                // printk("Activating Task %d\n", task_id);
                k_thread_resume(task_id);
            }
            // for (int j = 0; j < stbs.num_tasks; j++) {
            //     if (current_tick == stbs.task_table[j].next_activation) {
            //         printk("Activating Task %d\n", stbs.task_table[j].id);
            //         stbs.task_table[j].next_activation += stbs.task_table[j].ticks; // Schedule next activation
            //         k_thread_resume(stbs.task_table[j].id);
                    
            //     }

            // }
            fin_time = k_uptime_get();
            if( fin_time < release_time) {
                k_msleep(release_time - fin_time);
                release_time += stbs.tick_ms;
            }
            // for (int j = 0; j < stbs.num_tasks; j++) {
            //     if (current_tick == stbs.task_table[j].next_activation) {
            //         printk("Activating Task %d\n", stbs.task_table[j].id);
            //         stbs.task_table[j].next_activation += stbs.task_table[j].ticks; // Schedule next activation
            //         k_thread_resume(stbs.task_table[j].id);
                    
            //     }

            // }
            // fin_time = k_uptime_get();
            // if( fin_time < release_time) {
            //     k_msleep(release_time - fin_time);
            //     release_time += stbs.tick_ms;
            // }

        }
        current_tick = 0;
        // count++;
        // k_msleep(5000);
        // if(count == 2)
        //     break;

        
    }
    timing_stop();
}


// #include <stdio.h> // Needed for snprintf if used

void STBS_print_content() {
    // Constants for table formatting
    const char *table_header =  "| Tick | Task Name      | Execution Time | Priority |\n";
    const char *table_divider = "+------+----------------+----------------+----------+\n";

    // Check for empty scheduler table
    if (stbs.macro_cycle == 0) {
        printk("Scheduler table is empty.\n");
        return;
    }

    printk("Printing scheduler table contents:\n");
    printk("%s", table_divider);
    printk("%s", table_header);
    printk("%s", table_divider);

    for (int tick = 0; tick < stbs.macro_cycle; tick++) {
        if (entry[tick].num_tasks == 0) {
            // Print empty tick row
            printk("| %4d | %-14s | %-14s | %-8s |\n", tick, "No tasks", "-", "-");
            continue;
        }

        for (int task_idx = 0; task_idx < entry[tick].num_tasks; task_idx++) {
            Task current_task = entry[tick].tasks[task_idx];
            
            // Print task row
            printk("| %4d | %-14s | %-14d | %-8d |\n", 
                tick, 
                current_task.name, 
                current_task.exec_time,  // Replace with actual field
                current_task.priority    // Replace with actual field
            );
        }

        // Print total execution time row for this tick
        printk("| %4s | %-14s | %-14d | %-8s |\n", 
            "-", 
            "Total Time", 
            entry[tick].total_exec_time, 
            "-"
        );
        printk("%s", table_divider);
    }
}

void STBS_destroy(){

}
