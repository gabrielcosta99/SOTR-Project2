
#include "../include/stb_scheduler.h"
#include "../include/functions.h"

#include <stdlib.h>

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
            

            stbs.num_tasks++;
            printk("Added Task %d with period %d ticks\n", task_id, ticks);
            break;
        }
    }
}


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


                }
            }
            
            
        }
    }
    STBS_print_content();
    printk("Starting STBS\n");
    int current_tick = 0; // Keeps track of the current tick count
    int count = 0; // Keeps track of the current tick count
    k_msleep(20); // let the tasks arrive at the point where they suspend themselves
    while(1){


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

            fin_time = k_uptime_get();
            if( fin_time < release_time) {
                k_msleep(release_time - fin_time);
                release_time += stbs.tick_ms;
            }

        }
        current_tick = 0;

        
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


// testing functions
int STBS_GetNumTasks(void) {
    return stbs.num_tasks;
}

int STBS_GetTickMs(void) {
    return stbs.tick_ms;
}

const Task* STBS_GetTaskTable(void) {
    return stbs.task_table;
}

int STBS_GetMacroCycle(void) {
    return stbs.macro_cycle;
}

