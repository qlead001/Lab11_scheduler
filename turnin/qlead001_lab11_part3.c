/*	Author: Quinn Leader qlead001@ucr.edu
 *  Partner(s) Name: NA
 *	Lab Section: 026
 *	Assignment: Lab 11  Exercise 3
 *	Exercise Description: Display key pressed on LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "io.h"

// I think the following might work to change what port
// the keypad is using because the LCD uses PORTC.
// This does not throw warnings, but be very scared and
// approach with extreme caution. Expect strange results.
#undef PORTC
#undef PINC
#define PORTC PORTB
#define PINC PINB
#include "keypad.h"
#undef PORTC
#undef PINC
#define PORTC _SFR_IO8(0x08)
#define PINC _SFR_IO8(0x06)

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// -----Shared Variables-----
unsigned char key = 0, prev = 0;
// --------------------------

enum Keypad_States { press, release };

unsigned char pressed;

int KeypadTick(int state) {
    unsigned char input = GetKeypadKey();
    switch (state) {
        case press:
            if (input != '\0') {
                state = release;
                pressed = input;
            }
            break;
        case release:
            if (input == '\0') {
                state = press;
                key = pressed;
            }
            break;
        default: state = press; break;
    }
    return state;
}

enum LCD_States { output };

int LCDTick(int state) {
    switch (state) {
        case output: state = output; break;
        default: state = output; break;
    }
    switch (state) {
        case output:
            if (key != '\0' && key != prev) {
                LCD_Cursor(1);
                LCD_WriteData(key);
                prev = key;
            }
            break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xF0; PORTB = 0x0F;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    static task task1, task2;
    task *tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = press;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &KeypadTick;

    task2.state = output;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &LCDTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }
    TimerSet(GCD);
    TimerOn();
    LCD_init();

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
