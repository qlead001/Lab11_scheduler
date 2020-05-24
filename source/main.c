/*	Author: Quinn Leader qlead001@ucr.edu
 *  Partner(s) Name: NA
 *	Lab Section: 026
 *	Assignment: Lab 11  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "keypad.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// -----Shared Variables-----

// --------------------------

enum SM_States { state1, state2 };

int SMTick(int state) {
    switch (state) {
        case state1: state = state1; break;
        case state2: state = state1; break;
        default: state = state1; break;
    }
    switch (state) {
        case state1: break;
        case state2: break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
    static _task task1;
    _task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = start;
    task1.period = ;
    task1.elapsedTime = task1.period;
    task1.TickFct = &SMTick;

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
