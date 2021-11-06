-------------------
About
-------------------
Project Description:   CSE 321 Project 2
Contribitor List:      Nick Minor


--------------------
Features
--------------------
A user can interact with the system by:
-Pressing A
	Start or resume the timer when an input time is specified
-Pressing B
	Pause the timer
-Pressing D
	Input a time
-Pressing 0-9
	Enter the desired input time

The LCD screen will display the time remaining on the timer.
When the time has elapsed, the LCD screen will display "Time's Up" and two LEDs will light up.

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
	https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
JDH_1804_Datasheet
CSE321 LCD Library Files


--------------------
Getting Started
--------------------
Set up an empty project in Mbed Studio with MbedOS inside the 
project folder. Place the file "CSE321_project2_naminor_main.cpp" 
into the project as the only file with a main function. Select 
"NUCLEO-L4R5ZI" as the Target and connect the Nucleo to the 
computer using the micro USB cable. Connect the matrix keypad, 
LCD, LEDs to the Nucleo and breadboard. Build the project and run 
to start the program.

--------------------
CSE321_project2_naminor_main.cpp:
--------------------
Date Created: 2021/10/14
Language: C++
Description: The program is for a bare-metal and user-programmable
			 count-down alarm system.

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
	EventQueue q:
		EventQueue for printing messages to the serial monitor from within an ISR
	CSE321_LCD LCD:
		Object for interacting with LCD screen
	int row:
		Tracks the current row
	Ticker cycler:
		Ticker that will cycle through giving power to each row
	Timeout countdown:
		Calls the TimesUp function after a specified amount of time has passed
	Ticker tick:
		Calls secondPassed every second to allow printing to the LCD screen
	Ticker cylcer:
		Calls rowCycler every 80ms to cycle power through the rows of the matrix keypad

	InterruptIn col1(PC_11):
		Column 1 of keypad is attached to PC11
	InterruptIn col2(PC_10):
		Column 2 of keypad is attached to PC10
	InterruptIn col3(PC_8):
		Column 3 of keypad is attached to PC9
	InterruptIn col4(PC_8):
		Column 4 of keypad is attached to PC8
Functions:
	void secondPassed(void):
		Attached to Ticker tick to allow printing to the LCD after a second has passed
	void rowCycler(void):
		Cycles trough giving rows power

	Interrupt Service Routines:
		void isr_col1(void):
			Handler for column 1 of the matrix keypad: 1, 4, 7, *
		void isr_col2(void):
			Column 2:    2, 5, 8, 0
		void isr_col3(void):
			Column 3:    3, 6, 9, #
		void isr_col4(void):
			Column 4:    A, B, C, D
		void isrA_StartTimer(void):
			ISR for when A is pressed
		void isrB_StopTimer(void):
			ISR for when B is pressed
		void isrD_SetTimer(void):
			ISR for when D is pressed


----------
API and Built In Elements Used
----------
Header File: mbed.h
	InterruptIn
		Creates an interrupt triggered by an outside source (e.g. the matrix keypad)
	Timeout
		Sets up an interupt to be executed after a certain amount of time.
	Ticker	
		Sets up an interrupt to be continuously repeated at a specified time interval.
	EventQueue
		Moves events into a queue to be executed later. Allows functions that aren't ISR-safe
		to be used.
	API List: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

Header File: 1802.h		(CSE321 LCD API)


----------
Custom Functions
----------
	void secondPassed(void):
		Attached to Ticker tick to allow printing to the LCD after a second has passed
	void rowCycler(void):
		Cycles trough giving rows power