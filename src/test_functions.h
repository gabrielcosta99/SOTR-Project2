#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "zephyr/kernel/thread.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// Constants
#define NS_IN_SEC 1000000000L  // Number of nanoseconds in a second (useful for time calculations)

// Structure to represent a Task
typedef struct {
    k_tid_t id;                      // Unique identifier for the task
    int ticks;                   // Task's period in ticks (relative to the scheduler's tick)
    int next_activation;         // Tick count for the next activation of the task
    struct k_thread thread;      // Zephyr thread object to manage the task's execution
    k_tid_t thread_id;           // Identifier for the thread
    int priority;                // Priority level of the thread (lower values = higher priority in Zephyr)
} Task;

// Inline Functions

/**
 * Compares two tasks based on their period (ticks).
 * 
 * Used for sorting tasks in ascending order of their period.
 *
 * @param a Pointer to the first task
 * @param b Pointer to the second task
 * @return Negative value if a < b, zero if a == b, positive value if a > b
 */
static inline int compare_tasks(const void *a, const void *b) {
    const Task *taskA = (const Task *)a;
    const Task *taskB = (const Task *)b;
    return taskA->ticks - taskB->ticks; // Sort by period (ascending order)
}

/**
 * Calculates the Greatest Common Divisor (GCD) of two integers.
 * 
 * Useful for determining relationships between task periods.
 *
 * @param a First integer
 * @param b Second integer
 * @return The GCD of the two integers
 */
static inline int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Calculates the Least Common Multiple (LCM) of two integers.
 * 
 * Used to compute the macrocycle of the scheduler.
 *
 * @param a First integer
 * @param b Second integer
 * @return The LCM of the two integers
 */
static inline int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

/**
 * Calculates the Least Common Multiple (LCM) of an array of integers.
 * 
 * Helps determine the scheduler's macrocycle by finding the LCM of all task periods.
 *
 * @param arr Array of integers representing task periods
 * @param n Number of elements in the array
 * @return The LCM of the array (macrocycle duration)
 */
static inline int lcm_array(int arr[], int n) {
    int result = arr[0];
    for (int i = 1; i < n; i++) {
        result = lcm(result, arr[i]);
    }
    return result;
}

#endif // FUNCTIONS_H
