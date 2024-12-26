
#include "../include/stb_scheduler.h"
#include "../include/functions.h"

#include <stdlib.h>

static STB_scheduler stbs; // Global scheduler instance
static scheduler_table_entry *entry;

/**
 * @brief Initializes the STB scheduler.
 * @param tick_ms Tick duration in milliseconds.
 * @param max_tasks Maximum number of tasks that can be scheduled.
 */
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

    printk("STBS Initialized\n");
}

/**
 * @brief Adds a new task to the scheduler.
 * @param ticks Periodicity of the task in ticks.
 * @param task_id Task identifier (e.g., thread ID).
 * @param priority Task priority level.
 * @param execution_time Task execution time in ticks.
 * @param name Task name.
 */


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
            stbs.task_table[i].delay_count = 0;         // CHANGED
            stbs.task_table[i].name = name;
            

            stbs.num_tasks++;
            printk("Added Task %s with period %d ticks\n",name, ticks);
            break;
        }
    }
}

/**
 * @brief Schedules all the registered tasks and starts the scheduler .
 */
void STBS_Start() {
    int64_t fin_time=0, release_time=0;     /* Timing variables to control task periodicity */

    // Calculate macrocycle as the LCM of all task periods
    int task_ticks[stbs.max_tasks];
    for (int i = 0; i < stbs.num_tasks; i++) {
        task_ticks[i] = stbs.task_table[i].ticks;
    }
    stbs.macro_cycle = lcm_array(task_ticks, stbs.num_tasks);

    // Calculate the ticks in which each task will execute
    qsort(stbs.task_table,stbs.num_tasks,sizeof(Task),compare_tasks);

    // create the table with the times in which each task will execute
    // in the first tick, all taks are ready
    entry = k_malloc(stbs.macro_cycle * sizeof(scheduler_table_entry));
    if (!entry) {
        printk("Failed to allocate scheduler table\n");
        return;
    }

    // initialize table entries
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
    // create the actual table
    for(int tick = 0; tick < stbs.macro_cycle;tick++){

        for(int task_idx = 0; task_idx < stbs.num_tasks; task_idx++){
            if(tick == 0 || tick % stbs.task_table[task_idx].ticks == 0 || stbs.task_table[task_idx].to_be_executed){
                if(stbs.task_table[task_idx].exec_time + entry[tick].total_exec_time <= stbs.tick_ms){
                    entry[tick].tasks[entry[tick].num_tasks] = stbs.task_table[task_idx];
                    entry[tick].total_exec_time += stbs.task_table[task_idx].exec_time;
                    entry[tick].num_tasks++;
                    stbs.task_table[task_idx].to_be_executed = 0;
                    stbs.task_table[task_idx].delay_count = 0;    // CHANGED
                }
                else{
                    stbs.task_table[task_idx].delay_count += 1; // CHANGED
                    if(tick+1 > stbs.macro_cycle 
                     || stbs.task_table[task_idx].delay_count >= stbs.task_table[task_idx].ticks){ // CHANGED
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
    k_msleep(20); // let the tasks arrive at the point where they suspend themselves
    while(1){
        release_time = k_uptime_get() + stbs.tick_ms;
        for(int i = 0; i <stbs.macro_cycle;i++){
            current_tick++;
            for(int task_idx = 0; task_idx < entry[i].num_tasks; task_idx++){
                k_tid_t task_id = entry[i].tasks[task_idx].id;
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

/**
 * @brief Prints the contents of the scheduler table.
 */
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
                current_task.exec_time,  
                current_task.priority    
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
    for (int i = 0; i < stbs.macro_cycle; i++) {
        k_free(entry[i].tasks);
    }
    k_free(entry);

     // Free the task table and reset fields
    if (stbs.task_table) {
        k_free(stbs.task_table);
        stbs.task_table = NULL;
    }
    stbs.num_tasks = 0;
    stbs.macro_cycle = 0;
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

