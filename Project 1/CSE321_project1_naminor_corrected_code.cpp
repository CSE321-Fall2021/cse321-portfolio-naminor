/*-------------------------------------------------------------------------------------
*
*   File Name:      CSE321_project1_naminor_corrected_code.cpp
*   Programmer:     Nick Minor
*   Date:           2021/10/04
*   Purpose:        This file makes use of the Thread, DigitalOut and InterruptIn APIs 
*                   from the mbed.h header file in order to have an LED blink with an
*                   on time of 2000ms and an off time of 500ms when a button is pressed.
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 1, Part 6
*   Functions:      ISR_Handler(), activateThread(), disableThread()
*   Inputs:         BUTTON1 (PC13) - Integrated button on Nucleo
*   Outputs:        LED2 (PB7) - Integrated blue light on Nucleo
*
*   Constraints:    
*   Sources:        p1_code_provided.cpp
*                   MbedOS API Documentation: 
*                       https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
*
*------------------------------------------------------------------------------------*/

#include "mbed.h"

// Create a thread to drive an LED to have an on time of 2000ms and off time of 500ms
Thread controller;    // Allows for scheduling and controlling of parallel tasks

void ISR_Handler(); // The Interrupt Service Routine
void activateThread(); 
void disableThread();

DigitalOut light(LED2);     // establish blue led as an output
InterruptIn button(BUTTON1); // Button on Nucleo is the input
int threadState = 0;
int alternate = 0;  /* Look into eliminating this */


int main() {
  // start the allowed execution of the thread
  printf("----------------START----------------\n");
	printf("Starting state of thread: %d\n", controller.get_state());

  controller.start(ISR_Handler);  // Starts the thread "controller" executing ISR_Handler()
	printf("State of thread right after start: %d\n", controller.get_state());

  button.rise(activateThread);  // Calls activateThread() on a rising edge (pressing button)
	button.fall(disableThread);   // Calls disableThread() on a falling edge (letting go of button)
	//https://youtu.be/XN2FrUUq-zI 

  return 0;
}

// make the handler
void ISR_Handler() {
  while (true) {
    if (alternate == 0){
      light = !light; // Flip the state of light to high from low
      printf("Turned the light on, light = %d\n", light.read()); //you do need to update the print statement to be correct
      thread_sleep_for(2000); //Thread_sleep is a time delay function, causes a 2000 unit delay

      light = !light; // Flip the state of the light to low from high
      printf("Turned the light off, light = %d\n", light.read());
      thread_sleep_for(500); //Thread_sleep is a time delay function, causes a 500 unit delay
    }
  }
}

void activateThread() { // Toggle the state of the thread
	threadState = 1;    	/* Set Flag to on */
}

void disableThread() {
  if (threadState == 1){
    alternate++;      // alternate increments from 0 to 1 or from 1 to 2
    alternate %= 2;   // If alternate is 1, it stays 1. If it is 2, it becomes 0.
    threadState = 0;  /* Reset flag to off */
  }
}