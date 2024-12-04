
#include "../include/stb_scheduler.h"
#include <stdlib.h>

//global scheduler instance
static STB_scheduler stbs;

/**
 * Initializes the STBS scheduler with the given tick period and maximum number of tasks.
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

    // Initialize the Zephyr timer
    // k_timer_init(&stbs.scheduler_timer, scheduler_timer_handler, NULL);
    printk("STBS Initialized\n");
}

/**
 * Adds a new task to the scheduler with the given period and priority.
 */
void STBS_AddTask(int ticks, k_tid_t task_id, int priority) {
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

            stbs.num_tasks++;
            printk("Added Task %d with period %d ticks\n", task_id, ticks);
            return;
        }
    }

    printk("Error: Failed to add task\n");
}

/**
 * Starts the STBS scheduler.
 */
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
    scheduler_table_entry entry[stbs.macro_cycle];
    // in the first tick, all taks are ready
    for(int j = 0; j< stbs.macro_cycle;j++){
        entry[j].num_tasks = 0;
        // entry[j].tick = j;
        entry[j].task_id_list = k_malloc(stbs.num_tasks*sizeof(k_tid_t));
        entry[j].total_exec_time = 0;

    }
    printk("Starting table computation\n");
    // compute the next ticks
    for(int tick = 0; tick < stbs.macro_cycle;tick++){
        printk("\ncurrent_tick: %d\n",tick);

        for(int task_idx = 0; task_idx < stbs.num_tasks; task_idx++){
            if(tick == 0 || tick % stbs.task_table[task_idx].ticks == 0 || stbs.task_table[task_idx].to_be_executed){
                printk("total_exec_time: %d, task_exec_time: %d\n",entry[tick].total_exec_time,stbs.task_table[task_idx].exec_time);
                if(stbs.task_table[task_idx].exec_time + entry[tick].total_exec_time <= stbs.tick_ms){
                    entry[tick].task_id_list[entry[tick].num_tasks] = stbs.task_table[task_idx].id;
                    entry[tick].total_exec_time += stbs.task_table[task_idx].exec_time;
                    entry[tick].num_tasks++;
                    stbs.task_table[task_idx].to_be_executed = 0;
                    printk("Added task: %d to tick: %d\n",task_idx,tick);
                }
                else{
                    if(tick+1 > stbs.macro_cycle){
                        printk("System not schedulable\n");
                        exit(1);
                    }
                    stbs.task_table[task_idx].to_be_executed = 1;
                }
            }
        }
    }
    printk("Starting STBS\n");
    int current_tick = 0; // Keeps track of the current tick count
    int count = 0; // Keeps track of the current tick count
    k_msleep(20); // let the tasks arrive at the point where they suspend themselves
    while(1){
        release_time = k_uptime_get() + stbs.tick_ms;
        for(int i = 0; i <stbs.macro_cycle;i++){
            // Check each task for activation
            current_tick++;
            printk("i: %d\n",i);
            for(int task_idx = 0; task_idx < entry[i].num_tasks; task_idx++){
                k_tid_t task_id = entry[i].task_id_list[task_idx];
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
        count++;
        // k_msleep(5000);
        if(count == 2)
            break;

        
    }
    timing_stop();
}