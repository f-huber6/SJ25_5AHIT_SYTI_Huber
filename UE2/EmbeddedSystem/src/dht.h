/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - DHT-11 Library, by Charalampos Andrianakis on 18/12/11
*/


#ifndef DHT_H_
#define DHT_H_

#include <stdio.h>
#include <avr/io.h>

//setup port: Ich verwende PORTC Pin2 => Aber es braucht keinen ADC!!!
#warning define your Pins here!
#define DHT_DDR DDRC
#define DHT_PORT PORTC
#define DHT_PIN PINC
#define DHT_INPUTPIN 2

//sensor type
#define DHT_DHT11 1
#define DHT_DHT22 2
#define DHT_TYPE DHT_DHT11 //Wir haben DHT11

#define DHT_ERROR_NOERR         (0)
#define DHT_ERROR_STARTCOND1   (-1)
#define DHT_ERROR_STARTCOND2   (-2)
#define DHT_ERROR_WAIT4HIGH    (-3)
#define DHT_ERROR_WAIT4LOW     (-4)
#define DHT_ERROR_CHECKSUM     (-5)

//enable decimal precision (float)
#if DHT_TYPE == DHT_DHT11
#define DHT_FLOAT 0  //Das war falsch im Treiber
#elif DHT_TYPE == DHT_DHT22
#define DHT_FLOAT 1
#endif

//timeout retries, mindestens 200
#define DHT_TIMEOUT 200

//functions
#if DHT_FLOAT == 1
extern int8_t dht_gettemperature(float *temperature);
extern int8_t dht_gethumidity(float *humidity);
extern int8_t dht_gettemperaturehumidity(float *temperature, float *humidity);
#elif DHT_FLOAT == 0
extern int8_t dht_gettemperature(int8_t *temperature); //Nur Temp
extern int8_t dht_gethumidity(int8_t *humidity); //Nur Humidity
extern int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity); //Beides
#endif

#endif
