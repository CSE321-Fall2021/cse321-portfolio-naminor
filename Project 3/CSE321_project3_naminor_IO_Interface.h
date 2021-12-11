/*-------------------------------------------------------------------------------------
*
*   File Name:      IO_Interface.h
*   Programmer:     Nick Minor
*   Date:           2021 12 08
*   Purpose:        Contains IO_Interface and MatrixKeypad classes used in managing 
*                   the interaction between the input and output peripherals.
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 3
*   Classes         IO_Interface, MatrixKeypad
*   Inputs:         MatrixKeypad, Ultrasonic sensor
*   Outputs:        LCD screen, Buzzer
*
*------------------------------------------------------------------------------------*/

#include "mbed.h"
enum {INPUT_MODE = 1, WARNING_MODE, ALARM_MODE}; // Enums for input modes. (1, 2, 3)

/*
	Class:  IO_Interface
	Description:
        This class holds data used by peripherals as well as many functions that  
        facilitate interaction between the user, the inputs and the outputs.
    Variables:
        int input[]
            Integer array that holds the user's input for alarm range
        int mode
            Records the system's mode
        int currDist
            Holds the current distance from the object in cm
        int alarmDist
            Holds the value for the range that the user would like the buzzer to alarm at
        char lcd_output[]
            Holds the character array with the distance to the nearest object and the alarm
            range. This is printed to the LCD.
        bool timerCounting
            Records whether or not a timer is recording the time between a trigger and an echo
        bool allowUserInput
            Records if a user is allowed to specify a new distance using 0-9 keys
    Methods:
        IO_Interface
            Default constructor. Sets the current distance to 0, default alarm range to 100cm,
            the mode to 0, the input array indices to -1 to indicate no digits entered, 
            whether the timer is counting to false, and whether the user can use the 0-9
            keys to false.
        void changeMode
            Changes the current system mode to that specified
        int calcDistMetric
            Computes the user's input for the alarm range from the member int array input[]
            into an integer value and places it into the new alarm range alarmDist.
        void resetAlarmDist
            Resets the alarm array values to -1 to indicate a digit hasn't been entered yet
            for each index.
        void insertAlarmDist
            Parses the user's input from the matrix keypad into the member int array input[].
        void getOutputChars
            Converts the integer value for distance from the object, currDist, into a the member
            char array lcd_output[] for printing to the LCD screen.
        void ringBuzzer
            Does GPIO configurations to turn on the buzzer
        void silenceBuzzer
            Does GPIO configurations to turn off the buzzer
*/
class IO_Interface {
private:
    int input[3];   // Holds the user's specified input
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
        }
    }
    void insertAlarmDist(int val) { // Inserts a user's input from the keypad 
        if (input[2] == -1) {
            input[2] = val;
        }
        else if (input[1] == -1) {   // No digit has been inserted
            input[1] = input[2];
            input[2] = val;
        } else if (input[0] == -1) {
            input[0] = input[1];
            input[1] = input[2];
            input[2] = val;
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

        if (lcd_output[0] == 0 && lcd_output[1] == 0) { // 1-digit number
            lcd_output[0] = ' ';
            lcd_output[1] = ' ';
        } else if (lcd_output[0] == 0) {    // 2-digit number
            lcd_output[0] = ' ';
        }

        for (int i = 0; i < 3; i ++) {  // Convert integers to chars
            if (lcd_output[i] != ' ') {
                lcd_output[i] += '0';
            }
        }

        lcd_output[3] = ' '; lcd_output[4] = '/'; lcd_output[5] = ' ';
        if (input[0] == -1 && input[1] == -1 && input[2] == -1) {   // Default alarm range is 100
            lcd_output[6] = '1'; lcd_output[7] = '0'; lcd_output[8] = '0';
        }
        else {  // The alarm range is a user-specified value, not default
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
        
        //printf(" %c%c%c cm \n\r",lcd_output[0], lcd_output[1], lcd_output[2]);
    }
    void ringBuzzer() {
    // Set pin used with buzzer to high
        GPIOB->ODR &= ~(0x4);   // Turn on the buzzer

    }
    void silenceBuzzer() {
    // Set pin used with buzzer to low
        GPIOB->ODR |= 0x4; 
    }
};


/*
	Class:  MatrixKeypad
	Description:
        This class sets up the GPIO connections for the matrix keypad and records the
        current row receiving power.
    Variables:
        int row
            Tracks the current row that power is being given to.
    Methods:
        MatrixKeypad()
            Default constructor. Initializes GPIO ports and sets the current row to 1.
*/
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

};