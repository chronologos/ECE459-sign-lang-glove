# ECE459-sign-lang-glove
Embedded systems design project.

We are building a glove that recognizes ASL (American Sign Language) and converts it to text output.

## Materials
- mbed LPC1768 microcontroller
- 6 flex sensors (4 fingers, thumb, palm)
- LSM9DS0 Inertial Measurement Unit
- HC-06 Bluetooth Module

## Mbed code
`consensus.h` - Contains functions which establish consensus on what letter was formed by multiple polls of a gesture.
`flex_sensor.h` - Handles all interactions with Flex Sensors and ADC 
`imu_fsm.h` - Implements a mealy machine that we use to synthesize our different sensor inputs.


## Client-side Code
Client-side python script (full functionality on OSX only) has two modes of operation:
1: Test mode receives input and asks user what they actually meant to type. Stats are stored for future reference.
2: Input mode converts character received through BT to a keypress.
