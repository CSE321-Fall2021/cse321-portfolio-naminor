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
#include <chrono>
#include <string>

EventQueue q(32 * EVENTS_EVENT_SIZE);
CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);
Timeout countdown;
Ticker tick;


class LCDClock {
private:
    int currTime;
    int input[3];
public:
    int output[3];
    bool allowTimeInput;
    bool timerOn;
    bool allowPrint;
    LCDClock() {
        currTime = 0;
        allowTimeInput = false;
        allowPrint = false;
        timerOn = false;
        for (int i = 0; i < 3; i++) {
            input[i] = -1;
            output[i] = -1;
        }
    }
    int calcTime() {
        currTime = 0;
        // Minutes : Seconds Seconds
        if (input[0] != -1)
            currTime += input[0] * 60;  // Minutes
        if (input[1] != -1)
            currTime += input[1] * 10;  // Seconds
        if (input[2] != -1)
            currTime += input[2];       // Seconds
        return currTime;
    }
    void insertValue(int val) {
        if (input[2] == -1) {   // No digit has been inserted
            input[2] = val;

            output[2] = val;
        } else if (input[1] == -1) {
            input[1] = input[2];
            input[2] = val;

            output[1] = output[2];
            output[2] = val;
        } else if (input[0] == -1) {
            input[0] = input[1];
            input[1] = input[2];
            input[2] = val;

            output[0] = output[1];
            output[1] = output[2];
            output[2] = val;
        }
    }
    void resetInput() {
        for (int i = 0; i < 3; i++) { 
            input[i] = -1;
            output[i] = -1;
        }
    }
    int* getInput() {
        return input;
    }
    int* getOutput() {
        return output;
    }
    void getOutputChars(char printChars[5]) {
        currTime--;
        if (output[2] > 0) {
            output[2]--;
        }
        else if (output[1] > 0) {
            output[2] = 9;
            output[1]--;
        } else if (output[0] > 0) {
            output[2] = 9;
            output[1] = 5;
            output[0]--;
        }
        for (int i = 0; i < 3; i ++) {
            if (output[i] == -1)
                output[i] = 0;
        }
        printf("Time: %d:%d%d\n", output[0], output[1], output[2]);

        printChars[0] = output[0] + '0';
        printChars[1] = ':';
        printChars[2] = output[1] + '0';
        printChars[3] = output[2] + '0';
        printChars[4] = '\0';
    }
    int getTime() {return currTime;}
    void setTime(int time) {currTime = time;}
    void decrTime() {
        currTime--;
        printf("Current time: %d\n", currTime);
        //return currTime;
    }
    void TimesUp() {
        printf("Time's Up\n");
        resetInput();
    }
} cl;

void remainingTime(void);

Ticker cycler;  // Ticket that will cycle through rows
void rowCycler(void);   // Cycles the rows once
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
InterruptIn col1(PC_11, PullDown);     // Column 1 of keypad is attached to PC6
InterruptIn col2(PC_10, PullDown);
InterruptIn col3(PC_9, PullDown);
InterruptIn col4(PC_8, PullDown);

int main() {
    // Ports B and A used for outputs to power rows, Ports B and C for input
    RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
    // Set pins giving power to General Purpose Output mode (01)
    GPIOB->MODER |= 0x100;          // PB_4, row 1
    GPIOB->MODER &= ~(0x200);
    GPIOA->MODER |= 0x100;          // PA_4, row 2
    GPIOA->MODER &= ~(0x200);
    GPIOB->MODER |= 0x40;           // PB_3, row 3
    GPIOB->MODER &= ~(0x80);
    GPIOB->MODER |= 0x400;          // PB_5, row 4
    GPIOB->MODER &= ~(0x800);

    col1.rise(&isr_col1);
    col2.rise(&isr_col2);
    col3.rise(&isr_col3);
    col4.rise(&isr_col4);

    printf("Starting...\n");
    q.call(printf, "Starting LCD\n");
    q.dispatch_once();
    LCD.begin();
    /*LCD.setCursor(0, 0);
    LCD.print("Test");
    LCD.setCursor(0,1);
    LCD.print("Hello world");*/
    cycler.attach(&rowCycler, 80ms);
    //cl.setTime(10);
    tick.attach(&remainingTime, 1s);
    //int tl0 = 10;
    //countdown.attach(callback(&cl, &LCDClock::TimesUp), (chrono::seconds)tl0);

    while (true) { 
        if (cl.getTime() > 0 && cl.timerOn == true && cl.allowPrint == true) {
            char getChars[5];
            cl.getOutputChars(getChars);
            LCD.setCursor(0, 0);
            LCD.print("Time Remaining");
            LCD.setCursor(0, 1);
            LCD.print(getChars);
            cl.allowPrint = false;
        }
        if (cl.getTime() == 0 && cl.timerOn == true) {
            LCD.setCursor(0,0);
            LCD.clear();
            LCD.print("Time's Up");
        }
    }

    return 0;
}

void remainingTime() {
    if ((countdown.remaining_time() >= (chrono::microseconds)0) && cl.timerOn == true){
        //chrono::microseconds timeLeft = countdown.remaining_time() / 1000000; // Converts us to s
        //timeLeft++;  // Add 1 since the remaining time is rounded down in the us to s conversion
        cl.allowPrint = true;
        //chrono::seconds secondsLeft = std::chrono::duration_cast<std::chrono::seconds> (countdown.remaining_time());
/*
        if (cl.output[2] > 0) {
            cl.output[2]--;
        }
        else if (cl.output[1] > 0) {
            cl.output[2] = 9;
            cl.output[1]--;
        } else if (cl.output[0] > 0) {
            cl.output[2] = 9;
            cl.output[1] = 5;
            cl.output[0]--;
        }
        for (int i = 0; i < 3; i ++) {
            if (cl.output[i] == -1)
                cl.output[i] = 0;
        }
        //printf("Time: %d:%d%d\n", cl.output[0], cl.output[1], cl.output[2]);
        char printChars[4];
        for (int i = 0; i < 3; i++) {
            printChars[i] = cl.output[i] + '0';
        }
        printChars[3] = '\0';
        printf("Time: %c:%c%c\n", printChars[0], printChars[1], printChars[2]);
*/
       /* LCD.setCursor(0, 0);
        LCD.print("Time Remaining");

        LCD.setCursor(0, 1);
        LCD.clear();
        LCD.print(printChars);*/


        //printf("Remaining time: %llu\n", secondsLeft);
        
 
        //LCD.print(printChars);
        //printf("Remaining time: %llu\n", countdown.remaining_time()/1000000);
    }
}

void rowCycler() {
    if (row == 1) {
        //printf("row1\n");
        GPIOB->ODR |= 0x10;     // Turn on  PB_4, row 1
        GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
        GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    }
    else if (row == 2) {
        //printf("row2\n");
        GPIOA->ODR |= 0x10;     // Turn on  PA_4, row 2
        GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
        GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    }
    else if (row == 3) {
        //printf("row3\n");
        GPIOB->ODR |= 0x8;      // Turn on  PB_3, row 3
        GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
        GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
        GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    }
    else {
        //printf("row4\n");
        //q.call(printf, "Pressed 1\n");
        GPIOB->ODR |= 0x20;     // Turn on  PB_5, row 4
        GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
        GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
    }
    row++;
    row %= 4;
}

void isr_col1(void) {
    //q.call(printf, "col1: 1, 4, 7, *\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 1
            q.call(printf, "Pressed 1\n");
            q.dispatch_once();
            cl.insertValue(1);
        }
        else if (row == 3) {    // 4
            q.call(printf, "Pressed 4\n");
            q.dispatch_once();
            cl.insertValue(4);
        }
        else if (row == 0) {    // 7
            q.call(printf, "Pressed 7\n");
            q.dispatch_once();
            cl.insertValue(7);
        }
        else {    // *
            q.call(printf, "Pressed *\n");
            q.dispatch_once();
        }
    }
}

void isr_col2(void) {
    //q.call(printf, "col2: 2, 5, 8, 0\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 2
            q.call(printf, "Pressed 2\n");
            q.dispatch_once();
            cl.insertValue(2);
        }
        else if (row == 3) {    // 5
            q.call(printf, "Pressed 5\n");
            q.dispatch_once();
            cl.insertValue(5);
        }
        else if (row == 0) {    // 8
            q.call(printf, "Pressed 8\n");
            q.dispatch_once();
            cl.insertValue(8);
        }
        else {    // 0
            q.call(printf, "Pressed 0\n");
            q.dispatch_once();
            cl.insertValue(0);
        }
    }
}

void isr_col3(void) {
    //q.call(printf, "col3: 3, 6, 9, #\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 3
            q.call(printf, "Pressed 3\n");
            q.dispatch_once();
            cl.insertValue(3);
        }
        else if (row == 3) {    // 6
            q.call(printf, "Pressed 6\n");
            q.dispatch_once();
            cl.insertValue(6);
        }
        else if (row == 0) {    // 9
            q.call(printf, "Pressed 9\n");
            q.dispatch_once();
            cl.insertValue(9);
        }
        else {    // #
            q.call(printf, "Pressed #\n");
            q.dispatch_once();
        }
    }
}

void isr_col4(void) {
   // q.call(printf, "col4: A, B, C, D\n");
   //printf("col4: A, B, C, D\n");
    if (row == 2) {         // A
        q.call(printf, "Pressed A\n");
        q.dispatch_once();
        q.call(isrA_StartTimer);
        q.dispatch_once();
    }
    else if (row == 3) {    // B
        q.call(printf, "Pressed B\n");
        q.dispatch_once();
        q.call(isrB_StopTimer);
        q.dispatch_once();
    }
    else if (row == 0) {    // C
        q.call(printf, "Pressed C\n");
        q.dispatch_once();
    }
    else {    // D
        q.call(printf, "Pressed D\n");
        q.dispatch_once();
        q.call(isrD_SetTimer);
        q.dispatch_once();
    }
}

void isrA_StartTimer(void) {
    cl.allowTimeInput = false;
    cl.timerOn = true;
    printf("Time set: %d\n", cl.calcTime());
    cl.setTime(cl.calcTime());  
    countdown.attach(callback(&cl, &LCDClock::TimesUp), (chrono::seconds)cl.getTime());
}

void isrB_StopTimer(void) {
    countdown.detach();         // Timeout no longer has a function to execute once it finishes counting
    cl.timerOn = false;         // Turn the timer countdown off
}

void isrD_SetTimer(void) {
    cl.resetInput();            // Reset the current input values
    cl.allowTimeInput = true;   // Allow user to enter new input values
    q.call(printf, "Setting timer...\n");
    q.dispatch_once();


}