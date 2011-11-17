/*
DC_Motor_Speed_Controller
(c) Created by Charalampos Andrianakis on 11/14/11.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
*/
/*Information about rotary encoder how-to you can find here. 
http://www.candrian.gr/index.php/3-pin-rotary-encoder-how-to/
*/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define F_CPU 9600000UL

#define PWM         PB0
#define A           PB1
#define B           PB2
#define MAX         PB3
#define MIN         PB4
#define SAVE        PB5
#define PWM_VALUE   1
#define STEP        5   //PWM increment/decrement Step

//Falling Edge INT0 interrupt
ISR(INT0_vect){
    if (!(PINB & _BV(B))) {
        //cw left
        if (OCR0A>0) {
            OCR0A -= STEP;
            PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        }else{
            OCR0A = 0;
            PORTB   &=~  _BV(MIN);              //Turn MIN led ON
        }
    }else{
        //ccw right
        if (OCR0A<255) {
            OCR0A += STEP;
            PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both led OFF
        }else{
            OCR0A = 255;
            PORTB   &=~  _BV(MAX);              //Turn MAX led ON
        }
    }
}

void blink(){
    
    if (!(PINB & _BV(MIN))) {               //If MIN led ON
        
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MAX);               //Turn MAX led OFF       
        
    }else if(!(PINB & _BV(MAX))){           //If MAX led ON
        
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MIN);               //Turn MIN led OFF  
    }else{
        
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MAX) | _BV(MIN);    //Turn both leds OFF
        _delay_ms(50);
        PORTB   &=  _BV(MIN) & _BV(MAX);    //Turn both leds ON
        _delay_ms(50);
        PORTB   |=  _BV(MIN) | _BV(MAX);    //Turn both leds OFF  
        
    }


}

void port_init(){
        
    GIMSK   |=  _BV(INT0);                              //Enable external interrupt INT0
    MCUCR   |=  _BV(ISC01);                             //Falling Edge of INT0 generates interrupt
    
    DDRB    &=~ _BV(A)      &   _BV(B);                 //A & B pin as input
//    PORTB   |=  _BV(A)      |   _BV(B);                 //A & B internal pull-up enabled
    
    DDRB    |=  _BV(PWM)    |  _BV(MAX) |   _BV(MIN);   //MIN MAX PWM output 
}

void pwm_init(){
    
    TCCR0A  |=  _BV(COM0A1);                    //Set OC0A on Compare Match, clear OC0A at TOP
    TCCR0A  |=  _BV(WGM00)   | _BV(WGM01);      //Fast PWM Top OCR0A
    TCCR0B  |=  _BV(CS00) | _BV(CS01);          //Prescaler 64| 9.6Mhz/64=150kHz
}

int main(void){
    
    uint8_t temp;

    port_init();                                    //Initialize ports
    pwm_init();                                     //Initialize PWM

    PORTB   |=  _BV(MAX) | _BV(MIN);                //Turn both MAX & MIN leds OFF

    OCR0A=eeprom_read_byte((uint8_t *)PWM_VALUE);   //Read PWM saved value
    
    if (OCR0A==0) {                                 //If pwm is at minimum value turn MIN led on
        PORTB   &=~  _BV(MIN);                      //Turn MIN led ON
    }else if(OCR0A==255){                           //If pwm is at maximum value turn MAX led on
        PORTB   &=~  _BV(MAX);                      //Turn MAX led ON
    }else{                                          //Else turn both leds off
        PORTB   |=  _BV(MAX) | _BV(MIN);            //Turn both led OFF
    }
    
    sei();

    while (1) {
        //Every two seconds search if value has changed and save it to eeprom
        _delay_ms(1000);
        _delay_ms(1000);
        temp=eeprom_read_byte((uint8_t *)PWM_VALUE);
        if (temp!=OCR0A) {
            eeprom_write_byte((uint8_t *)PWM_VALUE, OCR0A);
            blink();
        }

    }

    return 0;
}
