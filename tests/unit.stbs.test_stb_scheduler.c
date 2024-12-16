#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include "../include/stb_scheduler.h"

#define MAX_EXECUTIONS 10

static int64_t task_start_times[2][MAX_EXECUTIONS]; // For two tasks, up to 10 executions
static int task_execution_count[2];

static void reset_task_data(void) {
    for (int i = 0; i < 2; i++) {
        task_execution_count[i] = 0;
        for (int j = 0; j < MAX_EXECUTIONS; j++) {
            task_start_times[i][j] = 0;
        }
    }
}

// A simple test task that records its start time and suspends itself immediately.
void test_task0(void *p1, void *p2, void *p3) {
    while (1) {
        int idx = task_execution_count[0];
        if (idx < MAX_EXECUTIONS) {
            task_start_times[0][idx] = k_uptime_get();
            task_execution_count[0]++;
        }
        k_thread_suspend(k_current_get());
    }
}

void test_task1(void *p1, void *p2, void *p3) {
    while (1) {
        int idx = task_execution_count[1];
        if (idx < MAX_EXECUTIONS) {
            task_start_times[1][idx] = k_uptime_get();
            task_execution_count[1]++;
        }
        k_thread_suspend(k_current_get());
    }
}

// Define threads
K_THREAD_DEFINE(task0_tid, 1024, test_task0, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(task1_tid, 1024, test_task1, NULL, NULL, NULL, 5, 0, 0);

// We'll also define the scheduler thread similarly, calling STBS_Start as its entry function.
//K_THREAD_DEFINE(scheduler_tid, 1024, STBS_Start, NULL, NULL, NULL, 5, 0, 0);

static void test_setup(void) {
    reset_task_data();
}

// Define the test suite
ZTEST_SUITE(stbs_test_suite, NULL, NULL, test_setup, NULL, NULL);

/**
 * Test that the scheduler initializes correctly.
 */
ZTEST(stbs_test_suite, test_scheduler_initialization) {
    STBS_Init(100, 10); // tick_ms = 100, max_tasks = 10
    
    zassert_equal(STBS_GetTickMs(), 100, "Incorrect tick period");
    zassert_equal(STBS_GetNumTasks(), 0, "Task count should be 0 after init");
}

/**
 * Test that tasks can be added and appear in the task table.
 */
ZTEST(stbs_test_suite, test_task_addition) {
    STBS_Init(100, 10);

    // Since threads are statically created by K_THREAD_DEFINE,
    // task0_tid and task1_tid are already defined and available.

    STBS_AddTask(2, task0_tid, 1, 5, "Task0");
    STBS_AddTask(3, task1_tid, 1, 5, "Task1");

    zassert_equal(STBS_GetNumTasks(), 1, "Two tasks should have been added");
    
    const Task* table = STBS_GetTaskTable();
    zassert_equal(table[0].ticks, 2, "Task0 should have period of 2 ticks");
    zassert_equal(table[1].ticks, 3, "Task1 should have period of 3 ticks");
    zassert_equal(table[0].id, task0_tid, "Task0 ID should match task0_tid");
    zassert_equal(table[1].id, task1_tid, "Task1 ID should match task1_tid");
}

/**
 * Test that tasks meet their deadlines.
 */
ZTEST(stbs_test_suite, test_tasks_meet_deadlines) {
    STBS_Init(100, 10);

    // Threads are already defined by K_THREAD_DEFINE, no need to create them again.
    // Just re-add tasks to the scheduler with their periods.
    STBS_AddTask(2, task0_tid, 1, 5, "Task0"); // Period 200ms
    STBS_AddTask(3, task1_tid, 1, 5, "Task1"); // Period 300ms

    // The scheduler thread is also pre-defined. It should start running immediately.
    // However, if STBS_Start is blocking, it might run forever.
    // Consider modifying STBS_Start for test conditions if needed.

    // Let the scheduler run for 1000ms
    k_sleep(K_MSEC(1000));

    // Check executions
    zassert_true(task_execution_count[0] >= 4 && task_execution_count[0] <= 6,
                "Task0 did not execute around the expected number of times");
    zassert_true(task_execution_count[1] >= 2 && task_execution_count[1] <= 4,
                "Task1 did not execute around the expected number of times");

    int margin = 20; // ms margin

    // Check Task0 deadlines
    for (int i = 0; i < task_execution_count[0]; i++) {
        int64_t expected = i * 200; // every 200ms
        int64_t actual = task_start_times[0][i] - task_start_times[0][0];
        zassert_within(actual, expected, margin, "Task0 missed its deadline at iteration %d", i);
    }

    // Check Task1 deadlines
    for (int i = 0; i < task_execution_count[1]; i++) {
        int64_t expected = i * 300; // every 300ms
        int64_t actual = task_start_times[1][i] - task_start_times[1][0];
        zassert_within(actual, expected, margin, "Task1 missed its deadline at iteration %d", i);
    }
}

