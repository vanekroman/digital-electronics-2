#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC

typedef struct
{
    volatile uint8_t *reg;
    uint8_t pin;

}GPIO_PIN;

// Used pin definitions
GPIO_PIN pin12 = {&DDRB, 2};
GPIO_PIN pin13 = {&DDRB, 3};

int main(void)
{
    TIM0_OVF_4MS
    TIM2_OVF_2MS
    
    // Enable overflow interrupt
    TIM0_OVF_ENABLE
    TIM2_OVF_ENABLE
    
    // Enables interrupts by setting the global interrupt mask
    sei();

    // Setup
    GPIO_mode_output(pin12.reg, pin12.pin);
    GPIO_mode_output(pin13.reg, pin13.pin);

    for(;;)
    {
        /* code */
    }
}

// Interrupt service routines
ISR(TIMER0_OVF_vect)
{
    static uint8_t n_overflows = 0;
    n_overflows++;

    // 400 ms timer
    if(n_overflows < 100)
    {
        n_overflows = 0;
        GPIO_toggle(pin12.reg, pin12.pin);
    }
}

ISR(TIMER2_OVF_vect)
{
    static uint8_t n_overflows = 0;
    n_overflows++;

    // 200 ms timer
    if(n_overflows < 100)
    {
        n_overflows = 0;
        GPIO_toggle(pin13.reg, pin13.pin);
    }
}