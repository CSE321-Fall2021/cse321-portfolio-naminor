-------------------
About
-------------------
Project Description: 
Contribitor List:


--------------------
Features
--------------------

--------------------
Required Materials
--------------------
-Nucleo L4R5ZI

--------------------
Resources and References
--------------------
MbedOS API Documentation: 
	https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html


--------------------
Getting Started
--------------------
This section will describe the general setup

--------------------
CSE321_project1_naminor_corrected_code.cpp:
--------------------
Date Created: 2021/10/04
Language: C++
Description:
	This file makes use of the Thread, DigitalOut and InterruptIn APIs from the mbed.h
	header file in order to have an LED blink with an on time of 2000ms and an off time
	of 500ms when a button is pressed.


This file has lots of things. There is a os typle tool used to create periodic events with a periepherial. The name of the file and the contents in here will be entirely updated.
 
This is totally not bare metal since there are some cool tools used. Those tools instantiate with a finite reference but gots their unique object created. 


----------
Declarations
----------
Variables, functions, objects and APIs are used in this program.

Header Files:
	mbed.h
Data Types:
	Thread controller:

	DigitalOut light:

	InterruptIn button:

	int threadState:

	int alternate:

Functions:
	void ISR_Handler():

	void activateThread():

	void disableThread():


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
activateThread:
disableThread:


saveTheWorldssss:
	Does savomg ps somentjinmg. 
	Inputs:
		None
	Globally referenced things used:
	Unicorns and fire, pringtf isglobal too


