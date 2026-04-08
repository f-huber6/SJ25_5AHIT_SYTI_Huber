#define F_CPU 16000000UL
//Start Text sozusagen
#define STX 0x02

//End Text sozusagen
#define ETX 0x03

//Ackknowledge
#define ACK 0x06
#define MAX_RETRIES 3

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "dht.h"
#include <stdio.h>
#include <util/delay.h>

int8_t currentTemp;
int8_t currentHumidity;
int8_t errorStatus;

volatile uint8_t send_Interval = 1;
volatile uint8_t second_counter = 0;
volatile char last_rx_char = 0;

volatile uint8_t ack_received = 0;
volatile uint8_t retry_count = 0;
volatile uint8_t sending_enabled = 1;
volatile uint8_t sequence_number = 0;
volatile uint8_t waiting_for_ack = 0;
volatile uint8_t ack_flag = 0;
volatile uint8_t fan_state = 0;

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
}

void init_timer1()
{
    TCCR1B |= (1 << WGM12);   // CTC
    TCCR1B |= (1 << CS12);    // Prescaler 256
    OCR1A = 62499;            // 1 Sekunde bei 16 MHz
    TIMSK1 |= (1 << OCIE1A);
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
            sequence_number);

    uart_sendChar(STX);
    uart_sendString(buffer);
    uart_sendChar(ETX);
}

void send_fan_status()
{
    char buffer[20];

    sprintf(buffer, "FAN%d", fan_state);

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

    DDRC |= (1 << PC1);
    PORTC &= ~(1 << PC1);

    DDRD |= (1 << PD2);
    PORTD &= ~(1 << PD2);

    sei();

    while(1)
    {
        init_dht11();
        print_values(currentTemp, currentHumidity);
        process_uart();

        if(ack_flag)
        {
            sequence_number++;
            waiting_for_ack = 0;
            retry_count = 0;
            ack_flag = 0;
        }
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

        if(!waiting_for_ack)
        {
            send_data_message();
            waiting_for_ack = 1;
            retry_count = 0;
        }
        else
        {
            if(retry_count < MAX_RETRIES)
            {
                send_data_message();
                retry_count++;
            }
            else
            {
                sending_enabled = 0;
                PORTC |= (1 << PC1);
            }
        }
    }
}


ISR(USART_RX_vect)
{
    const char rx = UDR0;

    if(rx == ACK)
    {
        ack_flag = 1;
    }

    else if(rx == 'r')
    {
        sending_enabled = 1;
        retry_count = 0;
        waiting_for_ack = 0;
        PORTC &= ~(1 << PC1);
    }

    else if(rx == 'd')
    {
        sending_enabled = 1;
        retry_count = 0;
        waiting_for_ack = 0;
        second_counter = 0;
        PORTC &= ~(1 << PC1);
    }

    else if(rx == 'q')
    {
        sending_enabled = 0;
        retry_count = 0;
        waiting_for_ack = 0;
        second_counter = 0;
        PORTC |= (1 << PC1);
    }

    else if(rx == 'e')
    {
        fan_state = 1;
        PORTD |= (1 << PD2);
    }

    else if(rx == 'a')
    {
        fan_state = 0;
        PORTD &= ~(1 << PD2);
    }

    else if(rx == 's')
    {
        send_fan_status();
    }

    else
    {
        last_rx_char = rx;
    }
}