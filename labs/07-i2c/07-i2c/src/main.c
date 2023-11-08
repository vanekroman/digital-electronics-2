/***********************************************************************
 * 
 * The I2C (TWI) bus scanner tests all addresses and detects devices
 * that are connected to the SDA and SCL signals.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2023 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <oled.h>
#include <math.h>

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Call function to test all I2C (TWI) combinations and send
 *           detected devices to UART.
 * Returns:  none
 * 
 * Some known devices:
 *     0x3c - OLED display
 *     0x57 - EEPROM
 *     0x5c - Temp+Humid
 *     0x68 - RTC
 *     0x68 - GY521
 *     0x76 - BME280
 *
 **********************************************************************/

/* Global variables --------------------------------------------------*/
// Declaration of "dht12" variable with structure "DHT_values_structure"
struct DHT_values_structure {
   uint8_t hum_int;
   uint8_t hum_dec;
   uint8_t temp_int;
   uint8_t temp_dec;
   uint8_t checksum;
} dht12;

// Flag for printing new data from sensor
volatile uint8_t new_sensor_data = 0;

// Avaraging variables
// number of sensor instances to avarange
#define N_DATA_AVG 10 


// Slave and internal addresses of temperature/humidity sensor DHT12
#define SENSOR_ADR 0x5c
#define SENSOR_HUM_MEM 0
#define SENSOR_TEMP_MEM 2
#define SENSOR_CHECKSUM 4


/* Function definitions ----------------------------------------------*/
/**********************************************************************
* Function: Main function where the program execution begins
* Purpose:  Wait for new data from the sensor and sent them to UART.
* Returns:  none
**********************************************************************/
int main(void)
{
    char string[2];  // String for converting numbers by itoa()

    // TWI
    twi_init();

    // OLED
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_charMode(DOUBLESIZE);
    oled_puts("OLED disp.");
    oled_charMode(NORMALSIZE);
    // oled_gotoxy(x, y)
    oled_gotoxy(0, 2);
    oled_puts("128x64, SHH1106");
    // oled_drawLine(x1, y1, x2, y2, color)
    oled_drawLine(0, 25, 120, 25, WHITE);
    oled_gotoxy(0, 4);
    oled_puts("BPC-DE2, Brno");

    // Copy buffer to display RAM
    oled_display();

    // UART
    uart_init(UART_BAUD_SELECT(115200, F_CPU));

    sei();  // Needed for UART

    // Test if sensor is ready
    if (twi_test_address(SENSOR_ADR) == 0)
    {
        uart_puts("I2C sensor detected\r\n");
        // OLED
        oled_clrscr();
        oled_gotoxy(0, 0);
        oled_puts("I2C sensor detected");
        oled_display();
    }
    else 
    {
        uart_puts("[ERROR] I2C device not detected\r\n");
        while (1);
    }

    // Timer1
    TIM1_OVF_262MS
    TIM1_OVF_ENABLE

    

    sei();

    // Infinite loop
    while (1) {
        
        static struct DHT_values_structure dht_sensors_data[N_DATA_AVG];

        if (new_sensor_data == N_DATA_AVG)
        {
            // calculate avg
            struct DHT_values_structure avg_values;
            avg_values.temp_dec = 0;
            avg_values.temp_int = 0;

            for (size_t i = 0; i < N_DATA_AVG; i++)
            {
                avg_values.temp_dec += dht_sensors_data[i].temp_dec;
                avg_values.temp_int += dht_sensors_data[i].temp_int;
            }
            avg_values.temp_dec = avg_values.temp_dec / N_DATA_AVG;
            avg_values.temp_int = avg_values.temp_int / N_DATA_AVG;

            // display avg value
            itoa(avg_values.temp_int, string, 10);
            uart_puts(string);
            uart_puts(".");
            itoa(avg_values.temp_dec, string, 10);
            uart_puts(string);
            uart_puts(" °C\r\n");

            new_sensor_data = 0;
        }

        if (new_sensor_data < N_DATA_AVG) 
        {
            // store data for avg
            dht_sensors_data[new_sensor_data] = dht12;
            // Do not print it again and wait for the new data
            new_sensor_data++;
        }
    }

    // Will never reach this
    return 0;
}


/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
* Function: Timer/Counter1 overflow interrupt
* Purpose:  Read temperature and humidity from DHT12, SLA = 0x5c.
**********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // Test ACK from sensor
    twi_start();
    if (twi_write((SENSOR_ADR<<1) | TWI_WRITE) == 0) {
        // Set internal memory location
        twi_write(SENSOR_TEMP_MEM);
        twi_stop();
        // Read data from internal memory
        twi_start();
        twi_write((SENSOR_ADR<<1) | TWI_READ);
        dht12.temp_int = twi_read(TWI_ACK);
        dht12.temp_dec = twi_read(TWI_NACK);

        new_sensor_data = 1;
    }
    twi_stop();
}