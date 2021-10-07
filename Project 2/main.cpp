/*-------------------------------------------------------------------------------------
*
*   File Name:      main.cpp
*   Programmer:     Nick Minor
*   Date:           2021/10/07
*   Purpose:        Blinks an external LED connected to Port C, Pin 11.
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 2, Stage 1
*   Functions:      Blink()
*   Inputs:         None
*   Outputs:        PC11: LED
*
*   Constraints:    N/A
*   Sources:        RM0432 Manual
*
*------------------------------------------------------------------------------------*/
#include "mbed.h"

//Thread controller;

//void Blink();

// Wiring
// PC_11 -> LED (short) -> Resistor -> Rail -> Ground

// main() runs in its own thread in the OS
int main()
{
    RCC->AHB2ENR |= 0x4;    // Turn on clock for Port c
    GPIOC->MODER &= ~(0x800000);    // Set mode for pin 13 to output (01)

    printf("Nick Minor\n");

    while (true) {
        GPIOC->ODR |= 0x800; // Turn output to PC11 on
        wait_us(1000000);

        GPIOC->ODR &= ~(0x800); // Turn output to PC11 off
        wait_us(1000000);
    }


    //controller.start(Blink);
    

    return 0;
}
/*
void Blink() {
    while (true) {
        GPIOC->ODR |= 0x800; // Turn output to PC11 on
        thread_sleep_for(1000);

        GPIOC->ODR &= ~(0x800); // Turn output to PC11 off
        thread_sleep_for(1000);
    }
}
*/