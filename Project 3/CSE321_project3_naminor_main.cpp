/*-------------------------------------------------------------------------------------
*
*   File Name:      CSE321_project3_naminor_main.cpp
*   Programmer:     Nick Minor
*   Date:           2021 12 09
*   Purpose:        
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 3
*   Functions:      isr_col1, isr_col2, isr_col3, isr_col4,
*                   isrA_StartTimer, isrB_StopTimer, isrD_SetTimer,
*                   A_ChangeDist, B_CompInput, rowCycler
*                   isr_start, isr_stop
*   Inputs:         Matrix keypad, HC-SR04 (Ultrasonic sensor)
*   Outputs:        LCD screen, Buzzer
*
*   Constraints:    Distance to object is shown in cm and can be up to 400cm away or as
*                   close as 2cm. The HC-SR04 is most reliable detection objects within
*                   a 30 degree angle. As distance increases, detection of smaller and 
*                   non-flat objects becomes less reliable.
*                   System must run forever with inclusion of a watchdog, a 
*                   synchronization technique, a bitwise driver control, critical section
*                   protection, as well as the use of threads and interrupts.
*   Sources:        RM0432 Manual
*                       https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
*                   UM2179 Manual
*                       https://www.st.com/resource/en/user_manual/dm00368330-stm32-nucleo144-boards-mb1312-stmicroelectronics.pdf
*                   JDH_1804_Datasheet
*                       Under "Learn and Documents section" https://www.seeedstudio.com/Grove-16x2-LCD-White-on-Blue.html
*                   HC-SR04 Datasheets
*                       https://www.electroschematics.com/wp-content/uploads/2013/07/HC-SR04-datasheet-version-2.pdf
*                       https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
*                   Buzzer Datasheet
*                       http://tinkbox.ph/sites/tinkbox.ph/files/downloads/5V_BUZZER_MODULE.pdf 
*                   CSE321 LCD Library Files
*                   MbedOS API Documentation: 
*	                   https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
*
*------------------------------------------------------------------------------------*/

#include "CSE321_project3_naminor_IO_Interface.h"
#include "1802.h"
#include <chrono>


// Ultrasonic 
// -> PA5 trigger output signal
// -> PA6 echo input signal

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8); // LCD
DigitalOut trigger(PA_6);               // For sending a pulse to the ultrasonic sensor's trigger
EventQueue q1(32 * EVENTS_EVENT_SIZE);  // EventQueue for the ultrasonic sensor
EventQueue q(32 * EVENTS_EVENT_SIZE);   // EventQueue for the matrix keypad
Thread t1;              // Thread for the ultrasonic sensor
Thread t2;              // Thread for the matrix keypad
Ticker tick;            // Sends a trigger every 500 ms
Ticker KeypadCycler;    // Ticker that will cycle through rows of the matrix keypad

InterruptIn echo(PA_5, PullDown);
int dist = 0;       // Tracks the distance to the closest object
int correction = 0; // Amount of time it takes to send the pulse
Timer sentinel;     // Records the time it takes between sending a trigger and receiving an echo
IO_Interface io;    // Interface object that will handle interaction between user's input with the buzzer and LCD
MatrixKeypad mk;    // Matrix keypad object that initializes the keypad's GPIO ports and records the current row

void rowCycler();
void isr_start(void);
void isr_stop(void);

// KEYPAD
InterruptIn col1(PC_11, PullDown);     // Column 1 of keypad is attached to PC11
InterruptIn col2(PC_10, PullDown);
InterruptIn col3(PC_9, PullDown);
InterruptIn col4(PC_8, PullDown);
void isr_col1(void);    // Handler for column 1 of the matrix keypad: 1, 4, 7, *
void isr_col2(void);    // Column 2:    2, 5, 8, 0
void isr_col3(void);    // Column 3:    3, 6, 9, #
void isr_col4(void);    // Column 4:    A, B, C, D
void A_ChangeDist(void);    // Change alarm distance when A is pressed
void B_CompInput(void);     // Confirm and apply the new alarm distance

int main() {
    RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
    GPIOB->MODER |= 0x10;           // Set PB_2 (Buzzer) to general purpose output (01)
    GPIOB->MODER &= ~(0x20);

    echo.fall(&isr_stop); // Stop the timer sentinel on the HC-SR04's echo's falling edges
    t1.start(callback(&q1, &EventQueue::dispatch_forever)); // Thread and eventqueue for the ultrasonic sensor
    tick.attach(q1.event(isr_start), 500ms);    // Send trigger to HC-SR04 and start timer every 500ms
    t2.start(callback(&q, &EventQueue::dispatch_forever));  // Thread and eventqueue for the matrix keypad
    
    // Start up the watchdog
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(2000); // Timeout after 2 seconds

    // Get software delay of sending the trigger
    sentinel.reset();
    sentinel.start();
    while (echo == 2) {};
    sentinel.stop();
    correction = sentinel.elapsed_time().count();
    printf("Software delay to be corrected: %d us\n",correction);

    // KEYPAD INTERRUPTS
    col1.rise(&isr_col1);   // Call isr_col1 when col1 has a rising edge (a key in column 1 was pressed)
    col2.rise(&isr_col2);   // Call isr_col2 when col2 has a rising edge
    col3.rise(&isr_col3);   // Call isr_col3 when col3 has a rising edge
    col4.rise(&isr_col4);   // Call isr_col4 when col4 has a rising edge
    KeypadCycler.attach(&rowCycler, 80ms);  // Cycle through rows avery 80ms
    t2.set_priority(osPriorityNormal3); // Keypad input is more important than ultrasonic echo

    LCD.begin();    // Start up the LCD

    while(1) {
        if (io.timerCounting == false) {    // An echo was received, its time to print it
            dist = (sentinel.elapsed_time().count()-correction)/58.0;   // convert elapsed time to distance
            dist-=38;
            if (dist < 400) // 4 meters is reliable range, any greater value most likely means echo was not received
                io.currDist = dist;
            //printf(" %d cm \n\r",dist);

            // Print the current distance to the LCD
            io.getOutputChars();    // Contains the distance to the object and the alarm range
            LCD.setCursor(0, 0);
            LCD.print("Distance");  // Prompt the user to enter a time
            LCD.setCursor(0, 1);
            LCD.print(io.lcd_output);
            if (dist < io.alarmDist) {  // If object is within alarm range, ring the buzzer
                io.ringBuzzer();    // GPIOB->ODR &= ~(0x4); 
            }
            else {      // If object is not in alarm range, silence the buzzer       
                io.silenceBuzzer(); // GPIOB->ODR |= 0x4; 
            }
            Watchdog::get_instance().kick();    // We have succesfully sent a trigger, received and echo and printed to the screen. Nothing went wrong.
            //thread_sleep_for(200);
            io.timerCounting = true;    // Disallow printing until the next echo is received
        }
    }
}

/*
	void isr_start
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called every 500ms by the ticker tick to send a pulse to the trigger of the 
        ultrasonic sensor. It resets and starts the timer.
*/
void isr_start(void) {
    io.timerCounting = true;    // Disallow printing within the main loop
    //printf("Started counting\n");
    trigger = 1;
    sentinel.reset();   // Reset the timer
    trigger = 0;        // Send the trigger
    sentinel.start();   // Start the timer
}

/*
	void isr_stop
  	  Parameters:	none
	  Return Value:	void

	Description:
        ISR handler called when the InterruptIn object associated with echo of the ultrasonic sensor
        recieves a falling edge. It stops the timer sentinel and allows printing within the main loop.
*/

void isr_stop(void) {
    //printf("Stopped Counting\n");
    sentinel.stop();    // Stop the timer
    io.timerCounting = false;   // Allow printing to LCD within the main loop
}

/*
	void rowCycler
  	  Parameters:	none
	  Return Value:	void

	Description:
        This function is called by the Ticker KeypadCycler to be executed every 80 ms. It cycles
        through giving power to each of the rows of the matrix keypad. With each iteration,
        power is given to a particular row and removed from the others and the variable row
        inside the MatrixKeypad object iterates to the next row. This in combination with the 
        column InteruptIn objects allow us to know exactly which key was pressed, as the 
        InterruptIn gives the column and row gives the row.
*/

void rowCycler() {
    if (mk.row == 1) {
        //printf("row1\n");
        GPIOB->ODR |= 0x10;     // Turn on  PB_4, row 1
        GPIOA->ODR &= ~(0x10);  // Turn off PA_4, row 2
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
        GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    }
    else if (mk.row == 2) {
        //printf("row2\n");
        GPIOA->ODR |= 0x10;     // Turn on  PA_4, row 2
        GPIOB->ODR &= ~(0x10);  // Turn off PB_4, row 1
        GPIOB->ODR &= ~(0x8);   // Turn off PB_3, row 3
        GPIOB->ODR &= ~(0x20);  // Turn off PB_5, row 4
    }
    else if (mk.row == 3) {
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
    mk.row++;
    mk.row %= 4;
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
    if (io.allowUserInput == true) {
        if (mk.row == 2) {         // 1
            q.call(printf, "Pressed 1\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 1); // Insert 1 into input
        }
        else if (mk.row == 3) {    // 4
            q.call(printf, "Pressed 4\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 4);  // Insert 4 into input
        }
        else if (mk.row == 0) {    // 7
            q.call(printf, "Pressed 7\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 7);  // Insert 7 into input
        }
        else {    // *
            q.call(printf, "Pressed *\n"); // Printing to serial monitor
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
    if (io.allowUserInput == true) {
        if (mk.row == 2) {         // 2
            q.call(printf, "Pressed 2\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 2);  // Insert 2 into input
        }
        else if (mk.row == 3) {    // 5
            q.call(printf, "Pressed 5\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 5);  // Insert 5 into input
        }
        else if (mk.row == 0) {    // 8
            q.call(printf, "Pressed 8\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 8);  // Insert 8 into input
        }
        else {    // 0
            q.call(printf, "Pressed 0\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 0);  // Insert 0 into input
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
    if (io.allowUserInput == true) {
        if (mk.row == 2) {         // 3
            q.call(printf, "Pressed 3\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 3);  // Insert 3 into input
        }
        else if (mk.row == 3) {    // 6
            q.call(printf, "Pressed 6\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 6); // Insert 6 into input
        }
        else if (mk.row == 0) {    // 9 
            q.call(printf, "Pressed 9\n"); // Printing to serial monitor
            q.call(callback(&io, &IO_Interface::insertAlarmDist), 9);  // Insert 9 into input
        }
        else {    // #
            q.call(printf, "Pressed #\n"); // Printing to serial monitor
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
        additional handlers depending on whether A, B or D were pressed:
            A: A_ChangeDist
            B: B_CompInput

    note: since row is iterated at the end of rowCycler, row will hold (row+1)%4 when the
          isr is called (eg If row 1 was supplying power, row will be 2)
*/
void isr_col4(void) {
   // q.call(printf, "col4: A, B, C, D\n");
    if (mk.row == 2) {         // A
        q.call(printf, "Pressed A\n"); // Printing to serial monitor
        q.call(A_ChangeDist);
    }
    else if (mk.row == 3) {    // B
        q.call(printf, "Pressed B\n"); // Printing to serial monitor
        q.call(B_CompInput);
    }
    else if (mk.row == 0) {    // C
        q.call(printf, "Pressed C\n"); // Printing to serial monitor
    }
    else {    // D
        q.call(printf, "Pressed D\n"); // Printing to serial monitor
    }
}

/*
	void A_ChangeDist
  	  Parameters:	none
	  Return Value:	void

	Description:
        This function allows the user to specify a new range for the alarm to ring at after
        pressing A on the matrix keypad. It resets the current input and activates the 0-9
        keys on the matrix keypad.
*/
void A_ChangeDist(void) {
    io.resetAlarmDist();        // Reset the the current alarm range
    io.allowUserInput = true;   // Allow user to use the 0-9 keys to specify a new alarm range
}

/*
	void B_CompInput
  	  Parameters:	none
	  Return Value:	void

	Description:
        This function computes the user's input from a character array to an int using the calcDistMetric
        method from the IO_Interface class. The new alarm range is set to the integer value.
*/
void B_CompInput(void) {
    io.allowUserInput = false;  // Stop user 0-9 input until they press A again
    io.calcDistMetric();    // Compute and set the user's input for the alarm range within the interface object
    printf("New alarm dist: %d\n", io.alarmDist);
}


/* WIRING

    Pins -> Matrix Keypad Rows:
        PB4 row1, PA4 row2, PB3 row3, PB5 row4

    Matrix Keypad Columns -> InterruptIn
        PC11 through PC8

        LCD ->  SDA (PB_9)
        |
        ->  SCL (PB_8)
        |
        ->  3v3 power
        |
        ->  ground

    Buzzer
        PB2 ->          |
                            -> Buzzer   -> Ground
        3v3 Power ->    |

    Ultrasonic Transducer:
        PA6 (trigger)   ->  |                       ->  PA5 (Echo)
                                ->    Ultrasonic    |
        5v Power        ->  |                       ->  Ground

*/