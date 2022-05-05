# Mini-DAQ

## Video Tutorials
- [MiniDAQ Chapter 1](https://vimeo.com/706663706)
- [MiniDAQ Chapter 2](https://vimeo.com/706663862)
- [MiniDAQ Chapter 2](https://vimeo.com/706664453)

## Arduino
- Hardware: [Arduino Mega 2560](https://store.arduino.cc/usa/mega-2560-r3)
- Software: [Arduino IDE](https://www.arduino.cc/en/software)
- File: Arduino.ino
- Notes:
  - Arduino software for acquiring signals from sensors and writing to EtherCAT network
  - Arduino Mega 2560 needs to be programmed with this code 

## Simulink
- Hardware: [Speedgoat](https://www.speedgoat.com/products-services/real-time-target-machines/baseline)
- Software: [MATLAB/Simulink](https://www.mathworks.com/)
- Files: MiniDAQ.slx initMiniDAQ.m postProcess.m 
- Notes:
  - Code that runs on the speedgoat necessary to visualize and store data

## EasyConfigurator
- Hardware: [EasyCAT](https://www.bausano.net/shop/en/home/1-arduino-ethercat.html)
- Software: [EasyConfigurator](https://www.bausano.net/en/hardware/ethercat-e-arduino/easycat.html)
- File: MiniDAQ.prj
- Notes:
  - This is only necessary to customize EtherCAT network if desired

## TwinCAT
- Hardware: [Arduino/EasyCAT]
- Software: [TwinCAT3](https://www.beckhoff.com/en-us/support/download-finder/software-and-tools/)
- File: MiniDAQ.sln
- Notes:
  - This is only necessary to customize EtherCAT network if desired
  - [Mathworks tutorial](https://www.mathworks.com/help/slrealtime/io_ref/configure-ethercat-hardware-by-using-twincat.html;jsessionid=dfc70ad347914cfaa3035acddc06)
