/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 022
 *	Assignment: Lab #7  Exercise #2
 *	Exercise Description: LED game with LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/1Qp5rGXNG4sg4V2iSrnHlrBYPisTs8Fv7?usp=sharing
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

unsigned char tmpB = 0x00;
unsigned char curr_light = 0x00;
unsigned char i = 0x00 ;
unsigned char tmpA = 0x00;
unsigned char light[4] = {0x01, 0x02, 0x04, 0x02};
unsigned char score = 5;

enum States {Start, NEXT, Hold, Release, Hold_R} state;
void Tick(){
    switch(state){
        case Start:
            tmpB = 0x00;
            curr_light = 0x00;
            i = 0x00;
            state = NEXT;
            break;
        case NEXT:
            if((tmpA & 0x01) == 1){
                if(light[curr_light] == 0x02 ){
                    score = (score < 9)? (score + 1) : 0x09; 
                }
                else{
                    score = (score > 0)? (score - 1) : 0x00;
                }
                state = Hold;

            }
            else{
                state = NEXT;
            }
            break;
        case Hold:
            if((tmpA & 0x01) == 1){
                state = Hold;
            }
            else if((tmpA & 0x01) == 0){
                state = Release;
            }
            break;
        case Release:
            if((tmpA & 0x01) == 0){
                state = Release;
            }
            else if((tmpA & 0x01) == 1){
                state = Hold_R;
            }
            break;
        case Hold_R:
            if((tmpA & 0x01) == 1){
                state = Hold_R;
            }
            if((tmpA & 0x01) == 0){
                i = 0;
                curr_light = 0;
                if(score >= 9){
                    score = 5;
                }
                state = NEXT;
            }
            break;
        default:
        break;
    }

    switch(state){
        case NEXT:
            i++;
            if(i > 3){
                i = 1;
                curr_light ++;
                if(curr_light >= 4){
                    curr_light = 0;
                }
            }
            tmpB = light[curr_light];
            break;

        default:
        break; 
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    
    TimerSet(100);
    TimerOn();
    tmpB = 0x00;
    curr_light = 0x00;
    i = 0x00;
    tmpA = 0x00;
    state = Start;
    
    /* Insert your solution below */
    LCD_init();
    while (1) {
        tmpA = ~PINA;
        Tick();
        PORTB = tmpB;
        LCD_ClearScreen();
        if(score == 9){
            LCD_DisplayString(1, "WINNER");
        }
        else{
            LCD_WriteData(score + '0');
        }
        
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
