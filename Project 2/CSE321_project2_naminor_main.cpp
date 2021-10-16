/*-------------------------------------------------------------------------------------
*
*   File Name:      CSE321_project2_naminor_main.cpp
*   Programmer:     Nick Minor
*   Date:           2021/10/15
*   Purpose:        The program is for a bare-metal and user-programmable count-down alarm 
*                   system. There is functionality for the user to input the time duration 
*                   of the timer as well as starting and stopping the timer. The time 
*                   remaining on the timer and the completion of the timer is communicated 
*                   to the user via an LCD screen and LEDs.
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 2
*   Functions:      isr_col1, isr_col2, isr_col3, isr_col4
*   Inputs:         Matrix keypad
*   Outputs:        LCD screen, LEDs
*
*   Constraints:    Time is represented as m:ss (valid times between 0:00 and 9:59),
*                   System runs forever
*   Sources:        RM0432 Manual
*                   JDH_1804_Datasheet
*                   CSE321 LCD Library Files
*                   MbedOS API Documentation: 
*	                   https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
*
*------------------------------------------------------------------------------------*/
#include "mbed.h"
#include "1802.h"

// Interrupt Service Routines:
void isr_col1(void);    // Handler for column 1 of the matrix keypad: 1, 4, 7, *
void isr_col2(void);    // Column 2:    2, 5, 8, 0
void isr_col3(void);    // Column 3:    3, 6, 9, #
void isr_col4(void);    // Column 4:    A, B, C, D

int row = 0;    // Tracks the current row

// When the button is pressed, the switch closes and the circuit completes
InterruptIn col1(PB_6);     // Column 1 of keypad is attached to PB6
InterruptIn col2(PB_7);
InterruptIn col3(PB_8);
InterruptIn col4(PB_9);

int main() {
    // Port C used for outputs to LEDs, pins 6, 7, 8 and 9
    RCC->AHB2ENR |= 0x4;            // Turn on clock for Port c
    // Set pins 6, 7, 8, 9 to General Purpose Output mode (01)
    GPIOC->MODER |= 0x55000;        
    GPIOC->MODER &= ~(0xAA000);

    // Call ISRs on rising edges, when a key in the column is pressed.
    col1.rise(&isr_col1);
    col2.rise(&isr_col2);
    col3.rise(&isr_col3);
    col4.rise(&isr_col4);

    while (true) {
        if (row == 1) {
            GPIOC->ODR |= 0x40;     // Turn on pin 6
            GPIOC->ODR &= ~(0x80);  // Turn off pin 7
            GPIOC->ODR &= ~(0x100); // Turn off pin 8
            GPIOC->ODR &= ~(0x200); // Turn off pin 9
        }
        else if (row == 2) {
            GPIOC->ODR |= 0x80;     // Turn on pin 7
            GPIOC->ODR &= ~(0x40);  // Turn off pin 6
            GPIOC->ODR &= ~(0x100); // Turn off pin 8
            GPIOC->ODR &= ~(0x200); // Turn off pin 9
        }
        else if (row == 3) {
            GPIOC->ODR |= 0x100;    // Turn on pin 8
            GPIOC->ODR &= ~(0x40);  // Turn off pin 6
            GPIOC->ODR &= ~(0x80);  // Turn off pin 7
            GPIOC->ODR &= ~(0x200); // Turn off pin 9
        }
        else {
            GPIOC->ODR |= 0x200;    // Turn on pin 9
            GPIOC->ODR &= ~(0x40);  // Turn off pin 6
            GPIOC->ODR &= ~(0x80);  // Turn off pin 7
            GPIOC->ODR &= ~(0x100); // Turn off pin 8
        }
        row++;
        row %= 4;
        wait_us(50000);

    }

}


void isr_col1(void) {
    printf("col1: 1, 4, 7, *\n");
    if (row == 1) {
        printf("Pressed 1\n");
    }
    else if (row == 2) {
        printf("Pressed 4\n");
    }
    else if (row == 3) {
        printf("Pressed 7\n");
    }
    else if (row == 4) {
        printf("Pressed *\n");
    }
}

void isr_col2(void) {
    printf("col2: 2, 5, 8, 0\n");
    if (row == 1) {
        printf("Pressed 2\n");
    }
    else if (row == 2) {
        printf("Pressed 5\n");
    }
    else if (row == 3) {
        printf("Pressed 8\n");
    }
    else if (row == 4) {
        printf("Pressed 0\n");
    }
}

void isr_col3(void) {
    printf("col3: 3, 6, 9, #\n");
    if (row == 1) {
        printf("Pressed 3\n");
    }
    else if (row == 2) {
        printf("Pressed 6\n");
    }
    else if (row == 3) {
        printf("Pressed 9\n");
    }
    else if (row == 4) {
        printf("Pressed #\n");
    }
}

void isr_col4(void) {
    printf("col4: A, B, C, D\n");
    if (row == 1) {
        printf("Pressed A\n");
    }
    else if (row == 2) {
        printf("Pressed B\n");
    }
    else if (row == 3) {
        printf("Pressed C\n");
    }
    else if (row == 4) {
        printf("Pressed D\n");
    }
}