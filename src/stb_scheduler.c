// Inits the STBS system, including creating eventual system tasks, initializing variables, etc.
typedef struct {
    int id;
    int period;
} task;


typedef struct{
    int tick;
    task *task_table;
    int max_tasks;
} stb_scheduler;


static stb_scheduler stb;
void STBS_Init(tick_ms, max_tasks){
    stb.tick = tick_ms; 
    stb.max_tasks = max_tasks;

    // initialize the "task_table" and each task
    stb.task_table = malloc(max_tasks*sizeof(task));
    for(int i = 0; i< max_tasks; i++){
        stb.task_table[i].id = -1;
        stb.task_table[i].period = 0;
    }
}

// Starts the STBS scheduler
void STBS_Start(){

}

// Stops the STBS scheduler
void STBS_Stop(){
    
}

//  Adds a task to the STBS scheduler, with a period of period_ticks. task_id is a suitable
// identifier (its nature depends on the method used to control the activation of the tasks)
void STBS_AddTask(period_ticks, task_id){
    for(int i = 0; i<stb.max_tasks;i++){
        if(stb.task_table[i].id == -1){
            stb.task_table[i].id = task_id;
            stb.task_table[i].period = period_ticks;
        }
    }
}


// Removes a task identified by task_id from the table
void STBS_RemoveTask(task_id){
    for(int i = 0; i<stb.max_tasks;i++){
        if(stb.task_table[i].id == task_id){
            stb.task_table[i].id = -1;
            stb.task_table[i].period = 0;
        }
    }
}


//  Used inside a task body, terminates a task’s job and makes the task wait for its next
// activation, triggered by the STBS scheduler.
void STBS_WaitPeriod(){

}


