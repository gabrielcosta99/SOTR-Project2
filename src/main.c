/**
 * @file
 * @brief Static Table-Based Scheduler (STBS) Example
 *
 * This file demonstrates a simple STBS implementation for Zephyr.
 * The scheduler uses a static table to manage tasks and activates them periodically.
 */

#include <sys/_intsup.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "zephyr/kernel/thread.h"
#include <zephyr/timing/timing.h>   /* for timing services */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>


// helper functions
#include "../include/functions.h"
#include "../include/stb_scheduler.h"
#include "../include/frames.h"
#include "../include/rtdb.h"
#include "zephyr/sys/sys_io.h"

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

/************************** FRAME PROCESSING ******************************/

/**
 * Process a frame received over UART.
 * @param frame Frame to process
 * @param frame_length Length of the frame
 * @param checksum Checksum of the frame
 * @param checksum Checksum of the frame
 */
void process_frame(const char *frame, int frame_length) {

    if(!strcmp(frame,"!PO13#")){
        frame = "!PO53#";      // change a character to simulate corrupted data 
        // printk("new frame: %s\n",frame);
    }
    // Validate frame structure
    if (frame_length < 5 || frame[0] != '!' || frame[frame_length - 1] != '#' ||
        !isdigit(frame[frame_length - 4]) ||
        !isdigit(frame[frame_length - 3]) ||
        !isdigit(frame[frame_length - 2])) {
        send_ack('4'); // Frame structure error
        return;
    }


    // Extract command and payload
    char device_id = frame[1];
    char command = frame[2];

    const char *checksum = &frame[frame_length - 4];

    
    int received_checksum = atoi(checksum);
    // Calculate and validate checksum!

    if (calculate_checksum(frame, frame_length-4) != received_checksum) {
        printk("Received checksum: %d  Calculated checksum: %d\n", received_checksum,calculate_checksum(frame, frame_length - 4));
        send_ack('3'); // Checksum error
        return;
    }

    // Process command
    switch (command) {
    case 'O': // Set individual LED
        char payloadA[2];
        memcpy(payloadA, &frame[3], frame_length - 7);
        if (payloadA[0] >= '1' && payloadA[0] <= '4' &&
            (payloadA[1] == '0' || payloadA[1] == '1')) {
            set_led(payloadA[0]-'1', payloadA[1]-'0');
            send_ack('1'); // Acknowledge success
        } else {
            send_ack('4'); // Invalid payload
        }
        break;

    case 'A': // Set all LEDs (atomic operation)
        char payloadB [4];
        memcpy(payloadB, &frame[3], frame_length - 7);  
        
        if (validate_led_states(payloadB)) {
            for (int i = 0; i < 4; i++) {
                set_led(i, payloadB[i]-'0');
            }
            send_ack('1'); // Acknowledge success
        } else {
            send_ack('4'); // Invalid payload
        }
        break;

    case 'I': // Read digital inputs
        send_inputs();
        break;

    case 'E': // Read digital outputs
        send_outputs();
        break;

    case 'C':
        rtdb.led0 = -1;
        break;

    default:
        send_ack('2'); // Unknown command
        break;
    }
}

/**
 * Calculate the checksum of a frame.
 * The checksum is the sum of all bytes in the frame, excluding the checksum itself.
 * @param frame Frame to calculate checksum for
 * @param length Length of the frame
 * @return Checksum of the frame
 */
int calculate_checksum(const char *frame, int length) {
    int checksum = 0;
    for (int i = 1; i < length; i++) {
        checksum += frame[i];
    
    }
    return checksum % 1000; // Return last 3 digits
}

/**
 * Send an acknowledgment frame over UART.
 * @param error_code Error code to send in the acknowledgment frame
 */
void send_ack(char error_code) {
    static char ack_frame[20]; // Ensure a static buffer is used
    snprintf(ack_frame, sizeof(ack_frame), "!MZO%c000#", error_code);

    int checksum = calculate_checksum(ack_frame, strlen(ack_frame) - 4);
    ack_frame[5] = '0' + (checksum / 100);
    ack_frame[6] = '0' + ((checksum / 10) % 10);
    ack_frame[7] = '0' + (checksum % 10);
    
    int ret = uart_tx(uart, ack_frame, strlen(ack_frame), SYS_FOREVER_MS);
    if (ret != 0) {
        printk("UART TX failed with error: %d\n", ret);
    }
}


/**
 * Set the state of an LED.
 * @param led_index Index of the LED (0-3)
 * @param value New state of the LED (0 or 1)
 */
void set_led(int led_index, int value) {
    switch (led_index) {
    case 0: rtdb.led0 = value; break;
    case 1: rtdb.led1 = value; break;
    case 2: rtdb.led2 = value; break;
    case 3: rtdb.led3 = value; break;
    default: break;
    }
}

/**
 * Validate the payload of an "A" command.
 * The payload must be a 4-character string containing only '0' and '1' characters.
 */
bool validate_led_states(const char *payload) {
    for (int i = 0; i < 4; i++) {
        if (payload[i] != '0' && payload[i] != '1') {
            return false;
        }
    }
    return true;
}

/**
 * Send the current state of the buttons over UART.
 */
void send_inputs() {
    static char input_frame[] = "!Mi0000####";
    input_frame[3] = '0' + rtdb.button0;
    input_frame[4] = '0' + rtdb.button1;
    input_frame[5] = '0' + rtdb.button2;
    input_frame[6] = '0' + rtdb.button3;

    // Update checksum
    int checksum = calculate_checksum(input_frame, strlen(input_frame) - 4);
    input_frame[7] = '0' + (checksum / 100); // Hundreds place
    input_frame[8] = '0' + ((checksum / 10) % 10); // Tens place
    input_frame[9] = '0' + (checksum % 10); // Units place

    int err = uart_tx(uart, input_frame, strlen(input_frame), SYS_FOREVER_MS);
    if (err) {
        printk("uart_tx() error. Error code:%d\n\r",err);
        return;
    }
}

/**
 * Send the current state of the LEDs over UART.
 */
void send_outputs() {
    static char output_frame[] = "!Me0000####";
    output_frame[3] = '0' + rtdb.led0;
    output_frame[4] = '0' + rtdb.led1;
    output_frame[5] = '0' + rtdb.led2;
    output_frame[6] = '0' + rtdb.led3;

    // Update checksum
    int checksum = calculate_checksum(output_frame, strlen(output_frame) - 4);
    output_frame[7] = '0' + (checksum / 100); // Hundreds place
    output_frame[8] = '0' + ((checksum / 10) % 10); // Tens place
    output_frame[9] = '0' + (checksum % 10); // Units place

    int err = uart_tx(uart, output_frame, strlen(output_frame), SYS_FOREVER_MS);
    if (err) {
        printk("uart_tx() error. Error code:%d\n\r",err);
        return;
    }
}


/************************** UART ******************************/

// static uint8_t tx_buf[] =   {"nRF Connect SDK Fundamentals Course\r\n"
//                             "Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit\r\n"};

static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
static char buffer[INPUT_BUFFER_SIZE];
static int buffer_idx = 0;
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
    static char frame_buffer[INPUT_BUFFER_SIZE] = {0}; // Frame buffer
    static int frame_idx = 0;

    switch (evt->type) {
    case UART_RX_RDY:
        for (size_t i = 0; i < evt->data.rx.len; i++) {
            char received_char = evt->data.rx.buf[evt->data.rx.offset + i];
            if (frame_idx == 0 && received_char == '!') { // Start of frame
                memset(frame_buffer, 0, INPUT_BUFFER_SIZE);
                frame_buffer[frame_idx++] = received_char;
                printk("%c", received_char);
            }
            else if(frame_idx>0) {
                printk("%c", received_char);
                frame_buffer[frame_idx++] = received_char;
                if (received_char == '#') { // End of frame
                    printk("\n");
                    // put the checksum in the buffer before the '#'
                    // int checksum = calculate_checksum(frame_buffer, frame_idx - 1);

                    process_frame(frame_buffer, frame_idx); // Process the frame
                    frame_idx = 0;
                }
            }
        }
        break;

    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
        break;

    default:
        break;
    }
}






/************************** THREADS ******************************/

// Configuration constants
#define TICK_MS 50          // Scheduler tick period in milliseconds
#define MAX_TASKS 15

extern const k_tid_t thread0,thread1,thread2,thread3;


/**
 * Task 0: Periodic task with period 1 tick
 * this task is responsible for updating the RTDB with the button states
 */
void task0(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        
        k_thread_suspend(thread0);
        int timer1 = k_uptime_get();
        // printk("T0->timer1: %d\n",timer1);
        gpio_pin_set_dt(&led0,rtdb.led0);
        gpio_pin_set_dt(&led1,rtdb.led1);
        gpio_pin_set_dt(&led2,rtdb.led2);
        gpio_pin_set_dt(&led3,rtdb.led3);

        rtdb.button0 = gpio_pin_get_dt(&button0); // Read button0 state
        rtdb.button1 = gpio_pin_get_dt(&button1); // Read button1 state
        rtdb.button2 = gpio_pin_get_dt(&button2); // Read button2 state
        rtdb.button3 = gpio_pin_get_dt(&button3); // Read button3 state
    
        int timer2 = k_uptime_get();
        // printk("T0->timer2: %d\n",timer2);
        // printk("Task0 execution time: %d\n",timer2-timer1);
        // RT_db_print(&rtdb);
        // gpio_pin_set_dt(&led3,rtdb.led3);
        // printk("Task0 executing %d\n",thread0); // Simulate task behavior
    }
}


/**
 * Task 1: Periodic task with period 2 ticks
 * this task is responsible for updating the led states based on the button states from the RTDB
 */
void task1(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    static int prev_button0 = 0;
    static int prev_button1 = 0;
    static int prev_button2 = 0;
    static int prev_button3 = 0;
    while (1) {
        k_thread_suspend(thread1);
        int timer1 = k_uptime_get();
        // printk("T1->timer1: %d\n",timer1);
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

        int timer2 = k_uptime_get();
        // printk("T1->timer2: %d\n",timer2);
        // printk("Task1 execution time: %d\n",timer2-timer1);

        // k_msleep(TICK_MS); // Simulate work
    }
}

// Validate rtdb entries and reset them if they are corrupted
void task2(void *argA, void *argB, void *argC) {
    // k_tid_t task_id = *(k_tid_t *)id_ptr; // Retrieve task ID
    while (1) {
        k_thread_suspend(thread2);

        int timer1 = k_uptime_get();
        // printk("T2->timer1: %d\n",timer1);
        if(rtdb.button0 != 0 && rtdb.button0 != 1) {rtdb.button0 = 0; printk("corrupted data\n");}
        if(rtdb.button1 != 0 && rtdb.button1 != 1) {rtdb.button1 = 0; printk("corrupted data\n");}
        if(rtdb.button2 != 0 && rtdb.button2 != 1) {rtdb.button2 = 0; printk("corrupted data\n");}
        if(rtdb.button3 != 0 && rtdb.button3 != 1) {rtdb.button3 = 0; printk("corrupted data\n");}

        if(rtdb.led0 != 0 && rtdb.led0 != 1) {rtdb.led0 = 0; printk("corrupted data\n");}
        if(rtdb.led1 != 0 && rtdb.led1 != 1) {rtdb.led1 = 0; printk("corrupted data\n");}
        if(rtdb.led2 != 0 && rtdb.led2 != 1) {rtdb.led2 = 0; printk("corrupted data\n");}
        if(rtdb.led3 != 0 && rtdb.led3 != 1) {rtdb.led3 = 0; printk("corrupted data\n");}
        int timer2 = k_uptime_get();
        // printk("T2->timer1: %d\n",timer2);
        // printk("Task2 execution time: %d\n",timer2-timer1);

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
K_THREAD_DEFINE(thread2, 512, task2, NULL, NULL, NULL,1,0,0);
K_THREAD_DEFINE(thread3, 512, task3, NULL, NULL, NULL,5,0,0);

/**
 * Main function demonstrating the Static Table-Based Scheduler (STBS).
 */
int main(void) {
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
	ret = uart_rx_enable(uart ,rx_buf,sizeof(rx_buf),RECEIVE_TIMEOUT);
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

    STBS_AddTask(1, thread0, 1,3,"thread0"); // Task 1: Period = 1 ticks
    STBS_AddTask(2, thread1, 2,3,"thread1"); // Task 2: Period = 2 tick
    STBS_AddTask(2, thread2, 1,3,"thread2"); // Task 3: Period = 3 ticks

    // STBS_AddTask(1, thread0, 10,40,"thread0"); // Task 1: Period = 1 ticks
    // STBS_AddTask(3, thread2, 5,50,"thread2"); // Task 3: Period = 3 ticks
    // STBS_AddTask(2, thread1, 7,60,"thread1"); // Task 2: Period = 2 tick
    // STBS_AddTask(2, thread3, 2,30,"thread3"); // Task 2: Period = 2 tick

    // STBS_print_content();
    // Start the scheduler
    STBS_Start();
    return 0;
}