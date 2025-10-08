#include <Arduino.h>
#include "lcd.h"
#include "dht.h"

#define F_CPU 16000000UL

int8_t currentTemp;
int8_t currentHumidity;
int8_t errorStatus;

void init_lcd()
{
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
}

void init_dht11()
{
    errorStatus = dht_gettemperaturehumidity(&currentTemp, &currentHumidity);
    if(errorStatus != 0)
    {
        lcd_puts("Dht11 err");
        _delay_ms(DHT_TIMEOUT);
    }
}

void print_values(const int8_t temp, const int8_t hum)
{
    char tempBuffer [9];
    char humBuffer[9];
    sprintf(tempBuffer, "%d", temp);
    sprintf(humBuffer, "%d", hum);
    lcd_clrscr();
    lcd_puts("Temp: ");
    lcd_puts(tempBuffer);
    lcd_puts("C");
    lcd_gotoxy(0,1);
    lcd_puts("Humid: ");
    lcd_puts(humBuffer);
    lcd_puts("%");

    _delay_ms(DHT_TIMEOUT);
}

int main()
{
    sei();
    init_lcd();

    while(1)
    {
        init_dht11();
        print_values(currentTemp, currentHumidity);
    }
}