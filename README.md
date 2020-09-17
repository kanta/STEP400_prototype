## STEP400_prototype
![STEP400_proto_r4](http://ponoor.com/manage/wp-content/uploads/2020/09/step400-angled-view.jpg)

STEP400 is a 4-axis stepper motor driver designed for art or design projects. STEP400 combines the following elements into one single board:

- [Arduino Zero](https://www.arduino.cc/en/Guide/ArduinoZero)
- Ethernet shield
- Four stepper motor drivers
- Sensor inputs for homing and limiting

The current firmware is focused on working with [Open Sound Control](http://opensoundcontrol.org/) (OSC) via Ethernet, which is a common protocol for creative coding environments, like openFrameworks, Processing, Max, Unity, or Touch Designer.

Stepper driver chips are STMicroelectronics's [powerSTEP01](https://www.st.com/en/motor-drivers/powerstep01.html) which provide most of functionalities as a stepper driver.

## Repository Contents

This repo is temporal place for working. All information will be moved to a new repo when the clean up is done.

### Firmware
[/rev4/STEP400_proto_r4_firmware/STEP400_proto_r4_firmware/](https://github.com/kanta/STEP400_prototype/tree/master/rev4/STEP400_proto_r4_firmware/STEP400_proto_r4_firmware)

Source files for the firmware. Some Visual Studio files are included but you can open .ino file with Arduino IDE.

### Hardware
[/rev4/hardware/](https://github.com/kanta/STEP400_prototype/tree/master/rev4/hardware)

PCB design files for Eagle.

## Firmware dependency
- **[OSC Library](https://github.com/CNMAT/OSC)** from CNMAT
- **[Ponoor_PowerSTEP01_Library](https://github.com/ponoor/Ponoor_PowerSTEP01_Library)**
- **[ArduinoJSON_Library](https://arduinojson.org/)**

## Examples
### Max
https://github.com/kanta/STEP400_prototype/blob/master/rev4/STEP400_proto_r4.maxpat

### openFrameworks
https://github.com/ponoor/step-series-example-openFrameworks

### Unity
https://github.com/ponoor/step-series-example-Unity

### Touch Designer
https://github.com/ponoor/step-series-example-TouchDesigner

## Documentation
Now working on [Wiki](https://github.com/kanta/STEP400_prototype/wiki).
