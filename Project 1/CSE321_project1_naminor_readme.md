-------------------
About
-------------------
Project Description:   CSE 321 Project 1 Part 6
                       Fixing p1_code_provided.cpp
Contribitor List:      Nick Minor


--------------------
Features
--------------------
This program causes the integrated blue led (LED2) on the nucleo to blink with an on time
of 2000ms and an off time of 500ms. When pressing the blue button on the nucleo, the light
can toggle between this blinking state and on where the blue light remains completely off.

--------------------
Required Materials
--------------------
-Nucleo L4R5ZI
-USB-A to micro USB-B cable
-Computer to run Mbed Studio (Windows, Mac or Linux versions available at https://os.mbed.com/studio/)

--------------------
Resources and References
--------------------
p1_code_provided.cpp
MbedOS API Documentation: 
	https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html


--------------------
Getting Started
--------------------
Set up an empty project in Mbed Studio with MbedOS inside the project folder. Place the file "CSE321_project1_naminor_corrected_code.cpp" into the project as the only file with a main function. Select "NUCLEO-L4R5ZI" as the Target and connect the Nucleo to the computer using the micro USB cable. Build the project and run to start the program. The blue light will begin to blink and the blue button on the Nucleo can be clicked to toggle the light between blinking and off.

--------------------
CSE321_project1_naminor_corrected_code.cpp:
--------------------
Date Created: 2021/10/04
Language: C++
Description:
	This file makes use of the Thread, DigitalOut and InterruptIn APIs from the mbed.h
	header file in order to have an LED blink with an on time of 2000ms and an off time
	of 500ms. When a button is pressed, the light will toggle between blinking and being 
	completely off.

----------
Declarations
----------
Variables, functions, objects and APIs are used in this program.

Header Files:
	mbed.h
		Contains the APIs used to interact with the Nucleo
Variables and API objects:
	Thread controller:
		Allows for scheduling and controlling of parallel tasks. Drives the LED blinking.
	DigitalOut light(LED2):
		An output named "light" is created corresponding to LED2 (blue light) on the Nucleo
	InterruptIn button(Button1):
		An input named "button" is instantiated which corresponds to the blue button on the Nucleo
	int buttonState:
		Tracks the current state of the button. 0 for down (pushed) and 1 for up (released).
	int alightStatus:
		Tracks the current status of the button. 0 for on and 1 for off.
Functions:
	void ISR_Handler():
		The intterrupt service routine
	void releaseButton():
		Callback function for rising edge (Button is up)
	void pushButton():
		Callback function for falling edge (Button is down)

----------
API and Built In Elements Used
----------
Header File: mbed.h
	Thread
		Allows for scheduling and controlling of parallel tasks. A block of code is executed upon the occurence of a specified event.
	DigitalOut
		Allows for usage of a GPIO pin as an output, doing all necessary initialization.
	InterruptIn
		Creates an interrupt triggered by an outside source (e.g. the button)
API List: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

----------
Custom Functions
----------

ISR_Handler:
	Executed by the thread "controller". When lightStatus is 0, this function causes the blue light to turn on for 2000ms and then turn off for 500ms. This continues while lightStatus is 0, resulting in the blinking effect to occur. If lightStatus is 1, nothing occurs, leaving the output "light" in the last state it was in, the low state (LED2 is off).
		Parameters: 	none
		Return Value:	void
releaseButton:
	This function activates every time the button is released from a down state. It resets the buttonState to 1 (up) so that when the button is pressed down again, the condition within pushButton() will be satisfied, which toggles the lightStatus.
		Parameters: 	none
		Return Value:	void
pushButton:
	After pressing the button once, this function causes lightStatus to become 1, which ends the condition in ISR_Handler() from executing during the while loop, turning the light off as a result. buttonState is set to 0 so this function does nothing until the thread is reactivated using releaseButton(), which executes when the button is released. 
	Upon pressing the button again, this function resets lightStatus to 0, which satisfies the condition in ISR_Handler(), causing the light to blink once more.
		Parameters: 	none
		Return Value:	void