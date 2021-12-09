#include "IO_Interface.h"
#include "1802.h"
#include <chrono>


// Ultrasonic 
// -> PA5 trigger output signal
// -> PA6 echo input signal

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);
DigitalOut trigger(PA_6);
EventQueue q(32 * EVENTS_EVENT_SIZE);
EventQueue q1(32 * EVENTS_EVENT_SIZE);
Thread t1;
Ticker tick;   // Sends a trigger every 500 ms

InterruptIn echo(PA_5, PullDown);
int dist = 0;
int correction = 0;
Timer sentinel;
IO_Interface io;

int startTime, endTime;


void isr_start(void);
void isr_stop(void);

int main() {
    RCC->AHB2ENR |= 0x7;            // Turn on clock for Ports A, B, C
    GPIOB->MODER |= 0x10;           // PB_2
    GPIOB->MODER &= ~(0x20);

    echo.fall(&isr_stop);



    t1.start(callback(&q1, &EventQueue::dispatch_forever));
    tick.attach(q1.event(isr_start), 500ms);

    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(2000); // Timeout after 2 seconds

    sentinel.reset();
    sentinel.start();
    while (echo == 2) {};
    sentinel.stop();
    correction = sentinel.elapsed_time().count();
    printf("Approximate software overhead timer delay is %d uS\n\r",correction);
    //Loop to read Sonar distance values, scale, and print
    LCD.begin();


    while(1) {
// trigger sonar to send a ping
        // trigger = 1;
        // sentinel.reset();
        // wait_us(10.0);
        // trigger = 0;

//wait for echo high
       // while (echo==0) {}
//echo high, so start timer
        //sentinel.start();
        
//wait for echo low
        //while (echo==1) {};
//stop timer and read value
        //sentinel.stop();
        
//subtract software overhead timer delay and scale to cm
     

    if (io.timerCounting == false) {
        dist = (sentinel.elapsed_time().count()-correction)/58.0;
        dist-=38;
        if (dist < 400) // 4 meters is reliable range
            io.currDist = dist;
        printf(" %d cm \n\r",dist);

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
        thread_sleep_for(200);
        io.timerCounting = true;
    }
//wait so that any echo(s) return before sending another ping
        //wait_us(500);
    }
}

void isr_start(void) {

    io.timerCounting = true;
    printf("Started counting\n");
    trigger = 1;
    sentinel.reset();
    trigger = 0;
    sentinel.start();
    startTime = sentinel.elapsed_time().count();
}

void isr_stop(void) {
    //printf("Stopped Counting\n");
    endTime = sentinel.elapsed_time().count();
    sentinel.stop();
    io.timerCounting = false;
    //dist = (endTime - startTime) / 58.0;
}






