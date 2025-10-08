#include <Arduino.h>
#include <avr/eeprom.h>
#include <lcd.h>
#define F_CPU 16000000UL

volatile uint8_t updateFlag1 = 0;
volatile uint8_t updateFlag2 = 0;
uint8_t randomNumber = 0;

//Define-Blocks sind Präprozessoranweisungen
//Compiler besteht eigentlich aus drei Teilen: Präprozessor, Compiler, Assembler
#define EEPROM_ADDR 46

void init_interrupts() {
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC11);
    EIMSK |= (1 << INT0) | (1 << INT1);
    PORTD |= (1 << PORTD2) | (1 << PORTD3);
    sei();
}

void init_lcd() {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("Ready...");
}

uint8_t generateNumber() {
    return rand() % 100;
}

void createNumber() {
    randomNumber = generateNumber();
    lcd_clrscr();
    lcd_puts("Num: ");
    char buffer[8];
    sprintf(buffer, "%d", randomNumber);
    lcd_puts(buffer);
}

void saveToEEPROM(uint8_t value) {
    eeprom_write_byte((uint8_t*)EEPROM_ADDR, value);
    lcd_clrscr();
    char buffer[8];
    lcd_puts("Saved: ");
    lcd_gotoxy(0, 1);
    sprintf(buffer, "%d", value);
    lcd_puts(buffer);
    _delay_ms(2000);
    init_lcd();
}

uint8_t readFromEEPROM() {
    return eeprom_read_byte((uint8_t*)EEPROM_ADDR);
}

int main() {
    init_interrupts();
    init_lcd();

    TCCR0B |= (1 << CS01);

    randomNumber = readFromEEPROM();
    lcd_clrscr();
    lcd_puts("Last...");
    lcd_gotoxy(0, 1);
    char buffer[8];
    sprintf(buffer, "%d", randomNumber);
    lcd_puts(buffer);
    _delay_ms(5000);
    init_lcd();

    while(1) {
        if(updateFlag1 == 1) {
            updateFlag1 = 0;
            srand(TCNT0);
            createNumber();
        }

        if(updateFlag2 == 1) {
            updateFlag2 = 0;
            saveToEEPROM(randomNumber);
        }
    }
}

ISR(INT0_vect) {
    updateFlag1 = 1;
}

ISR(INT1_vect) {
    updateFlag2 = 1;
}