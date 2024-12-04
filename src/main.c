/**
 * @file
 * @brief Static Table-Based Scheduler (STBS) Example
 *
 * This file demonstrates a simple STBS implementation for Zephyr.
 * The scheduler uses a static table to manage tasks and activates them periodically.
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include "zephyr/kernel/thread.h"
#include <zephyr/timing/timing.h>   /* for timing services */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include "stb_scheduler.h"

#include "../implementations/RTDB.c"

// GLOBAL

RT_db rtdb;

/************************************  UART  ***********************************/
#define SLEEP_TIME_MS 1000
#define RECEIVE_BUFF_SIZE 10
#define RECEIVE_TIMEOUT 100
#define INPUT_BUFFER_SIZE 20




// LEDS
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

// BUTTONS
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);



const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t tx_buf[] =   {"nRF Connect SDK Fundamentals Course\r\n"
                             "Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit\r\n"};

static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
static char buffer[INPUT_BUFFER_SIZE];
static int buffer_idx = 0;
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {

	case UART_RX_RDY:
		// if ((evt->data.rx.len) == 1) {
		for (size_t i = 0; i < evt->data.rx.len; i++) {
			char received_char = evt->data.rx.buf[evt->data.rx.offset + i];
            if(buffer_idx == 0 && received_char == '!'){
				memset(buffer, 0, INPUT_BUFFER_SIZE);  
                buffer[buffer_idx++] = received_char;
				printk("%c",received_char);
            }
            else if (buffer_idx>0 && received_char == '#'){
                buffer[buffer_idx++] = received_char;
                int result = RT_db_update(&rtdb,buffer);
                if(result == 1){
                    printk("YAAAAAAY\n");
                }
                else{
                    printk("NAAAAAAH\n");
                }
                memset(buffer,0,INPUT_BUFFER_SIZE);
                buffer_idx = 0;
            }else if (buffer_idx>0) {
                buffer[buffer_idx++] = received_char;
				printk("%c",received_char);
            }
		}
		break;
    case UART_TX_DONE:
        // if(buffer[1] == 'P'){	// PC -> Microcontroller communication
        //     if(buffer[2] == 'O' && buffer[5]=='#'){ // turn on a LED
        //         if(buffer[3] == '1'){	// LED 1
        //             if(buffer[4] == '1' ){ // turn on
        //                 gpio_pin_toggle_dt(&led0);
        //                 printk("toggling led0\n");
        //             } 
        //         } else if(buffer[3] == '2'){	// LED 2
        //             if(buffer[4] == '1') // turn on
        //                 gpio_pin_toggle_dt(&led1);
        //         } else if(buffer[3] == '3'){	// LED 3
        //             if(buffer[4] == '1') // turn on
        //                 gpio_pin_toggle_dt(&led2);
        //         }
        //         memset(buffer,0,INPUT_BUFFER_SIZE);
        //         buffer_idx = 0;
        //     }
        // }
        
        break;

	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
		break;

	default:
		break;
	}
}




/************************** THREADS ******************************/

// Configuration constants
#define TICK_MS 200          // Scheduler tick period in milliseconds
#define MAX_TASKS 15

extern const k_tid_t thread0,thread1,thread2,thread3;

void task0(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread0);
        gpio_pin_set_dt(&led0,rtdb.led0);
        gpio_pin_set_dt(&led1,rtdb.led1);
        gpio_pin_set_dt(&led2,rtdb.led2);
        gpio_pin_set_dt(&led3,rtdb.led3);
        // RT_db_print(&rtdb);
        // gpio_pin_set_dt(&led3,rtdb.led3);
        // printk("Task0 executing %d\n",thread0); // Simulate task behavior
    }
}

void task1(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    static int prev_button0 = 0;
    static int prev_button1 = 0;
    static int prev_button2 = 0;
    static int prev_button3 = 0;
    while (1) {
        k_thread_suspend(thread1);
        
        rtdb.button0 = gpio_pin_get_dt(&button0); // Read button0 state
        rtdb.button1 = gpio_pin_get_dt(&button1); // Read button1 state
        rtdb.button2 = gpio_pin_get_dt(&button2); // Read button2 state
        rtdb.button3 = gpio_pin_get_dt(&button3); // Read button3 state
        

        // based on the button state,change the led state once
        if(rtdb.button0 == 1 && prev_button0 == 0){
            rtdb.led0 = !rtdb.led0;
        }
        if(rtdb.button1 == 1 && prev_button1 == 0){
            rtdb.led1 = !rtdb.led1;
        }
        if(rtdb.button2 == 1 && prev_button2 == 0){
            rtdb.led2 = !rtdb.led2;
        }
        if(rtdb.button3 == 1 && prev_button3 == 0){
            rtdb.led3 = !rtdb.led3;
        }

        prev_button0 = rtdb.button0;
        prev_button1 = rtdb.button1;
        prev_button2 = rtdb.button2;
        prev_button3 = rtdb.button3;
        

        // k_msleep(TICK_MS); // Simulate work
    }
}

void task2(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread2);
        



        // k_msleep(TICK_MS); // Simulate work
    }
}

void task3(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread3);
        //printk("Task3 executing %d\n",thread3); // Simulate task behavior
        // k_msleep(TICK_MS); // Simulate work
    }
}

/*
 * defining threads
*/
K_THREAD_DEFINE(thread0 , 512, task0, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread1, 512, task1, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread2, 512, task2, NULL, NULL, NULL,5,0,0);
K_THREAD_DEFINE(thread3, 512, task3, NULL, NULL, NULL,5,0,0);

/**
 * Main function demonstrating the Static Table-Based Scheduler (STBS).
 */
void main(void) {
    printk("Zephyr STBS Example\n");

    uint8_t msg[INPUT_BUFFER_SIZE];
    int ret;

	/* Verify that the UART device is ready */
	if (!device_is_ready(uart)){
		printk("UART device not ready\r\n");
		return 1 ;
	}
	/* Verify that the LED devices are ready */
	if (!device_is_ready(led0.port)){
		printk("GPIO device is not ready\r\n");
		return 1;
	}
	/* Configure the GPIOs of the LEDs */
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ; 
	}
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ;
	}
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ;
	}
    ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 1 ;
    }

    /* Configure the GPIOs of the buttons */
    ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
    if (ret < 0) {
        return 1 ;
    }

    ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
    if (ret < 0) {
        return 1 ;
    }

    ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
    if (ret < 0) {
        return 1 ;
    }

    ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
    if (ret < 0) {
        return 1 ;
    }


	/* Register the UART callback function */
	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret) {
		return 1;
	}
	/* Send the data over UART by calling uart_tx() */
	// ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	// if (ret) {
	// 	return 1;
	// }
	/* Start receiving by calling uart_rx_enable() and pass it the address of the receive buffer */
	ret = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
	if (ret) {
		return 1;
	}


    RT_db_init(&rtdb);



    // Initialize the scheduler
    STBS_Init(TICK_MS,MAX_TASKS);

    // Add tasks with different periods
    // STBS_AddTask(1, thread0, 1,40,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 1,120,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 1,160,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 1,40,"thread3"); // Task 2: Period = 2 tick

    STBS_AddTask(1, thread0, 1,20,"thread0"); // Task 1: Period = 1 ticks
    STBS_AddTask(2, thread1, 1,20,"thread1"); // Task 2: Period = 2 tick
    STBS_AddTask(3, thread2, 1,20,"thread2"); // Task 3: Period = 3 ticks

    // STBS_AddTask(1, thread0, 10,40,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 5,50,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 7,60,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 2,30,"thread3"); // Task 2: Period = 2 tick

    // STBS_print_content();
    // Start the scheduler
    STBS_Start();
}
