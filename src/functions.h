typedef struct {
    int id;
    int period;
    int next_activation;
} Task;


int compare_tasks(const void *a,const void *b){
    const Task *taskA = (const Task *)a;
    const Task *taskB = (const Task *)b;

    return taskA->period - taskB->period; // Sort in ascending order of period
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