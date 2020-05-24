/*	Author: Quinn Leader qlead001@ucr.edu
 *  Partner(s) Name: NA
 *	Lab Section: 026
 *	Assignment: Lab 11  Exercise 2
 *	Exercise Description: Scroll text on LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// -----Shared Variables-----
unsigned char *msg = "CS120B is Legend... wait for it DARY!";
unsigned char msgIndex = 0, len = sizeof(msg)/sizeof(char);
// --------------------------

enum LCD_States { output };

int LCDTick(int state) {
    switch (state) {
        case output: state = output; break;
        default: state = output; break;
    }
    switch (state) {
        case output:
            LCD_Cursor(1);
            unsigned char remaining = len-msgIndex, i;
            for (i = 0; i < 32 && i < remaining; i++) {
                LCD_WriteData(msg[i+msgIndex]);
            }
            msgIndex = (msgIndex+i)%len;
            break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = output;
    task1.period = 300;
    task1.elapsedTime = task1.period;
    task1.TickFct = &LCDTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }
    TimerSet(GCD);
    TimerOn();

    while (1) {
        for (i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime >= tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
