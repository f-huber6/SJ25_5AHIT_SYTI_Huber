/*
 * lcd_definitions.h
 * Konfiguration fuer Atmega2560
 */

//Zum Einbinden der Dateien
// lcd.c in lcd.cpp umbenennen
// in lcd.cpp das einbinden: #include "lcd_definitions.h"
// Hier auch einbinden
// Und natürlich muss lcd.h in der Main eingebunden werden
//LCD initialisieren mit der Konstanten LCD_DISP_ON


#ifndef LCD_LCD_DEFINITIONS_H_
#define LCD_LCD_DEFINITIONS_H_

#include <avr/io.h>

#define LCD_PORT         PORTD        /**< port for the LCD lines   */
#define LCD_DATA0_PORT   LCD_PORT     /**< port for 4bit data bit 0 */
#define LCD_DATA1_PORT   LCD_PORT     /**< port for 4bit data bit 1 */
#define LCD_DATA2_PORT   LCD_PORT     /**< port for 4bit data bit 2 */
#define LCD_DATA3_PORT   LCD_PORT     /**< port for 4bit data bit 3 */
#define LCD_DATA0_PIN    PORTD4          /**< pin for 4bit data bit 0  */
#define LCD_DATA1_PIN    PORTD5          /**< pin for 4bit data bit 1  */
#define LCD_DATA2_PIN    PORTD6          /**< pin for 4bit data bit 2  */
#define LCD_DATA3_PIN    PORTD7          /**< pin for 4bit data bit 3  */
#define LCD_RS_PORT      PORTC     /**< port for RS line         */
#define LCD_RS_PIN       PORTC5          /**< pin  for RS line         */
#define LCD_RW_PORT      PORTC     /**< port for RW line         */
#define LCD_RW_PIN       PORTC4          /**< pin  for RW line         */
#define LCD_E_PORT       PORTC     /**< port for Enable line     */
#define LCD_E_PIN        PORTC3          /**< pin  for Enable line     */

#endif /* LCD_LCD_DEFINITIONS_H_ */
