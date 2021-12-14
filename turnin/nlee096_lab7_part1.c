/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 022
 *	Assignment: Lab #7  Exercise #1
 *	Exercise Description: increment decrement with LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/1aBqs_qGyQHm0nZXN3nDZJnYX9reKDLb9?usp=sharing
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; 
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}
void TimerOff(){
    TCCR1B = 0x00;
}
void TimerISR(){
    TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

enum States {Start, WAIT, A0_PRESS, A1_PRESS, RESET} state;
unsigned char tempA;
unsigned char cnt;
unsigned char secWait;

void Tick(){
    switch(state){ // Transitions...
        case Start:
            cnt = 0x00;
            secWait = 0;
            state = WAIT;
            break;
        case WAIT:
            secWait = 0;
            if(tempA == 0x00){
                state = WAIT;
            }
            else if(tempA == 0x03){
                cnt = 0x00;
                state = RESET;
            }
            else if(tempA == 0x01){
                cnt = (cnt<9)? (cnt+1) :  0x09;
                state = A0_PRESS;
            }
            else if(tempA == 0x02){
                cnt = (cnt>0)? (cnt-1) :  0x00;
                state = A1_PRESS;
            }
            break;
        case A0_PRESS:
            if(tempA == 0x00){
                secWait = 0;
                state = WAIT;
            }
            else if(tempA == 0x03){
                cnt = 0x00;
                state = RESET;
            }
            else if(tempA == 0x01){
                state = A0_PRESS;
            }
            else if(tempA == 0x02){
                cnt = (cnt>0)? (cnt-1) :  0x00;
                state = A1_PRESS;
            }
            break;
        case A1_PRESS:
            if(tempA == 0x00){
                secWait = 0;
                state = WAIT;
            }
            else if(tempA == 0x03){
                cnt = 0x00;
                state = RESET;
            }
            else if(tempA == 0x01){
                cnt = (cnt<9)? (cnt+1) :  0x09;
                state = A0_PRESS;
            }
            else if(tempA == 0x02){
                state = A1_PRESS;
            }
            break;
        case RESET:
            if(tempA == 0x00){
                state = WAIT;
            }
            else if(tempA == 0x03){
                cnt = 0x00;
                state = RESET;
            }
            else if(tempA == 0x01){
                cnt = (cnt<9)? (cnt+1) :  0x09;
                state = A0_PRESS;
            }
            else if(tempA == 0x02){
                cnt = (cnt>0)? (cnt-1) :  0x00;
                state = A1_PRESS;
            }
        default:
            break;

    } 
    switch(state){
        case A0_PRESS:
            secWait++;
            if(secWait >= 10){
                cnt = (cnt<9)? (cnt+1) :  0x09;
                secWait = 0;
            }
            break;
        case A1_PRESS:
            secWait++;
            if(secWait >= 10){
                cnt = (cnt>0)? (cnt-1) :  0x00;
                secWait = 0;
            }
            break;

    }
}
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    
    TimerSet(100);
    TimerOn();
    LCD_init();
    cnt = 0x00;
    tempA = 0x00;
    secWait = 0x00;
    state = Start;
    unsigned char change = 0;

    LCD_WriteData(cnt + '0');
    while (1) {
        tempA = ~PINA;
        Tick();
        PORTC = cnt;
        if(cnt != change){
            change = cnt;
            LCD_ClearScreen();
            LCD_WriteData(cnt + '0');
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
