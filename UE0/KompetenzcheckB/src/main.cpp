#include <Arduino.h>
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL

//Was brauche ich?
//LCD, Zeile eins die Temperatur immer, wenn Taste T1 betätigt wird, soll der Schwellenwert für 1sek angezeigt werden


void adc_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
    ADMUX |= (1 << REFS0);
}

uint16_t adc_read() {
    ADMUX |=  (1 << MUX0);
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

volatile uint8_t button_pressed = 0;
void interrupt_init() {
    EIMSK |= (1 << INT0);
    EICRA |= (1 << ISC01);
    PORTD |= (1 << PORTD2);
}

ISR(INT0_vect) {button_pressed = 1;}

volatile uint8_t tick1s = 0;
void timer1_init() {
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
    OCR1A = 15624; // (f_Cpu/1024) - 1 => Wegen 1sekunde, müssen wir das rechnen
    TIMSK1 |= (1 << OCIE1A); //Timer Compare
}

ISR(TIMER1_COMPA_vect) {tick1s = 1;}

//LED an PB0
void led_init() {DDRB |= (1 << DDB0);}
void led_on() {PORTB |= (1 << PORTB0);}
void led_off() {PORTB &= ~(1 << PORTB0);}

void lcd_show_temp(float temp) {
    char buffer[8];
    sprintf(buffer, "Temp: %2.1f C", temp);
    lcd_gotoxy(0, 0);
    lcd_puts(buffer);
}

void lcd_show_threshold(float threshold) {
    char buffer[8];
    sprintf(buffer, "Schw: %2.1fC", threshold);
    lcd_gotoxy(0, 0);
    lcd_puts(buffer);
}

//Temperatur-Berechnung aus dem ADC-Wert (NTC)
float ntc_celsius_from_adc(uint16_t adcValue) {
    float tempK = log(10000.0 * ((1024.0 / (float)adcValue - 1)));
    tempK = 1.0 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK)) * tempK);
    return tempK - 273.15;
}

int main() {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    adc_init();
    timer1_init();
    interrupt_init();
    led_init();
    sei();

    //Startschwellenwert
    float threshold = 22.0f;
    uint8_t showSW = 0; //Wenn =1, soll der Schwellenwert angezeigt werden

    while(1) {
        if(button_pressed) {
            button_pressed = 0;
            threshold += 0.5f;
            if(threshold > 24.0f)
                threshold = 20.0f;
            showSW = 1;
            lcd_show_threshold(threshold);
        }

        if(tick1s) {
            tick1s = 0;

            uint16_t raw_adc_value = adc_read();
            float tempC = ntc_celsius_from_adc(raw_adc_value);

            if(tempC > threshold)
                led_on();

            else
                led_off();

            if(showSW)
                showSW = 0;

            else {
                lcd_show_temp(tempC);
            }
        }
    }
}