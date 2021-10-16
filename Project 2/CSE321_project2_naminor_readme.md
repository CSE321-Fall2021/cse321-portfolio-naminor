-------------------
About
-------------------
Project Description:   CSE 321 Project 2
                       
Contribitor List:      Nick Minor


--------------------
Features
--------------------
There is functionality for the user to input the time duration of the timer as well as starting and stopping the timer. The time remaining on the timer and the completion of the timer is communicated to the user via an LCD screen and LEDs.

--------------------
Required Materials
--------------------
-Nucleo L4R5ZI
-LCD
-LEDs
-Matrix Keypad
-Solderless Breadboard
-Jumper wires
-USB-A to micro USB-B cable
-Computer to run Mbed Studio (Windows, Mac or Linux versions available at https://os.mbed.com/studio/)

--------------------
Resources and References
--------------------
MbedOS API Documentation: 
	https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
RM0432 Manual
JDH_1804_Datasheet
CSE321 LCD Library Files


--------------------
Getting Started
--------------------
Set up an empty project in Mbed Studio with MbedOS inside the project folder. Place the file "CSE321_project2_naminor_main.cpp" into the project as the only file with a main function. Select "NUCLEO-L4R5ZI" as the Target and connect the Nucleo to the computer using the micro USB cable. Connect the matrix keypad, LCD, LEDs to the Nucleo and breadboard. Build the project and run to start the program.

--------------------
CSE321_project2_naminor_main.cpp:
--------------------
Date Created: 2021/10/14
Language: C++
Description:

----------
Declarations
----------
Variables, functions, objects and APIs are used in this program.

Header Files:
	mbed.h
		Contains the APIs used to interact with the Nucleo
	1802.h
		API used to interact with the LCD
Variables and API objects:

Functions:


----------
API and Built In Elements Used
----------
Header File: mbed.h
	InterruptIn
		Creates an interrupt triggered by an outside source (e.g. the matrix keypad)	
	API List: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

Header File: 1802.h		(CSE321 LCD API)


----------
Custom Functions
----------
