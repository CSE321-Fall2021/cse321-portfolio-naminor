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
-Pressing C:
	Specify new distance
-Pressing D:
	Toggle specification between cm and inches
-Pressing 0-9:
	Specifying Distance


The LCD screen will:
-prompt the user for their inputs.
-display the distance to the nearest object in its path.


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
JDH_1804_Datasheet
CSE321 LCD Library Files
HR-SR04 Datasheets:
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
CSE321_project3_naminor_main.cpp:
--------------------
Date Created: In the future!
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
	None yet!
Functions:
	None yet!


----------
API and Built In Elements Used
----------
Header File: mbed.h
	API List: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

Header File: 1802.h		(CSE321 LCD API)


----------
Custom Functions
----------
None yet!