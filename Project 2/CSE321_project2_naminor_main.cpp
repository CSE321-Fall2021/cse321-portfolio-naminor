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

EventQueue q(32 * EVENTS_EVENT_SIZE);
//CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);

// Interrupt Service Routines:
void isr_col1(void);    // Handler for column 1 of the matrix keypad: 1, 4, 7, *
void isr_col2(void);    // Column 2:    2, 5, 8, 0
void isr_col3(void);    // Column 3:    3, 6, 9, #
void isr_col4(void);    // Column 4:    A, B, C, D

void isrA_StartTimer(void);
void isrB_StopTimer(void);
void isrD_SetTimer(void);

int row = 1;    // Tracks the current row

// When the button is pressed, the switch closes and the circuit completes
InterruptIn col1(PC_6, PullDown);     // Column 1 of keypad is attached to PC6
InterruptIn col2(PB_15, PullDown);
InterruptIn col3(PB_8, PullDown);
InterruptIn col4(PB_9, PullDown);
// NEED GPIO OUT

int main() {
    // Ports B and A used for outputs to power rows, Ports B and C for input
    RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
    // Set pins giving power to General Purpose Output mode (01)
    // Does interruptin do this already? find out
    GPIOB->MODER |= 0x100;          // PB_4, row 1
    GPIOB->MODER &= ~(0x200);
    GPIOA->MODER |= 0x100;          // PA_4, row 2
    GPIOA->MODER &= ~(0x200);
    GPIOB->MODER |= 0x40;           // PB_3, row 3
    GPIOB->MODER &= ~(0x80);
    GPIOB->MODER |= 0x400;          // PB_5, row 4
    GPIOB->MODER &= ~(0x800);
    
    // Set the pins reading if a column was pressed to input mode (00)
   /* GPIOC->MODER &= ~(0x3000);      // PC_6
    GPIOB->MODER &= ~(0xC0000000);  // PB_15
    GPIOB->MODER &= ~(0x30000);     // PB_8
    GPIOB->MODER &= ~(0xC0000);     // PB_9*/
    
    // Call ISRs on rising edges, when a key in the column is pressed.
 /*   col1.rise(q.event(isr_col1));
    col2.rise(q.event(isr_col2));
    col3.rise(q.event(isr_col3));
    col4.rise(q.event(isr_col4));*/

    col1.rise(&isr_col1);
    col2.rise(&isr_col2);
    col3.rise(&isr_col3);
    col4.rise(&isr_col4);

    printf("Starting...\n");
    q.call(printf, "Testing call\n");
    q.dispatch_once();
    //LCD.begin();

    while (true) {
        if (row == 1) {
            //printf("row1\n");
            GPIOB->ODR |= 0x10;     // Turn on  PB_4, row 1
            GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
            GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
            GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
            wait_us(1000);
        }
        else if (row == 2) {
            //printf("row2\n");
            GPIOA->ODR |= 0x10;     // Turn on  PA_4, row 2
            GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
            GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
            GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
            wait_us(1000);
        }
        else if (row == 3) {
           //printf("row3\n");
            GPIOB->ODR |= 0x8;      // Turn on  PB_3, row 3
            GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
            GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
            GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
            wait_us(1000);
        }
        else {
            //printf("row4\n");
            //q.call(printf, "Pressed 1\n");
            GPIOB->ODR |= 0x20;     // Turn on  PB_5, row 4
            GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
            GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
            GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
            wait_us(1000);
        }
        wait_us(50000);
        row %= 4;
        row++;

    }
   // q.dispatch_forever();
    return 0;
}


void isr_col1(void) {
    //q.call(printf, "col1: 1, 4, 7, *\n");
    //printf("col1: 1, 4, 7, *\n");
    if (row == 1) {         // 1
        q.call(printf, "Pressed 1\n");
        q.dispatch_once();
        //printf("Pressed 1\n");
    }
    else if (row == 2) {    // 4
        q.call(printf, "Pressed 4\n");
        q.dispatch_once();
        //printf("Pressed 4\n");
    }
    else if (row == 3) {    // 7
        q.call(printf, "Pressed 7\n");
        q.dispatch_once();
        //printf("Pressed 7\n");
    }
    else {    // *
        q.call(printf, "Pressed *\n");
        q.dispatch_once();
        //printf("Pressed *\n");
    }
    wait_us(5000);
}

void isr_col2(void) {
    //q.call(printf, "col2: 2, 5, 8, 0\n");
    //printf("col2: 2, 5, 8, 0\n");
    if (row == 1) {         // 2
        q.call(printf, "Pressed 2\n");
        q.dispatch_once();
        //printf("Pressed 2\n");
    }
    else if (row == 2) {    // 5
        q.call(printf, "Pressed 5\n");
        q.dispatch_once();
        //printf("Pressed 5\n");
    }
    else if (row == 3) {    // 8
        q.call(printf, "Pressed 8\n");
        q.dispatch_once();
        //printf("Pressed 8\n");
    }
    else {    // 0
        q.call(printf, "Pressed 0\n");
        q.dispatch_once();
        //printf("Pressed 0\n");
    }
    wait_us(5000);
}

void isr_col3(void) {
    //q.call(printf, "col3: 3, 6, 9, #\n");
    //printf("col3: 3, 6, 9, #\n");
    if (row == 1) {         // 3
        q.call(printf, "Pressed 3\n");
        q.dispatch_once();
        //printf("Pressed 3\n");
    }
    else if (row == 2) {    // 6
        q.call(printf, "Pressed 6\n");
        q.dispatch_once();
        //printf("Pressed 6\n");
    }
    else if (row == 3) {    // 9
        q.call(printf, "Pressed 9\n");
        q.dispatch_once();
        //printf("Pressed 9\n");
    }
    else {    // #
        q.call(printf, "Pressed #\n");
        q.dispatch_once();
        //printf("Pressed #\n");
    }
    wait_us(5000);
}

void isr_col4(void) {
   // q.call(printf, "col4: A, B, C, D\n");
   //printf("col4: A, B, C, D\n");
    if (row == 1) {         // A
        q.call(printf, "Pressed A\n");
        q.dispatch_once();
        //printf("Pressed A\n");
    }
    else if (row == 2) {    // B
        q.call(printf, "Pressed B\n");
        q.dispatch_once();
        //printf("Pressed B\n");
    }
    else if (row == 3) {    // C
        q.call(printf, "Pressed C\n");
        q.dispatch_once();
        //printf("Pressed C\n");
    }
    else {    // D
        q.call(printf, "Pressed D\n");
        q.dispatch_once();
        //printf("Pressed D\n");
    }
    wait_us(5000);
}

void isrA_StartTimer(void) {

}

void isrB_StopTimer(void) {

}

void isrD_SetTimer(void) {

}