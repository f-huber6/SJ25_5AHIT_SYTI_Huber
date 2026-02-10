#include <Arduino.h>
#include "lcd.h"
#include "dht.h"
#include <util/delay.h>

#define F_CPU 16000000UL

//Start Text sozusagem
#define STX 0x02

//End Text sozusagen
#define ETX 0x03

#define ACK 0x06
#define MAX_RETRIES 3

//Mit <>-Klammern würde er im Compiler-Standardpfad suchen, mit "" sucht er im Projektverzeichnis
//DHT11 liefert nur Ganzzahlen, DHT22 auch Nachkommastellen

int8_t currentTemp;
int8_t currentHumidity;
int8_t errorStatus;

//Variablen für Step 3
volatile uint8_t send_Interval = 1;
volatile uint8_t second_counter = 0;
volatile char last_rx_char = 0;

volatile uint8_t seq_num = 1;
volatile uint8_t ack_received = 0;
volatile uint8_t retry_count = 0;
volatile uint8_t sending_enabled = 1;

//ASCII-Zeichen
//0x31 = 1, 0x34 = 4

void uart_init()
{
    //Baudrate: 9600 bei 16MHz --> UBRR = 103
    UBRR0H = 0;
    UBRR0L = 103;

    //Senden und Empfangen aktivieren
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);

    //8 Datenbits, 1 Stopbit, kein Paritätsbit
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

    //UART-Interrupt aktivieren - Step3
    UCSR0B |= (1 << RXCIE0);
}

void uart_sendChar(char s)
{
    //Warten, bis Buffer leer
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = s;
}

void uart_sendString(const char *s)
{
    while(*s)
    {
        uart_sendChar(*s++);
    }
}

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

void init_timer1()
{
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 62499;
}

void process_uart()
{
    if (last_rx_char == 0)
        return;

    if (last_rx_char == '1')
        send_Interval = 1;
    else if (last_rx_char == '4')
        send_Interval = 4;

    last_rx_char = 0;
}

void send_data_message()
{
    char buffer[40];

    sprintf(buffer,
        "DATE%d|HU%d|SN%d",
            currentTemp,
            currentHumidity,
            seq_num);

    uart_sendChar(STX);
    uart_sendString(buffer);
    uart_sendChar(ETX);
}

int main()
{
    cli();
    uart_init();
    init_lcd();
    init_timer1();
    sei();

    while(1)
    {
        init_dht11();
        print_values(currentTemp, currentHumidity);
        process_uart();
    }
}

ISR(TIMER1_COMPA_vect)
{
    if(!sending_enabled)
        return;

    second_counter++;

    if(second_counter >= send_Interval)
    {
        second_counter = 0;

        if(!ack_received)
        {
            if(retry_count >= MAX_RETRIES)
            {
                sending_enabled = 0;
                return;
            }

            send_data_message();
            retry_count++;
        }

        else
        {
            ack_received = 0;
            retry_count = 0;
            seq_num++;

            send_data_message();
        }

        char buffer[10];
        sprintf(buffer, "%d%d", currentTemp, currentHumidity);

        uart_sendChar(STX);
        uart_sendString(buffer);
        uart_sendChar(ETX);
    }
}

ISR(USART_RX_vect)
{
    const char rx = UDR0;

    if(rx == ACK)
    {
        ack_received = 1;
    }

    else if(rx == 'r')
    {
        sending_enabled = 1;
        retry_count = 0;
        ack_received = 0;
    }

    else
    {
        last_rx_char = rx;
    }

}