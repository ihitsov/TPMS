Installation and use of iTPMS based on CANBus messages for wheel speed in T7 cars with ESP/TCS

Installation of Arduino IDE and the extra libraries

1.	Download Arduino and install the stable version of Arduino IDE for your operation system:
https://www.arduino.cc/en/Main/Software
Accept all standard setting and install all of the required add-ons
2.	Start Arduino 
3.	Open the attached sketch (.ino)
4.	In File>Preferences find the empty field called “Additional Boards Manager URL” and paste this link there: http://arduino.esp8266.com/stable/package_esp8266com_index.json
5.	Open board manager from Tools>Board:Arduino/Genunino Uno>Boards Manager
6.	Search for esp8266, it will find only one result, click install
7.	Go to https://github.com/Seeed-Studio/CAN_BUS_Shield, click download or clone and download the zip file. Then extract it here: C:\Users\YourUserNmae\Documents\Arduino\libraries
10.	Replace the mcp_can.h and mcp_can_dfs.h files in C:\Program Files (x86)\Arduino\libraries\CAN_BUS_Shield-master with the ones provided in this project
11.	Go to Tool>Board and select NodeMCU 1.0 (Or the appropriate board for you)
12.	Go to Tool>Board and select the appropriate com port
13.	Go to Tool>Board and select the highest upload speed
14.	Restart Arduino IDE
15.	Open the Sketch and click the upload button (right arrow). If everything went well it will start to flash the logger.
16. Go to  while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_16MHz)), and set the correct frequency, based on the oscillator on your CAN board. Valid is 8 or 16 MHZ
If you have a normal Arduino board instead of ESP8266, you can skip steps 4-7

Usage:
Hook up the Arduino to P-bus
Calibration is started by pressing clear+down SID buttons. The SID will display a message that it is in calibration mode, followed by "calibration ready" message. 
At this point the TPMS should be ready.



