## Project Overview
The aim of this project was to use the Warp firmware as a base to create a device to measure and display rowing stroke rate using the FRDM-KL03Z development board.

This readme is not intended to cover the entirety of the warp firmware; for information about building and running the Warp firmware see /README.md, and for information about the overall implementation see /src/boot/ksdk1.1.0/README.md. This readme is intended to cover the files and code used specifically to accomplish the aims of this project on top of the base firmware.

The main logic for the rate meter is implemented in main() in /ksdk1.1.0/boot.c. There are three other components used in the project, one built in to the KL03Z and 2 external. The built in MMA8451Q digital accelerometer is used to measure the acceleration needed to indentify strokes. The SSD1331 OLED screen is used to display the output (the stroke rate). The INA219 current meter is used for analysis of power usage.

### Source file descriptions
##### 'boot.c'
This is the core of the system, and main() contains the key logic for measuring rate.

##### 'devMMA8451Q.c'
This is the device driver for the accelerometer. I added a function to return the acceleration in just one axis in a more useable form to simplify the main algorithm.

##### 'devSSD1331.c'
This is the device driver for the OLED screen. I added functions to clear the screen, draw a rectangle, and draw numbers on a virtual 7 segment display.

##### 'devINA219.c'
This is the device driver for the current meter. I implemented this driver to provide a convenient way to read from the device.

### Using the firmware
Once the program is loaded onto the development board, using it is very simple. Connect the board to usb power (or 5V battery source) and wait for the screen to turn on. Oscillating the device in the X direction (parallel to the short side of the development board) will cause the OLED screen to display the rate (in 'strokes' per minute) that it is being oscillated at.