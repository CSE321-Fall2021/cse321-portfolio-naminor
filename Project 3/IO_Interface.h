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
    int input[4];
    int mode;

public:
    int currDist;   // Holds the current distance from the object in cm
    int alarmDist;  // Holds the value that the user would like the buzzer to alarm at
    char lcd_output[6];    // Used to output distance to the LCD in cm
    bool timerCounting;
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
    }
    void changeMode(int newMode) {mode = newMode;}
    int calcDistMetric() {  // Calculates current distance from the input array
        alarmDist = 0;
        if (input[0] != -1)
            alarmDist += input[0] * 1000;
        if (input[1] != -1)
            alarmDist += input[1] * 100;
        if (input[2] != -1)
            alarmDist += input[2] * 10;
        if (input[3] != -1)
            alarmDist += input[3];
        return alarmDist;
    }
    void resetAlarmDist() {
        for (int i = 0; i < 4; i++) { 
            input[i] = -1;   // -1 means the user hasn't entered a number for that digit yet
            //output[i] = -1;
        }
    }
    void insertAlarmDist(int val) { // Inserts a user's input from the keypad 
        if (input[3] == -1) {
            input[3] = val;
            //output[3] = val;
        }
        // This conditional is new, check this if there are issues
        else if (input[2] == -1) {   // No digit has been inserted
            input[2] = input[3];
            input[2] = val;

            //output[2] = output[3];
            //output[3] = val;
        } else if (input[1] == -1) {
            input[1] = input[2];
            input[2] = val;

            //output[1] = output[2];
            //output[2] = val;
        } else if (input[0] == -1) {
            input[0] = input[1];
            input[1] = input[2];
            input[2] = val;

            /*output[0] = output[1];
            output[1] = output[2];
            output[2] = val;*/
        }
    }
    void getOutputChars() {
        for (int i = 0; i < 3; i++) {
            lcd_output[i] = 0;
        }
        int conversion = currDist;

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

        lcd_output[3] = ' '; lcd_output[4] = 'c'; lcd_output[5] = 'm';  // Append " cm"
        
        printf(" %c%c%c cm \n\r",lcd_output[0], lcd_output[1], lcd_output[2]);
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