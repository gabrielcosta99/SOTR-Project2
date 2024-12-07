#ifndef FRAMES_H
#define FRAMES_H

#include <stdbool.h>

// Function prototypes
void process_frames(const char *frame, int frame_length, int checksum);
int calculate_checksum(const char *frame, int length);
void send_ack(char error_code);
void set_led(int led_index, int value);
bool validate_led_states(const char *payload);
void send_inputs();
void send_outputs();

#endif // FRAMES_H
