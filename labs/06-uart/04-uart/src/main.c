/***********************************************************************
 * 
 * Use USART unit and transmit data between ATmega328P and computer.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2018 Tomas Fryza
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
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions

// UART terminal intro
char* terminal_intro = "______________________________\n         UART TERMINAL           \n______________________________\r\n";

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data four times 
 *           per second.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize USART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    
    // Configure 16-bit Timer/Counter1 to transmit UART data
    // Set prescaler to 262 ms and enable overflow interrupt
    TIM1_OVF_262MS;
    TIM1_OVF_ENABLE


    // Enables interrupts by setting the global interrupt mask
    sei();

    // Put strings to ringbuffer for transmitting via UART
    uart_puts(terminal_intro);

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Transmit UART data four times per second.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // Transmit UART string(s)
    uint8_t value;
    // Strings for converted numbers by itoa()
    char string_dec[8];  
    char string_hex[8];
    char string_bin[8];
    
    static uint8_t i = 0;

    value = uart_getc();
    if (value != '\0') {  // Data available from UART
        // Display ASCII code of received character
        itoa(value, string_dec, 10);
        itoa(value, string_hex, 16);
        itoa(value, string_bin, 2);
        // Style of output string
        if(i%2) uart_puts("\x1b[4;32m");
        else uart_puts("\x1b[0m");
        i++;

        // Format response
        uart_puts(string_dec);
        if(value < 100) uart_puts(" ");
        uart_puts("  |  0x");
        uart_puts(string_hex);
        uart_puts("  |  0b");
        uart_puts(string_bin);
        uart_puts("\r\n");
    }
}