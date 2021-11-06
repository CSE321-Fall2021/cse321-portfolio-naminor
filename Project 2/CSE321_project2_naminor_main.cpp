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
*   Functions:      isr_col1, isr_col2, isr_col3, isr_col4,
*                   isrA_StartTimer, isrB_StopTimer, isrD_SetTimer,
*                   secondPassed, rowCycler
*                   class LCDClock methods:
*                       calcTime, insertValue, resetInput, getInput, getOutput,
*                       getOutputChars, getTime, setTime, TimesUp
*
*   Inputs:         Matrix keypad
*   Outputs:        LCD screen, LEDs
*
*   Constraints:    Time is represented as m:ss (valid times between 0:00 and 9:59),
*                   System runs forever
*   Sources:        RM0432 Manual
*                       https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
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

EventQueue q(32 * EVENTS_EVENT_SIZE);   // for printing messages to the serial monitor from within an ISR
CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);     // Object for interacting with LCD screen
Timeout countdown;  // Calls the TimesUp function after a specified amount of time has passed
Ticker tick;        // Calls secondPassed every second to allow printing to the LCD screen
void secondPassed(void); // Attached to Ticker tick to allow printing to the LCD after a second has passed
Ticker cycler;      // Ticker that will cycle through rows
void rowCycler(void);    // Cycles through giving rows power

class LCDClock {
private:
    int input[3];   // Holds the user's inputted numbers in array form m:ss
public:
    int currTime;   // Holds the user's input in seconds
    int output[3];  // Counts down the timer, used in outputting to LCD
    bool allowTimeInput;    // Allow the user to enter numbers using the keypad
    bool timerOn;           // The timer is currently running
    bool allowPrint;        // Allow the time remaining to be print to the LCD
    bool timerPaused;       // Tracks if the timer is currently paused
    LCDClock() {    // Constructor
        currTime = 0;
        allowTimeInput = false;
        allowPrint = false;
        timerOn = false;
        timerPaused = false;
        for (int i = 0; i < 3; i++) {
            input[i] = -1;  // -1 means the user hasn't entered a number for that digit yet
            output[i] = -1;
        }
    }
    int calcTime() {    // Converts user's input into seconds
        currTime = 0;
        // Minutes : Seconds Seconds
        if (input[0] != -1) // If there is a number entered for the digit
            currTime += input[0] * 60;  // Minutes
        if (input[1] != -1)
            currTime += input[1] * 10;  // Seconds
        if (input[2] != -1)
            currTime += input[2];       // Seconds
        return currTime;    // Return the converted time
    }
    void insertValue(int val) { // Inserts a user's input from the keypad 
        if (input[2] == -1) {   // No digit has been inserted
            input[2] = val;     // Assign value to the least significant digit

            output[2] = val;
        } else if (input[1] == -1) {    // 1 digit has been inserted
            input[1] = input[2];    // Shift number already entered up a digit
            input[2] = val;         // Assign value to the least significant digit

            output[1] = output[2];
            output[2] = val;
        } else if (input[0] == -1) {    // 2 digits have been inserted
            input[0] = input[1];    // Shift first number entered up to the most significant digit
            input[1] = input[2];    // Shift second number entered up a digit
            input[2] = val;         // Assign value to the least significant digit

            output[0] = output[1];
            output[1] = output[2];
            output[2] = val;
        }
    }
    void resetInput() {     // Resets the user's input so a new one can be specified
        for (int i = 0; i < 3; i++) { 
            input[i] = -1;  // -1 means the user hasn't entered a number for that digit yet
            output[i] = -1;
        }
    }
    int* getInput() {return input;}     // Returns input array (original value)
    int* getOutput() {return output;}   // Returns output array (original value - time passed)
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
        // Printing time remaining to serial monitor
        printf("Time: %d:%d%d   Seconds: %d\n", output[0], output[1], output[2], currTime); 

        printChars[0] = output[0] + '0';
        printChars[1] = ':';
        printChars[2] = output[1] + '0';
        printChars[3] = output[2] + '0';
        printChars[4] = '\0';
    }
    int getTime() {return currTime;}
    void setTime(int time) {currTime = time;}
    void TimesUp() {
        printf("Time's Up\n"); // Printing to serial monitor
        resetInput();
    }
} cl;


// Interrupt Service Routines:
void isr_col1(void);    // Handler for column 1 of the matrix keypad: 1, 4, 7, *
void isr_col2(void);    // Column 2:    2, 5, 8, 0
void isr_col3(void);    // Column 3:    3, 6, 9, #
void isr_col4(void);    // Column 4:    A, B, C, D
void isrA_StartTimer(void); // ISR for when A is pressed
void isrB_StopTimer(void);  // ISR for when B is pressed
void isrD_SetTimer(void);   // ISR for when D is pressed

int row = 1;    // Tracks the current row

// When the button is pressed, the switch closes and the circuit completes
InterruptIn col1(PC_11, PullDown);     // Column 1 of keypad is attached to PC11
InterruptIn col2(PC_10, PullDown);
InterruptIn col3(PC_9, PullDown);
InterruptIn col4(PC_8, PullDown);



int main() {
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
    // Set pins giving power to LEDs when timer completes to General Purpose Output mode (01)
    GPIOB->MODER |= 0x10;           // PB_2
    GPIOB->MODER &= ~(0x20);
    GPIOB->MODER |= 0x1000;         // PB_6
    GPIOB->MODER &= ~(0x2000);

    col1.rise(&isr_col1);   // Call isr_col1 when col1 has a rising edge (a key in column 1 was pressed)
    col2.rise(&isr_col2);   // Call isr_col2 when col2 has a rising edge
    col3.rise(&isr_col3);   // Call isr_col3 when col3 has a rising edge
    col4.rise(&isr_col4);   // Call isr_col4 when col4 has a rising edge

    printf("Starting...\n");
    q.call(printf, "Starting LCD\n"); // Printing to serial monitor
    q.dispatch_once();
    LCD.begin();

    cycler.attach(&rowCycler, 80ms);    // Cycle power through rows every 80ms
    tick.attach(&secondPassed, 1s);     // Allow printing to LCD every 1 second

    while (true) { 
        // If the timer is not yet running, but time input is allowed
        if (cl.timerOn == false && cl.allowTimeInput == true){
            LCD.setCursor(0, 0);
            LCD.clear();
            LCD.print("Input time:");   // Prompt the user to enter a time
        }
        // If the timer is on, has not yet reached 0, and printing is allowed (1s has passed since last print)
        if (cl.getTime() > 0 && cl.timerOn == true && cl.allowPrint == true) {
            GPIOB->ODR &= ~(0x4);   // Reset LEDs that light up when time's up, if they are on
            GPIOB->ODR &= ~(0x40);
            char getChars[5];
            cl.getOutputChars(getChars); // Get the current time in an array of chars
            LCD.setCursor(0, 0);
            LCD.print("Time Remaining:");
            LCD.setCursor(0, 1);
            LCD.print(getChars);    // Print the remaining time
            cl.allowPrint = false;  // Disallow printing until a second has passed
        }
        // If the timer is on and has reached 0
        if (cl.getTime() == 0 && cl.timerOn == true) {
            LCD.setCursor(0,0);
            LCD.clear();
            LCD.print("Time's Up"); // Tell user the specified time has passed
            GPIOB->ODR |= 0x4;      // Illuminate the LEDs
            GPIOB->ODR |= 0x40;
        }
    }
    return 0;
}

/*
	void secondPassed
  	  Parameters:	none
	  Return Value:	void

	Description:
        This function is called every second by the ticker tick. If the timer is on with a
        remaining time greater than 0, printing to the LCD will be allowed every 1 second.
*/
void secondPassed() {
    // If the remaining time is greater than 0 and the timer is on
    if ((countdown.remaining_time() >= (chrono::microseconds)0) && cl.timerOn == true){
        cl.allowPrint = true;   // Allow printing to the LCD
    }
}

/*
	void rowCycler
  	  Parameters:	none
	  Return Value:	void

	Description:
        This function is called by the Ticker tick to be executed every 80 ms. It cycles
        through giving power to each of the rows of the matrix keypad. With each iteration,
        power is given to a particular row and removed from the others and the variable row
        iterates. This in combination with the column InteruptIn objects allow us to know 
        exactly which key was pressed, as the InterruptIn gives the column and row gives
        the row.
*/
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
        GPIOB->ODR |= 0x20;     // Turn on  PB_5, row 4
        GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
        GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
    }
    row++;
    row %= 4;
}

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
void isr_col1(void) {
    //q.call(printf, "col1: 1, 4, 7, *\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 1
            q.call(printf, "Pressed 1\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(1);  // Insert 1 into input
        }
        else if (row == 3) {    // 4
            q.call(printf, "Pressed 4\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(4);  // Insert 4 into input
        }
        else if (row == 0) {    // 7
            q.call(printf, "Pressed 7\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(7);  // Insert 7 into input
        }
        else {    // *
            q.call(printf, "Pressed *\n"); // Printing to serial monitor
            q.dispatch_once();
        }
    }
}

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
void isr_col2(void) {
    //q.call(printf, "col2: 2, 5, 8, 0\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 2
            q.call(printf, "Pressed 2\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(2);  // Insert 2 into input
        }
        else if (row == 3) {    // 5
            q.call(printf, "Pressed 5\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(5);  // Insert 5 into input
        }
        else if (row == 0) {    // 8
            q.call(printf, "Pressed 8\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(8);  // Insert 8 into input
        }
        else {    // 0
            q.call(printf, "Pressed 0\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(0);  // Insert 0 into input
        }
    }
}

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
void isr_col3(void) {
    //q.call(printf, "col3: 3, 6, 9, #\n");
    if (cl.allowTimeInput == true) {
        if (row == 2) {         // 3
            q.call(printf, "Pressed 3\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(3);  // Insert 3 into input
        }
        else if (row == 3) {    // 6
            q.call(printf, "Pressed 6\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(6);  // Insert 6 into input
        }
        else if (row == 0) {    // 9 
            q.call(printf, "Pressed 9\n"); // Printing to serial monitor
            q.dispatch_once();
            cl.insertValue(9);  // Insert 9 into input
        }
        else {    // #
            q.call(printf, "Pressed #\n"); // Printing to serial monitor
            q.dispatch_once();
        }
    }
}

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
void isr_col4(void) {
   // q.call(printf, "col4: A, B, C, D\n");
    if (row == 2) {         // A
        q.call(printf, "Pressed A\n"); // Printing to serial monitor
        q.dispatch_once();
        q.call(isrA_StartTimer);    // Call ISR for starting timer (A)
        q.dispatch_once();
    }
    else if (row == 3) {    // B
        q.call(printf, "Pressed B\n"); // Printing to serial monitor
        q.dispatch_once();
        q.call(isrB_StopTimer);     // Call ISR for stopping timer (B)
        q.dispatch_once();
    }
    else if (row == 0) {    // C
        q.call(printf, "Pressed C\n"); // Printing to serial monitor
        q.dispatch_once();
    }
    else {    // D
        q.call(printf, "Pressed D\n"); // Printing to serial monitor
        q.dispatch_once();
        q.call(isrD_SetTimer);      // Call ISR for setting timer (D)
        q.dispatch_once();
    }
}

/*
	void isrA_StartTimer
  	  Parameters:	none
	  Return Value:	void

	Description:
        This handler starts the timer by setting the timerOn variable to true, which allows the timer to
        run in the while loop inside main. If the timer was paused it will resume from the time prior to the
        pause, otherwise it will start using the value specified by the user. The Timeout countdown is set up
        to call TimesUp after the corresponding time has passed.

*/
void isrA_StartTimer(void) {
    cl.allowTimeInput = false;      // User can't enter input while timer is running
    cl.timerOn = true;              // Timer is running
    if (cl.timerPaused == false) {  // If the timer isn't currently paused, a new value must have been entered
        cl.setTime(cl.calcTime());  // Set the clock's time to this new value
    }
    /*  
    The time initially displayed to the user will be one less what they input, since a very
    small amount of time passes between when they insert the value and when it is displayed 
    to the LCD. To circumvent this, the time input by the user needs to be incremented by 1. 
    */  
    // output is [m][s][s], from [0][0][0] to [9][5][9]
    if (cl.output[2] < 9) { // If least significant digit can be incremented without rolling over
        cl.output[2]++;     // Increment it
        cl.currTime++;
    } else if (cl.output[1] < 5) {  // If the next digit can be incremented w/out rolling over
        cl.output[2] = 0;   // Roll over the least significant
        cl.output[1]++;     // Increment the middle digit
        cl.currTime++;
    } else if (cl.output[0] < 9) {  // // If the most significant digit can be incremented w/out rolling over
        cl.output[2] = 0;   // Roll over the least significant and middle digits
        cl.output[1] = 0;
        cl.output[0]++;     // Increment the most significant digit
        cl.currTime++;
    }
    
    // Execute TimesUp after the specified time has passed
    countdown.attach(callback(&cl, &LCDClock::TimesUp), (chrono::seconds)cl.currTime);
    cl.timerPaused = false;     // Resume/Start timer
}

/*
	void isrB_StopTimer
  	  Parameters:	none
	  Return Value:	void

	Description:
        This handler stops the timer by detaching TimesUp after the Timeout has finished and setting the
        boolean variables timerOn and timerPaused such that the timer doesn't continue to count down within
        the while loop inside main.
*/
void isrB_StopTimer(void) {
    countdown.detach();         // Timeout no longer has a function to execute once it finishes counting
    cl.timerOn = false;         // Turn the timer countdown off
    cl.timerPaused = true;      // Pause the timer
}

/*
	void isrD_SetTimer
  	  Parameters:	none
	  Return Value:	void

	Description:
        This handler allows the user to specify a new input time to count down from. When allowTimeInput
        is set to be true, isr_col1, isr_col2 and isr_col3 will be able to insert values 0-9 into the 
        input array when their rising edges occur.

*/
void isrD_SetTimer(void) {
    cl.resetInput();            // Reset the current input values
    cl.allowTimeInput = true;   // Allow user to enter new input values
    cl.timerPaused = false;     // Unpause the timer
    q.call(printf, "Setting timer...\n");   // Printing to serial monitor
    q.dispatch_once();
}



/*  WIRING

    Connect Power -> Breadboard -> Matrix Keypad rows ()
        PB4 row1, PA4 row2, PB3 row3, PB5 row4

    Matrix Keypad columns -> Breadboard -> InterruptIn (PC11 through PC8)
                                        |
                                        -> LED (long) -> resistor -> ground

    LCD ->  SDA (PB_9)
        |
        ->  SCL (PB_8)
        |
        ->  3v3 power
        |
        ->  ground

    LEDs:
        PB2 -> LED (long) -> resistor -> ground
        PB6 -> LED (long) -> resistor -> ground
*/