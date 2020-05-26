/*	Author: Quinn Leader qlead001@ucr.edu
 *  Partner(s) Name: NA
 *	Lab Section: 026
 *	Assignment: Lab 11  Exercise 5
 *	Exercise Description: Game on LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void LCD_DisplayCenter( unsigned char row, const unsigned char* string) {
    unsigned char pad = (16-strlen(string))/2 + 1;
    LCD_Cursor(16*row+pad);
    while(*string) {
        LCD_WriteData(*string++);
    }
}

// -----Shared Variables-----
unsigned char pause = 1, up = 0, down = 0, gameover = 0,
              update = 0, pos = 0;
unsigned short score = 0;
unsigned char row1[16] = {0}, row2[16] = {0};
// --------------------------

enum Button_States { input };

unsigned char old0 = 0, old1 = 0, old2 = 0;

int ButtonTick(int state) {
    unsigned char in = (~PINA);
    unsigned char A0 = (in&0x01), A1 = (in&0x02), A2 = (in&0x04);
    switch (state) {
        case input:
            if (A0 != old0) {
                old0 = A0;
                if (A0) up = 1;
            }
            if (A1 != old1) {
                old1 = A1;
                if (A1) down = 1;
            }
            if (A2 != old2) {
                old2 = A2;
                if (A2) pause = pause?0:1;
            }
            break;
        default: state = input; break;
    }
    return state;
}

enum Game_States { gamePause, gamePlay };

const unsigned char maxPeriod = 5, startDiff = 10;
unsigned char countPeriod, periods, cycles, difficulty, start = 1;

void clearRows(void) {
    for (unsigned char i = 0; i < 16; i++) {
        row1[i] = 0;
        row2[i] = 0;
    }
}

int GameTick(int state) {
    switch (state) {
        case gamePause:
            if (!pause) {
                if (gameover || start) {
                    score = 0;
                    pos = 0;
                    gameover = start = 0;
                    periods = maxPeriod;
                    difficulty = startDiff;
                    countPeriod = cycles = 0;
                    clearRows();
                    LCD_ClearScreen();
                }
                up = down = 0;
                state = gamePlay;
            }
            break;
        case gamePlay:
            if (pause) state = gamePause;
            else {
                if (up) {
                    pos = 0;
                    up = down = 0;
                } else if (down) {
                    pos = 1;
                    down = 0;
                }
                if ((pos && row2[0])||(!pos && row1[0])) {
                    update = gameover = pause = 1;
                    state = pause;
                } else if (countPeriod++ == periods) {
                    countPeriod = 0;
                    update = 1;
                    for (unsigned char i = 0; i < 15; i++) {
                        row1[i] = row1[i+1];
                        row2[i] = row2[i+1];
                    }
                    row1[15] = 0; row2[15] = 0;
                    if (!(rand() % difficulty)) {
                        if (rand() % 2) row2[15] = 1;
                        else row1[15] = 1;
                    }
                    if (cycles++ == 10) {
                        cycles = 0;
                        if (periods > 1) periods--;
                        else if (difficulty > 2) difficulty--;
                    }
                    score += 1+startDiff+maxPeriod-periods-difficulty;
                }
            }
            break;
        default: state = gamePause; break;
    }
    return state;
}

enum LCD_States { output };

void scoreString(unsigned short score, unsigned char* buf) {
    strcpy(buf, "Score: ");
    unsigned char suppress = 1;
    unsigned char i = strlen(buf);
    for (unsigned short div = 10000; div > 1; div /= 10) {
        unsigned char num = (unsigned char)(score/div);
        score = score % div;
        if (!suppress || num > 0) {
            suppress = 0;
            buf[i++] = num+'0';
        }
    }
    buf[i] = (unsigned char)(score)+'0';
    buf[i+1] = '\0';
}

int LCDTick(int state) {
    switch (state) {
        case output: state = output; break;
        default: state = output; break;
    }
    switch (state) {
        case output:
            if (update) {
                update = 0;
                if (gameover) {
                    unsigned char buf[13];
                    LCD_ClearScreen();
                    LCD_DisplayCenter(0, "Game Over!");
                    LCD_Cursor(27);
                    scoreString(score, buf);
                    LCD_DisplayCenter(1, buf);
                }else{
                    unsigned char prev = row1[0];
                    LCD_Cursor(1);
                    LCD_WriteData(prev?'#':' ');
                    for (unsigned char i = 1; i < 16; i++) {
                        if (prev != row1[i]) {
                            LCD_Cursor(i+1);
                            LCD_WriteData(row1[i]?'#':' ');
                        }
                        prev = row1[i];
                    }
                    prev = row2[0];
                    LCD_Cursor(17);
                    LCD_WriteData(prev?'#':' ');
                    for (unsigned char i = 1; i < 16; i++) {
                        if (prev != row2[i]) {
                            LCD_Cursor(i+17);
                            LCD_WriteData(row2[i]?'#':' ');
                        }
                        prev = row2[i];
                    }
                }
            }
            if (!gameover) {
                if (pos) LCD_Cursor(17);
                else LCD_Cursor(1);
            }
            break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = input;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ButtonTick;

    task2.state = gamePause;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &GameTick;

    task3.state = output;
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &LCDTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for (i = 1; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }
    srand(time(NULL));
    TimerSet(GCD);
    TimerOn();
    LCD_init();
    LCD_DisplayCenter(0, "Press to start");
    LCD_Cursor(1);

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
