-------------------
About
-------------------
Project Description:   CSE 321 Project 3
Contribitor List:      Nick Minor


--------------------
Features
--------------------
A user can interact with the system by doing the following on the Matrix Keypad:
-Pressing A:
	Security Alarm Mode
-Pressing B:
	Distance Warning Mode
-Pressing 0-9:
	Specifying Distance

The LCD screen will display the distance to the nearest object in its path and the
distance at which the alarm will ring at.


--------------------
Required Materials
--------------------
-Nucleo L4R5ZI
-LCD
-Matrix Keypad
-HR-SR04 (Ultrasonic sensor)
-Buzzer
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
UM2179 Manual
	https://www.st.com/resource/en/user_manual/dm00368330-stm32-nucleo144-boards-mb1312-stmicroelectronics.pdf
JDH_1804_Datasheet
	Under "Learn and Documents section" https://www.seeedstudio.com/Grove-16x2-LCD-White-on-Blue.html
CSE321 LCD Library Files
HC-SR04 Datasheets:
	https://www.electroschematics.com/wp-content/uploads/2013/07/HC-SR04-datasheet-version-2.pdf  
	https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf  
Buzzer datasheet:
	http://tinkbox.ph/sites/tinkbox.ph/files/downloads/5V_BUZZER_MODULE.pdf  


--------------------
Getting Started
--------------------
Set up an empty project in Mbed Studio with MbedOS inside the 
project folder. Place the file "CSE321_project3_naminor_main.cpp" 
into the project as the only file with a main function. Select 
"NUCLEO-L4R5ZI" as the Target and connect the Nucleo to the 
computer using the micro USB cable. Connect the matrix keypad, 
LCD, ultrasonic sensor and buzzer to the Nucleo and breadboard. 
Build the project and run to start the program.


--------------------
CSE321_project3_naminor_IO_Interface.h:
--------------------
Date Created: 2021 12 09
Language: C++
Description:
	Contains IO_Interface and MatrixKeypad classes used in managing the 
	interaction between the input and output peripherals.	
	
----------
Classes
----------
Class:  IO_Interface
	Description:
        This class holds data used by peripherals as well as many functions that  
        facilitate interaction between the user, the inputs and the outputs.
    Variables:
        int input[]
            Integer array that holds the user's input for alarm range
        int mode
            Records the system's mode
        int currDist
            Holds the current distance from the object in cm
        int alarmDist
            Holds the value for the range that the user would like the buzzer to alarm at
        char lcd_output[]
            Holds the character array with the distance to the nearest object and the alarm
            range. This is printed to the LCD.
        bool timerCounting
            Records whether or not a timer is recording the time between a trigger and an echo
        bool allowUserInput
            Records if a user is allowed to specify a new distance using 0-9 keys
    Methods:
        IO_Interface
            Default constructor. Sets the current distance to 0, default alarm range to 100cm,
            the mode to 0, the input array indices to -1 to indicate no digits entered, 
            whether the timer is counting to false, and whether the user can use the 0-9
            keys to false.
        void changeMode
            Changes the current system mode to that specified
        int calcDistMetric
            Computes the user's input for the alarm range from the member int array input[]
            into an integer value and places it into the new alarm range alarmDist.
        void resetAlarmDist
            Resets the alarm array values to -1 to indicate a digit hasn't been entered yet
            for each index.
        void insertAlarmDist
            Parses the user's input from the matrix keypad into the member int array input[].
        void getOutputChars
            Converts the integer value for distance from the object, currDist, into a the member
            char array lcd_output[] for printing to the LCD screen.
        void ringBuzzer
            Does GPIO configurations to turn on the buzzer
        void silenceBuzzer
            Does GPIO configurations to turn off the buzzer
Class:  MatrixKeypad
	Description:
        This class sets up the GPIO connections for the matrix keypad and records the
        current row receiving power.
    Variables:
        int row
            Tracks the current row that power is being given to.
    Methods:
        MatrixKeypad()
            Default constructor. Initializes GPIO ports and sets the current row to 1.


--------------------
CSE321_project3_naminor_main.cpp:
--------------------
Date Created: 2021 12 09
Language: C++
Purpose: 
	This system determines if something has passed through an area of interest using an ultrasonic 
	sensor and notifies the user. The system can sound a buzzer if anything passes within a certain 
	distance and the distance to the nearest object is displayed on the LCD screen. The range that
	the alarm will sound within can be modified by the user using the matrix keypad. This system 
	can be used for safety applications such as keeping people away from dangerous areas, as a 
	security alarm or placed on a vehicle to help a driver avoid collisions. 

----------
Declarations
----------
Variables, functions, objects and APIs are used in this program.
Header Files:
	mbed.h
		Contains the APIs used to interact with the Nucleo
	1802.h
		API used to interact with the LCD
	CSE321_project3_naminor_IO_Interface.h
		Contains IO_Interface and MatrixKeypad classes
Variables and API objects:
	CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8) 
		LCD Screen object
	DigitalOut trigger(PA_6)              
		For sending a pulse to the ultrasonic sensor's trigger
	EventQueue q1(32 * EVENTS_EVENT_SIZE) 
		EventQueue for the ultrasonic sensor
	EventQueue q(32 * EVENTS_EVENT_SIZE)
		EventQueue for the matrix keypad
	Thread t1
		Thread for the ultrasonic sensor
	Thread t2
		Thread for the matrix keypad
	Ticker tick
		Sends a trigger every 500 ms
	Ticker KeypadCycler
		Ticker that will cycle through rows of the matrix keypad
	InterruptIn echo(PA_5, PullDown)
		For receiving the echo from the ultrasonic sensor
	int dist
		Tracks the distance to the closest object
	int correction
		Amount of time it takes to send the pulse
	Timer sentinel
		Records the time it takes between sending a trigger and receiving an echo
	IO_Interface io
		Interface object that will handle interaction between user's input with the buzzer and LCD
	MatrixKeypad mk
		Matrix keypad object that initializes the keypad's GPIO ports and records the current row
Functions:
	void rowCycler()
		Cycles through which rows receive power
	Interrupt Service Routines:
		void isr_start(void)
			Sends trigger to ultrasonic sensor and starts timer
		void isr_stop(void)
			Stops timer and allows printing to LCD after ultrasonic sensor receives an echo
		void isr_col1(void):
			Handler for column 1 of the matrix keypad:	1, 4, 7, *
		void isr_col2(void):
			Handler for column 1 of the matrix keypad:	2, 5, 8, 0
		void isr_col3(void):
			Handler for column 1 of the matrix keypad:	3, 6, 9, #
		void isr_col4(void):
			Handler for column 1 of the matrix keypad:	A, B, C, D
		void A_ChangeDist(void)
			Change alarm distance when A is pressed
		void B_CompInput(void)
			Confirm and apply the new alarm distance when B is pressed


----------
API and Built In Elements Used
----------
Header File: mbed.h
	InterruptIn
		Creates an interrupt triggered by an outside source (e.g. the matrix keypad).
	DigitalOut
		Configures and controls output pins.
	Timer
		Creates a stopwatch that tracks how much time has passed.
	Ticker	
		Sets up an interrupt to be continuously repeated at a specified time interval.
	EventQueue
		Moves events into a queue to be executed later. Allows functions that aren't ISR-safe
		to be used and is thread-safe.
	Thread
		Allows for creating and controlling of parallel tasks.
	Watchdog
		Resets the system to a safe state in the case of some failure.

	API List: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

Header File: 1802.h		(CSE321 LCD API)
	CSE321_LCD
		Allows interaction with the LCD screen with functionality such as printing and
		clearing text on the screen.

----------
Custom Functions
----------
void rowCycler()
	Cycles through which rows receive power
void isr_start(void)
	Sends trigger to ultrasonic sensor and starts timer
void isr_stop(void)
	Stops timer and allows printing to LCD after ultrasonic sensor receives an echo
void isr_col1(void):
	Handler for column 1 of the matrix keypad:	1, 4, 7, *
void isr_col2(void):
	Handler for column 1 of the matrix keypad:	2, 5, 8, 0
void isr_col3(void):
	Handler for column 1 of the matrix keypad:	3, 6, 9, #
void isr_col4(void):
	Handler for column 1 of the matrix keypad:	A, B, C, D
void A_ChangeDist(void)
	Change alarm distance when A is pressed
void B_CompInput(void)
	Confirm and apply the new alarm distance when B is pressed