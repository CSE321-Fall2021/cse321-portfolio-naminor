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

enum (INPUT_MODE = 1, WARNING_MODE, ALARM_MODE); // Enums for input modes. (1, 2, 3)
class IO_Interface {
private:
    int input[4];
    int mode;

public:
    int currDist;   // Holds the current distance from the object in cm
    int alarmDist;  // Holds the value that the user would like the buzzer to alarm at
    int output[4];    // Used to output distance to the LCD in cm
    enum (INPUT_MODE = 1, WARNING_MODE, ALARM_MODE); // Enums for input modes. (1, 2, 3)

    IO_Interface() {
        currDist = 0;
        alarmDist = -1; // -1 means alarm is inactive
        mode = 0;
        for (int i = 0; i < 3; i++) {
            input[i] = -1;  // -1 means the user hasn't entered a number for that digit yet
            output[i] = -1;
        }
    }
    void changeMode(int newMode) {mode = newMode;}
    int calcDistMetric() {  // Calculates current distance from the input array
        currDist = 0;
        if (input[0] != -1)
            currDist += input[0] * 1000;
        if (input[1] != -1)
            currDist += input[1] * 100;
        if (input[2] != -1)
            currDist += input[2] * 10;
        if (input[3] != -1)
            currDist += input[3];
        return currDist;
    }
    void resetInput() {
        for (int i = 0; i < 4; i++) { 
            input[i] = -1;   // -1 means the user hasn't entered a number for that digit yet
            output[i] = -1;
        }
    }
    void insertValue(int val) { // Inserts a user's input from the keypad 
        if (input[3] == -1) {
            input[3] = val;
            output[3] = val;
        }
        // This conditional is new, check this if there are issues
        else if (input[2] == -1) {   // No digit has been inserted
            input[2] = input[3];
            input[2] = val;

            output[2] = output[3];
            output[3] = val;
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
    void ringBuzzer(/*int pin*/) { // Can use pwm
    // try frequency between 2k and 5k?
    // Set pin used with buzzer to high

    }
    void silenceBuzzer(/*int pin*/) {
    // Set pin used with buzzer to low

    }
} io;