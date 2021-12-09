/*-------------------------------------------------------------------------------------
*
*   File Name:      CSE321_project3_naminor_main.cpp
*   Programmer:     Nick Minor
*   Date:           2021 12 09
*   Purpose:        
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

#include "IO_Interface.h"
#include "1802.h"
#include <chrono>


// Ultrasonic 
// -> PA5 trigger output signal
// -> PA6 echo input signal

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);
DigitalOut trigger(PA_6);
EventQueue q1(32 * EVENTS_EVENT_SIZE);
EventQueue q(32 * EVENTS_EVENT_SIZE);
Thread t1;
Thread t2;
Ticker tick;   // Sends a trigger every 500 ms
Ticker KeypadCycler;    // Ticker that will cycle through rows of the matrix keypad

InterruptIn echo(PA_5, PullDown);
int dist = 0;
int correction = 0;
Timer sentinel;
IO_Interface io;
MatrixKeypad mk;

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
void A_ChangeDist(void);
void B_CompInput(void);

int main() {
    RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
    GPIOB->MODER |= 0x10;           // PB_2
    GPIOB->MODER &= ~(0x20);

    echo.fall(&isr_stop);

    t1.start(callback(&q1, &EventQueue::dispatch_forever));
    tick.attach(q1.event(isr_start), 500ms);
    t2.start(callback(&q, &EventQueue::dispatch_forever));
    

    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(2000); // Timeout after 2 seconds

    sentinel.reset();
    sentinel.start();
    while (echo == 2) {};
    sentinel.stop();
    correction = sentinel.elapsed_time().count();
    printf("Software delay to be corrected: %d us\n",correction);
    //Loop to read Sonar distance values, scale, and print
    LCD.begin();



    // KEYPAD
    //MatrixKeypad mk;
    // col1.rise(callback(&mk, &MatrixKeypad::isr_col1));   // Call isr_col1 when col1 has a rising edge (a key in column 1 was pressed)
    // col2.rise(callback(&mk, &MatrixKeypad::isr_col2));   // Call isr_col2 when col2 has a rising edge
    // col3.rise(callback(&mk, &MatrixKeypad::isr_col3));   // Call isr_col3 when col3 has a rising edge
    // col4.rise(callback(&mk, &MatrixKeypad::isr_col4));   // Call isr_col4 when col4 has a rising edge    
    col1.rise(&isr_col1);   // Call isr_col1 when col1 has a rising edge (a key in column 1 was pressed)
    col2.rise(&isr_col2);   // Call isr_col2 when col2 has a rising edge
    col3.rise(&isr_col3);   // Call isr_col3 when col3 has a rising edge
    col4.rise(&isr_col4);   // Call isr_col4 when col4 has a rising edge
    //KeypadCycler.attach(q.event(callback(&mk, &MatrixKeypad::rowCycler)), 80ms);    // Cycle power through rows every 80ms
    KeypadCycler.attach(&rowCycler, 80ms);
    t2.set_priority(osPriorityNormal3);
    while(1) {
        if (io.timerCounting == false) {
            dist = (sentinel.elapsed_time().count()-correction)/58.0;
            dist-=38;
            if (dist < 400) // 4 meters is reliable range
                io.currDist = dist;
            //printf(" %d cm \n\r",dist);

            io.getOutputChars();
            LCD.setCursor(0, 0);
            LCD.print("Distance");   // Prompt the user to enter a time
            LCD.setCursor(0, 1);
            LCD.print(io.lcd_output);
            if (dist < io.alarmDist) {
                //GPIOB->ODR &= ~(0x4);   // Turn on the buzzer
                io.ringBuzzer();
            }
            else {
                //GPIOB->ODR |= 0x4; 
                io.silenceBuzzer();
            }
            Watchdog::get_instance().kick();
            //thread_sleep_for(200);
            io.timerCounting = true;
        }
    }
}

void isr_start(void) {

    io.timerCounting = true;
    //printf("Started counting\n");
    trigger = 1;
    sentinel.reset();
    trigger = 0;
    sentinel.start();
}

void isr_stop(void) {
    //printf("Stopped Counting\n");
    sentinel.stop();
    io.timerCounting = false;
    //dist = (endTime - startTime) / 58.0;
}


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
        additional ISR handlers depending on whether A, B or D were pressed:
            A: isrA_StartTimer
            B: isrB_StopTimer
            D: isrD_SetTimer

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

void A_ChangeDist(void) {
    io.resetAlarmDist();
    io.allowUserInput = true;
}

void B_CompInput(void) {
    io.allowUserInput = false;
    io.calcDistMetric();
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