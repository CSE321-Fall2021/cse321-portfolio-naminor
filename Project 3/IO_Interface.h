/*-------------------------------------------------------------------------------------
*
*   File Name:      IO_Interface.h
*   Programmer:     Nick Minor
*   Date:           2021 12 08
*   Purpose:        Contains IO_Interface class used for 
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 3
*   Functions:      
*   Inputs:         
*   Outputs:        
*
*   Constraints:    
*   Sources:        
*
*------------------------------------------------------------------------------------*/
#include "mbed.h"
enum {INPUT_MODE = 1, WARNING_MODE, ALARM_MODE}; // Enums for input modes. (1, 2, 3)
class IO_Interface {
private:
    int input[3];
    int mode;

public:
    int currDist;   // Holds the current distance from the object in cm
    int alarmDist;  // Holds the value that the user would like the buzzer to alarm at
    char lcd_output[12];    // Used to output distance to the LCD in cm
    bool timerCounting;
    bool allowUserInput;
    //enum {INPUT_MODE = 1, WARNING_MODE, ALARM_MODE}; // Enums for input modes. (1, 2, 3)

    IO_Interface() {
        currDist = 0;
        alarmDist = 100; // Buzzers ring at 1 meter by default
        mode = 0;
        for (int i = 0; i < 3; i++) {
            input[i] = -1;  // -1 means the user hasn't entered a number for that digit yet
            //lcd_output[i] = -1;
        }
        timerCounting = false;
        allowUserInput = false;
    }
    void changeMode(int newMode) {mode = newMode;}
    int calcDistMetric() {  // Calculates current distance from the input array
        alarmDist = 0;
        if (input[0] != -1)
            alarmDist += input[0] * 100;
        if (input[1] != -1)
            alarmDist += input[1] * 10;
        if (input[2] != -1)
            alarmDist += input[2] * 1;
        printf("New distance: %d\n", alarmDist);
        return alarmDist;
    }
    void resetAlarmDist() {
        for (int i = 0; i < 4; i++) { 
            input[i] = -1;   // -1 means the user hasn't entered a number for that digit yet
            //output[i] = -1;
        }
    }
    void insertAlarmDist(int val) { // Inserts a user's input from the keypad 
        if (input[2] == -1) {
            input[2] = val;
            //output[3] = val;
        }
        // This conditional is new, check this if there are issues
        else if (input[1] == -1) {   // No digit has been inserted
            input[1] = input[2];
            input[2] = val;

            //output[2] = output[3];
            //output[3] = val;
        } else if (input[0] == -1) {
            input[0] = input[1];
            input[1] = input[2];
            input[2] = val;

            //output[1] = output[2];
            //output[2] = val;
        }
    }
    void getOutputChars() {
        for (int i = 0; i < 3; i++) {
            lcd_output[i] = 0;
        }
        int conversion = currDist;
        int conv2 = alarmDist;

        if (conversion >= 100) {    // 100's place
            lcd_output[0] = 0;
            while (conversion - 100 >= 0) {
                lcd_output[0]++;
                conversion -= 100;
            }
        }
        if (conversion >= 10) { // 10's place
            lcd_output[1] = 0;
            while (conversion - 10 >= 0) {
                lcd_output[1]++;
                conversion -= 10;
            }
        }
        if (conversion > 0) {   // 1's place
            lcd_output[2] = 0;
            while (conversion - 1 >= 0) {
                lcd_output[2]++;
                conversion -= 1;
            }
        }

        if (lcd_output[0] == 0 && lcd_output[1] == 0) {
            lcd_output[0] = ' ';
            lcd_output[1] = ' ';
        } else if (lcd_output[0] == 0) {
            lcd_output[0] = ' ';
        }

        for (int i = 0; i < 3; i ++) {
            if (lcd_output[i] != ' ') {
                lcd_output[i] += '0';
            }
        }

        lcd_output[3] = ' '; lcd_output[4] = '/'; lcd_output[5] = ' ';
        if (input[0] == -1 && input[1] == -1 && input[2] == -1) {
            lcd_output[6] = '1'; lcd_output[7] = '0'; lcd_output[8] = '0';
        }
        else {
            if (input[0] == -1)
                lcd_output[6] = ' ';
            else
                lcd_output[6] = input[0] + '0'; 
            if (input[1] == -1)
                lcd_output[7] = ' ';
            else
                lcd_output[7] = input[1] + '0'; 
            lcd_output[8] = input[2] + '0';
        }
        lcd_output[9] = ' '; lcd_output[10] = 'c'; lcd_output[11] = 'm';  // Append " cm"
        
        //lcd_output[3] = ' '; lcd_output[4] = 'c'; lcd_output[5] = 'm';  // Append " cm"
        
        //printf(" %c%c%c cm \n\r",lcd_output[0], lcd_output[1], lcd_output[2]);
    }

    
    void ringBuzzer() { // Can use pwm
    // try frequency between 2k and 5k?
    // Set pin used with buzzer to high
        GPIOB->ODR &= ~(0x4);   // Turn on the buzzer

    }
    void silenceBuzzer() {
    // Set pin used with buzzer to low
        GPIOB->ODR |= 0x4; 
    }
};


//EventQueue q(32 * EVENTS_EVENT_SIZE);
class MatrixKeypad : IO_Interface {
public:
    int row;
    MatrixKeypad() {
        // Ports B and A used for outputs to power rows, Ports B and C for input
        RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
        // Set pins giving power to Matrix Keypad rows to General Purpose Output mode (01)
        GPIOB->MODER |= 0x100;          // PB_4, row 1
        GPIOB->MODER &= ~(0x200);
        GPIOA->MODER |= 0x100;          // PA_4, row 2
        GPIOA->MODER &= ~(0x200);
        GPIOB->MODER |= 0x40;           // PB_3, row 3
        GPIOB->MODER &= ~(0x80);
        GPIOB->MODER |= 0x400;          // PB_5, row 4
        GPIOB->MODER &= ~(0x800);
        row = 1;
    }
    
    // void isr_col1(void);    // Handler for column 1 of the matrix keypad: 1, 4, 7, *
    // void isr_col2(void);    // Column 2:    2, 5, 8, 0
    // void isr_col3(void);    // Column 3:    3, 6, 9, #
    // void isr_col4(void);    // Column 4:    A, B, C, D

    

    // void rowCycler() {
    //     if (row == 1) {
    //         //printf("row1\n");
    //         GPIOB->ODR |= 0x10;     // Turn on  PB_4, row 1
    //         GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
    //         GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
    //         GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    //     }
    //     else if (row == 2) {
    //         //printf("row2\n");
    //         GPIOA->ODR |= 0x10;     // Turn on  PA_4, row 2
    //         GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
    //         GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
    //         GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    //     }
    //     else if (row == 3) {
    //         //printf("row3\n");
    //         GPIOB->ODR |= 0x8;      // Turn on  PB_3, row 3
    //         GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
    //         GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
    //         GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    //     }
    //     else {
    //         //printf("row4\n");
    //         GPIOB->ODR |= 0x20;     // Turn on  PB_5, row 4
    //         GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
    //         GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
    //         GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
    //     }
    //     row++;
    //     row %= 4;
    // }

};

/*
	void isr_col1
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called when the InterruptIn object associated with column 1 on the 
        matrix keypad recieves a rising edge. The action taken will depend on what row was 
        supplying power to the keypad when the rising edge occured. This handler will insert 
        1, 4 or 7 into the LCDs clock input array when inputting time is allowed. (The user
        has pressed D)

    note: since row is iterated at the end of rowCycler, row will hold (row+1)%4 when the
          isr is called (eg If row 1 was supplying power, row will be 2)
*/
// inline void MatrixKeypad::isr_col1(void) {
//     //q.call(printf, "col1: 1, 4, 7, *\n");
//     if (allowUserInput == true) {
//         if (row == 2) {         // 1
//             q.call(printf, "Pressed 1\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(1);  // Insert 1 into input
//         }
//         else if (row == 3) {    // 4
//             q.call(printf, "Pressed 4\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(4);  // Insert 4 into input
//         }
//         else if (row == 0) {    // 7
//             q.call(printf, "Pressed 7\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(7);  // Insert 7 into input
//         }
//         else {    // *
//             q.call(printf, "Pressed *\n"); // Printing to serial monitor
//             q.dispatch_once();
//         }
//     }
// }

/*
	void isr_col2
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called when the InterruptIn object associated with column 2 on the 
        matrix keypad recieves a rising edge. The action taken will depend on what row was 
        supplying power to the keypad when the rising edge occured. This handler will insert 
        2, 5, 8 or 0 into the LCDs clock input array when inputting time is allowed. (The user
        has pressed D)

    note: since row is iterated at the end of rowCycler, row will hold (row+1)%4 when the
          isr is called (eg If row 1 was supplying power, row will be 2)
*/
// inline void MatrixKeypad::isr_col2(void) {
//     //q.call(printf, "col2: 2, 5, 8, 0\n");
//     if (allowUserInput == true) {
//         if (row == 2) {         // 2
//             q.call(printf, "Pressed 2\n"); // Printing to serial monitor
//             q.dispatch_once();
//             q.call(&IO_Interface::insertAlarmDist, 2);  // Insert 2 into input
//         }
//         else if (row == 3) {    // 5
//             q.call(printf, "Pressed 5\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(5);  // Insert 5 into input
//         }
//         else if (row == 0) {    // 8
//             q.call(printf, "Pressed 8\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(8);  // Insert 8 into input
//         }
//         else {    // 0
//             q.call(printf, "Pressed 0\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(0);  // Insert 0 into input
//         }
//     }
// }

/*
	void isr_col3
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called when the InterruptIn object associated with column 3 on the 
        matrix keypad recieves a rising edge. The action taken will depend on what row was 
        supplying power to the keypad when the rising edge occured. This handler will insert 
        3, 6 or 9 into the LCDs clock input array when inputting time is allowed. (The user
        has pressed D)

    note: since row is iterated at the end of rowCycler, row will hold (row+1)%4 when the
          isr is called (eg If row 1 was supplying power, row will be 2)
*/
// inline void MatrixKeypad::isr_col3(void) {
//     //q.call(printf, "col3: 3, 6, 9, #\n");
//     if (allowUserInput == true) {
//         if (row == 2) {         // 3
//             q.call(printf, "Pressed 3\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(3);  // Insert 3 into input
//         }
//         else if (row == 3) {    // 6
//             q.call(printf, "Pressed 6\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(6);  // Insert 6 into input
//         }
//         else if (row == 0) {    // 9 
//             q.call(printf, "Pressed 9\n"); // Printing to serial monitor
//             q.dispatch_once();
//             insertAlarmDist(9);  // Insert 9 into input
//         }
//         else {    // #
//             q.call(printf, "Pressed #\n"); // Printing to serial monitor
//             q.dispatch_once();
//         }
//     }
// }

/*
	void isr_col4
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called when the InterruptIn object associated with column 4 on the 
        matrix keypad recieves a rising edge. The action taken will depend on what row was 
        supplying power to the keypad when the rising edge occured. This handler will call 
        additional ISR handlers depending on whether A, B or D were pressed:
            A: isrA_StartTimer
            B: isrB_StopTimer
            D: isrD_SetTimer

    note: since row is iterated at the end of rowCycler, row will hold (row+1)%4 when the
          isr is called (eg If row 1 was supplying power, row will be 2)
*/
// inline void MatrixKeypad::isr_col4(void) {
//    // q.call(printf, "col4: A, B, C, D\n");
//     if (row == 2) {         // A
//         q.call(printf, "Pressed A\n"); // Printing to serial monitor
//         q.dispatch_once();
//         allowUserInput = true;
//     }
//     else if (row == 3) {    // B
//         q.call(printf, "Pressed B\n"); // Printing to serial monitor
//         q.dispatch_once();
//         allowUserInput = false;
//         calcDistMetric();
//         // q.call(isrB_StopTimer);     // Call ISR for stopping timer (B)
//         // q.dispatch_once();
//     }
//     else if (row == 0) {    // C
//         //q.call(printf, "Pressed C\n"); // Printing to serial monitor
//         //q.dispatch_once();
//     }
//     else {    // D
//         q.call(printf, "Pressed D\n"); // Printing to serial monitor
//         q.dispatch_once();
//         // q.call(isrD_SetTimer);      // Call ISR for setting timer (D)
//         // q.dispatch_once();
//     }
// }
