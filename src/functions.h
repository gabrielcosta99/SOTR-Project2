#define NS_IN_SEC 1000000000L



typedef struct {
    k_tid_t id;                      // Unique identifier for the task
    int ticks;                   // Task's period in ticks (relative to the scheduler's tick)
    int next_activation;         // Tick count for the next activation of the task
    // struct k_thread thread;      // Zephyr thread object to manage the task's execution
    // k_tid_t thread_id;           // Identifier for the thread
    int priority;                // Priority level of the thread (lower values = higher priority in Zephyr)
    int exec_time;              // execution time in ms
    int to_be_executed;         // flag that says if the task was supposed to be executed in a previous tick, but it didnt have space available
    char *name;
} Task;


int compare_tasks(const void *a,const void *b){
    const Task *taskA = (const Task *)a;
    const Task *taskB = (const Task *)b;

    if(taskA->priority != taskB->priority)  // Sort by priority
        return taskA->priority - taskB->priority;

    // If the priority is the same
    return taskA->ticks - taskB->ticks; // Sort by period
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Function to calculate LCM of two numbers
int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

// Function to calculate LCM of an array of numbers
int lcm_array(int arr[], int n) {
    int result = arr[0];
    for (int i = 1; i < n; i++) {
        result = lcm(result, arr[i]);
    }
    return result;
}

