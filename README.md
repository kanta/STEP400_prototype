# STEP400_prototype
![STEP400_proto_r4](http://ponoor.com/data/img/STEP400/STEP400_proto_r4.JPG)

4 axis stepper motor driver board which can control over Ethernet. The current firmware is based on [Open Sound Control](http://opensoundcontrol.org/) protocol.
[Arduino Zero](https://www.arduino.cc/en/Guide/ArduinoZero) and official [Ethernet library](https://www.arduino.cc/en/reference/ethernet) compatible.

Stepper driver chips are STMicroelectronics's [powerSTEP01](https://www.st.com/en/motor-drivers/powerstep01.html) which provide most of functionalities as a stepper driver.


# Repository Contents
- **/rev4/STEP400_proto_r4_firmware/STEP400_proto_r4_firmware/** - Source files for the firmware. Some Visual Studio files are included but you can open .ino file with Arduino IDE.
- **/rev4/hardware/** - PCB design files for Eagle.

## Firmware dependency
- **[OSC Library](https://github.com/CNMAT/OSC)** from CNMAT
- **[Ponoor_PowerSTEP01_Library](https://github.com/ponoor/Ponoor_PowerSTEP01_Library)**

# Documentation
Documentations are not available yet but now working on [Wiki](https://github.com/kanta/STEP400_prototype/wiki).

# License Information
This product is open source.
Various bits of the code have different licenses applied.
Distributed as-is; no warranty is given.
