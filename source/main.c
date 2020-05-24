/*	Author: Quinn Leader qlead001@ucr.edu
 *  Partner(s) Name: NA
 *	Lab Section: 026
 *	Assignment: Lab 11  Exercise 1
 *	Exercise Description: Keypad to hex on LEDs
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
unsigned char key;
// --------------------------

enum Keypad_States { input };

int KeypadTick(int state) {
    switch (state) {
        case input: state = input; break;
        default: state = input; break;
    }
    switch (state) {
        case input:
            key = GetKeypadKey();
            switch (key) {
                case '\0': PORTB = 0x1F; break;
                case '1': PORTB = 0x01; break;
                case '2': PORTB = 0x02; break;
                case '3': PORTB = 0x03; break;
                case '4': PORTB = 0x04; break;
                case '5': PORTB = 0x05; break;
                case '6': PORTB = 0x06; break;
                case '7': PORTB = 0x07; break;
                case '8': PORTB = 0x08; break;
                case '9': PORTB = 0x09; break;
                case 'A': PORTB = 0x0A; break;
                case 'B': PORTB = 0x0B; break;
                case 'C': PORTB = 0x0C; break;
                case 'D': PORTB = 0x0D; break;
                case '*': PORTB = 0x0E; break;
                case '0': PORTB = 0x00; break;
                case '#': PORTB = 0x0F; break;
                default: PORTB = 0x1B; break;
            }
            break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = input;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &KeypadTick;

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
