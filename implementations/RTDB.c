#include "../definitions/rtdb.h"
#include <string.h>

#define COMMANDS {"!PO11#", "!PO10#", "!PO21#", "!PO20#", "!PO31#", "!PO30#", "!PO41#", "!PO40#"}


void RT_db_init(RT_db *db){
    db->led0 = 0;
    db->led1 = 0;
    db->led2 = 0;
    db->led3 = 0;
    db->button0 = 0;
    db->button1 = 0;
    db->button2 = 0;
    db->button3 = 0;
}

int RT_db_update(RT_db *db, char *command){
    // check if the command is a valid command
    const char *valid_commands[] = COMMANDS;
    int is_valid_command = 0;
    for (int i = 0; i < sizeof(valid_commands) / sizeof(valid_commands[0]); i++) {
        if (strcmp(command, valid_commands[i]) == 0) {
            is_valid_command = 1;
            break;
        }
    }
    if (!is_valid_command) {

        return 0;
    }else{
        if(command[1] == 'P'){	// PC -> Microcontroller communication
            if(command[2] == 'O' && command[5]=='#'){ // turn on a LED
                if(command[3] == '1'){	// LED 1
                    if(command[4] == '1' ){ // turn on
                        db->led0 = 1;
                    } else if(command[4] == '0'){
                        db->led0 = 0;
                    }
                } else if(command[3] == '2'){	// LED 2
                    if(command[4] == '1' ){ // turn on
                        db->led1 = 1;
                    } else if(command[4] == '0'){
                        db->led1 = 0;
                    }
                } else if(command[3] == '3'){	// LED 3
                    if(command[4] == '1' ){ // turn on
                        db->led2 = 1;
                    } else if(command[4] == '0'){
                        db->led2 = 0;
                    }
                    
                }
                
            }
        return 1;
        }
    }
}

void RT_db_print(RT_db *db){
    printk("LED0: %d\n",db->led0);
    printk("LED1: %d\n",db->led1);
    printk("LED2: %d\n",db->led2);
    printk("LED3: %d\n",db->led3);
    printk("Button0: %d\n",db->button0);
    printk("Button1: %d\n",db->button1);
    printk("Button2: %d\n",db->button2);
    printk("Button3: %d\n",db->button3);
}