# EMV3_HardwareTest
This repository contains a hardware test program for the EMV3 platform. It is intended to be used
to test the hardware and in limited cases test installed field devices.

/**********************************************************************************************************************/
Caution: This program should only be used on an uninstalled system OR a system that has the relay outputs disconnected.
When testing the Digital IO (option 7) the relays are energized/de-energized every 2 seconds until a key is pressed.
/**********************************************************************************************************************/

Program Build:
Once the repository has been downloaded AND PlatformIO has updated the all the libraries 
you will need to make 2 manual changes prior to building the code. These manual chan ges are required
for the graphics testing.
Step 1: Add lv_conf.h
  In the "src\extras" folder there are 2 files which need to be copied. The first file called "lv_conf.h.extra" 
  needs to be copied to ".pio\libdeps\esp32dev" and renamed to "lv_conf.h". 
Step 2: Change User_Setup.h
  The second file called "User_Setup.h.extra" needs to be copied to ".pio\libdeps\esp32dev\TFT_eSPI".
  There is a file already there called "User_Setup.h" and this needs to be deleted. 
  Now rename "User_Setup.h.extra" to "User_Setup.h". The project should be ready to build.

Note: If either the lvgl or TFT_eSPI libraries change, you must redo steps 1 and 2 above.

Program Use:
  This test program uses a simple ascii menu system for the user interface. The top level menu will be displayed on the serial console.
  The default baud rate is 115000. All of the commands are 1 keystroke and do not require a return. 
  Typing h or H or ? will return back to the top menu.
  The top menu will look as follows:
  
1. Show ESP information
2. Enter wifi credentials
3. Connect to wifi network (enter SSID to be able to connect)
4. Current wifi info
5. Scan wifi networks
6. Scan I2C devices
7. Test Digital IO
8. Scan 1 Wire devices
9. Test Analog Inputs
0. Restart
t/T. TFT Test
c/C. Calibrate Touchscreen and TFT Test

h, H, ? <cr>. This menu

/**********************************************************************************************************************/
Caution: This program should only be used on an uninstalled system OR a system that has the relay outputs disconnected.
When testing the Digital IO (option 7) the relays are energized/de-energized every 2 seconds until a key is pressed.
/**********************************************************************************************************************/
  
