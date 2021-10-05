/*-------------------------------------------------------------------------------------
*
*   File Name:      CSE321_project1_naminor_corrected_code.cpp
*   Programmer:     Nick Minor
*   Date:           2021/10/04
*   Purpose:        This file makes use of the Thread, DigitalOut and InterruptIn APIs 
*                   from the mbed.h header file in order to have an LED blink with an
*                   on time of 2000ms and an off time of 500ms. When a button is pressed,
*                   the light will toggle between blinking and being completely off.
*
*   Course:         CSE 321 - Realtime and Embedded Systems
*   Assignment:     Project 1, Part 6
*   Functions:      ISR_Handler(), releaseButton(), pushButton()
*   Inputs:         BUTTON1 (PC13) - Integrated button on Nucleo
*   Outputs:        LED2 (PB7) - Integrated blue light on Nucleo
*
*   Constraints:    N/A
*   Sources:        p1_code_provided.cpp
*                   MbedOS API Documentation: 
*                       https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
*
*------------------------------------------------------------------------------------*/

#include "mbed.h"

// Create a thread to drive an LED to have an on time of 2000ms and off time of 500ms
Thread controller;		// Allows for scheduling and controlling of parallel tasks

void ISR_Handler();		// The Interrupt Service Routine
void releaseButton();	// Callback function for rising edge (Button is up)
void pushButton();		// Callback function for falling edge (Button is down)

DigitalOut light(LED2);			// establish blue led as an output
InterruptIn button(BUTTON1);	// Button on Nucleo is the input
int buttonState = 0;	// Button is down when 0, and up when 1
int lightStatus = 0;	// Light is on when 0, and off when 1


int main() {
	// start the allowed execution of the thread
	printf("----------------START----------------\n");
	printf("Starting state of thread: %d\n", controller.get_state());

	controller.start(ISR_Handler);	// Starts the thread "controller" executing ISR_Handler()
	printf("State of thread right after start: %d\n", controller.get_state());

	button.rise(releaseButton);	// Calls releaseButton() on a rising edge (default)
	button.fall(pushButton);	// Calls pushButton() on a falling edge (pressing the button)

	return 0;
}

/*
	void ISR_Handler()
	  Parameters: 	none
	  Return Value:	void

  	Description:
	  Executed by the thread "controller". When lightStatus is 0, this function causes the blue light 
	  to turn on for 2000ms and then turn off for 500ms. This continues while lightStatus is 0, 
	  resulting in the blinking effect to occur. If lightStatus is 1, nothing occurs, leaving the 
	  output "light" in the last state it was in, the low state (LED2 is off).
*/
// make the handler
void ISR_Handler() {
	while (true) {
		if (lightStatus == 0){
			light = !light; // Flip the state of light from low to high
			printf("Turned the light on, light = %d\n", light.read()); //you do need to update the print statement to be correct
			thread_sleep_for(2000); //Thread_sleep is a time delay function, causes a 2000 unit delay

			light = !light; // Flip the state of the light from high to low
			printf("Turned the light off, light = %d\n", light.read());
			thread_sleep_for(500); //Thread_sleep is a time delay function, causes a 500 unit delay
		}
  	}
}

/*
	void releaseButton()
	  Parameters:   none
	  Return Value:	void

	Description:
	  This function activates every time the button is released from a down state. It resets the
	  buttonState to 1 (up) so that when the button is pressed down again, the condition within 
	  pushButton() will be satisfied, which toggles the lightStatus.
*/
void releaseButton() {	// Toggle the state of the thread
	buttonState = 1;	// Button is released, now up
}

/*
	void pushButton()
  	  Parameters:	none
	  Return Value:	void

	Description:
	  After pressing the button once, this function causes lightStatus to become 1, which ends the 
	  condition in ISR_Handler() from executing during the while loop, turning the light off as 
	  a result. buttonState is set to 0 so this function does nothing until the thread is reactivated
	  using releaseButton(), which executes when the button is released.
	  Upon pressing the button again, this function resets lightStatus to 0, which satisfies the condition
	  in ISR_Handler(), causing the light to blink once more.
*/
void pushButton() {
	if (buttonState == 1){	// If the button is up
		lightStatus++;		// lightStatus increments from 0 to 1 or from 1 to 2
		lightStatus %= 2;	// If lightStatus is 1, it stays 1. If 0, it stays 0. If it is 2, it becomes 0.
		buttonState = 0;	// Button is pushed down
	}
}