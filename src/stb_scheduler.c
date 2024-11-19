#include <stdlib.h>
#include <stdio.h>
#include<unistd.h>
#include "functions.h"

// Inits the STBS system, including creating eventual system tasks, initializing variables, etc.

// the "Task" structure is defined in "functions.h"

typedef struct{
    int tick;
    Task *task_table;
    int max_tasks;
    int num_tasks;
    int macroCycle;
} STB_scheduler;

static STB_scheduler stb;

void STBS_Init(int tick_ms, int max_tasks){
    stb.tick = tick_ms; 
    stb.max_tasks = max_tasks;

    // initialize the "task_table" and each task
    stb.task_table = malloc(max_tasks*sizeof(Task));
    for(int i = 0; i< max_tasks; i++){
        stb.task_table[i].id = -1;
        stb.task_table[i].period = 0;
        stb.task_table[i].next_activation = -1;
    }
    
    printf("Initialized STBS\n");
}

// UNFINISHED
// Starts the STBS scheduler
void STBS_Start(){
    if(stb.num_tasks == 0){
        printf("There are no tasks to start\n");
        return;
    }

    // calculate the least common multiple of the tasks period (to get the "macro-cycle")
    int periods[stb.num_tasks];
    for(int i = 0; i< stb.num_tasks;i++){
        periods[i] = stb.task_table[i].period;
    }
    stb.macroCycle = lcm_array(periods,stb.num_tasks);
    printf("macroCycle: %d\n",stb.macroCycle);

    // schedule each task
    // Task ordered_task_list[stb.num_tasks];
    // for(int i = 0; i< stb.num_tasks; i++){
    //     ordered_task_list[i] = stb.task_table[i];
    // }
    //      start by sorting the tasks by their period
    qsort(stb.task_table,stb.num_tasks,sizeof(Task),compare_tasks);

    printf("Started STBS\n");
    int current_tick = 0;
    while(1){
        current_tick+=stb.tick;
        for (int i = 0; i < stb.num_tasks; i++) {
            if (stb.task_table[i].id != -1 && current_tick == stb.task_table[i].next_activation) {
                // Activate task
                stb.task_table[i].next_activation += stb.task_table[i].period;

                // Invoke the task 
            }
        }
        usleep(stb.tick*1000);
    }

}

// Stops the STBS scheduler
// void STBS_Stop(){
    
// }

//  Adds a task to the STBS scheduler, with a period of period_ticks. task_id is a suitable
// identifier (its nature depends on the method used to control the activation of the tasks)
void STBS_AddTask(int period_ticks, int task_id, void (*task_ptr)(int)){
    // leaving it like this for now because we might not need the list of tasks to NOT have a gap.
    for(int i = 0; i<stb.max_tasks;i++){
        if(stb.task_table[i].id == -1){
            stb.task_table[i].id = task_id;
            stb.task_table[i].period = period_ticks;
            stb.task_table[i].next_activation = period_ticks;
            stb.task_table[i].task_ptr = task_ptr;
            stb.num_tasks++;
            break;
        }
    }

    printf("Added task with id: %d\n",task_id);
}


// Removes a task identified by task_id from the table
void STBS_RemoveTask(int task_id){
    int i;
    // find the position of the task with the id "task_id" on the list
    for(i = 0; task_id!=stb.task_table[i].id;i++){
        // if we passed through all the tasks, then the id is invalid
        if(i>=stb.num_tasks){
            printf("Invalid id: %d\n",task_id);
            return ;
        }
    }
    // If we arrived here, then we found the task. Now we "delete it"
    stb.task_table[i].id = -1;
    stb.task_table[i].period = 0;
    // if we remove a task from the middle of the list, we need to close the gap it creates
    for(int j = i; j<stb.num_tasks-1;j++){
        stb.task_table[j] = stb.task_table[j+1];
    }
    stb.num_tasks--;
}


//  Used inside a task body, terminates a task’s job and makes the task wait for its next
// activation, triggered by the STBS scheduler.
// void STBS_WaitPeriod(){

// }


int main(void){
    STBS_Init(5,15);
    STBS_AddTask(10,1,&fun);
    STBS_AddTask(5,2,&fun);
    STBS_AddTask(15,3,&fun);
    STBS_Start();
    // for(int i = 0; i<stb.num_tasks;i++){
    //     printf("task period: %d\n",stb.task_table[i].period);
    // }
    // STBS_RemoveTask(5);
    // for(int i = 0; i<stb.num_tasks;i++){
    //     printf("task period: %d\n",stb.task_table[i].period);
    // }
    return 0;
}
