/*
 Name:		STEP400_proto_r4_firmware.ino

 target:    Arduino Zero
 Created:	2020/04/08 10:24:41
 Author:	kanta
*/

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function
#include "samd.h" // for code sense

#include <cstdint>
#include <SPI.h>
#include <OSCMessage.h>
#include <Ethernet.h>
#include <Adafruit_SleepyDog.h>
#include <powerSTEP01ArduinoLibrary.h>

#define ledPin	13u
const uint8_t dipSwPin[8] = { A5,SCL,7u,SDA,2u,9u,3u,0u };
const uint8_t limitSwPin[4] = { 1u,5u,8u,A1 };
#define SD_CS_PIN	4u
#define SD_DETECT_PIN   A4
#define SETUP_SW_PIN    31u

#define NUM_POWERSTEP   4
#define POWERSTEP_MISO	6u	// D6 /SERCOM3/PAD[2] miso
#define POWERSTEP_MOSI	11u	// D11/SERCOM3/PAD[0] mosi
#define POWERSTEP_SCK	12u	// D12/SERCOM3/PAD[3] sck
SPIClass powerStepSPI(&sercom3, POWERSTEP_MISO, POWERSTEP_SCK, POWERSTEP_MOSI, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);// MISO/SCK/MOSI pins

#define POWERSTEP_CS_PIN A0
#define POWERSTEP_RESET_PIN A2
powerSTEP powerSteps[] = {
    powerSTEP(3, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(2, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(1, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(0, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN)
};

#define TARGET_MOTOR_ALL    255u
#define TARGET_MOTOR_FIRST  1u
#define TARGET_MOTOR_LAST   4u
//#define TARGET_MOTOR_ERROR  254u

byte mac[] = { 0x60, 0x95, 0xCE, 0x10, 0x02, 0x00 },
    myId = 0;
IPAddress myIp(10, 0, 0, 100);
IPAddress destIp(10, 0, 0, 10);
unsigned int outPort = 50100;
unsigned int inPort = 50000;
EthernetUDP Udp;

#define W5500_RESET_PIN A3

#define POLL_DURATION   10

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(W5500_RESET_PIN, OUTPUT);
    pinMode(SD_DETECT_PIN, INPUT_PULLUP);
    pinMode(SETUP_SW_PIN, INPUT_PULLUP);

    for (auto i=0; i < NUM_POWERSTEP; i++)
    {
        //pinMode(limitSwPin[i], INPUT_PULLUP);
    }
    for (auto i=0; i<8; ++i)
    {
        pinMode(dipSwPin[i], INPUT_PULLUP);
        myId |= (!digitalRead(dipSwPin[i])) << i;
    }
    mac[5] += myId;
    myIp[3] += myId;
    outPort += myId;

    pinMode(POWERSTEP_RESET_PIN, OUTPUT);
    pinMode(POWERSTEP_CS_PIN, OUTPUT);
    pinMode(POWERSTEP_MOSI, OUTPUT);
    pinMode(POWERSTEP_MISO, INPUT);
    pinMode(POWERSTEP_SCK, OUTPUT);
    digitalWrite(POWERSTEP_RESET_PIN, HIGH);
    digitalWrite(POWERSTEP_RESET_PIN, LOW);
    delay(10);
    digitalWrite(POWERSTEP_RESET_PIN, HIGH);
    digitalWrite(POWERSTEP_CS_PIN, HIGH);
    powerStepSPI.begin();
    pinPeripheral(POWERSTEP_MOSI, PIO_SERCOM_ALT);
    pinPeripheral(POWERSTEP_SCK, PIO_SERCOM_ALT);
    pinPeripheral(POWERSTEP_MISO, PIO_SERCOM_ALT);
    powerStepSPI.setDataMode(SPI_MODE3);

    for (uint8_t i = 0; i < NUM_POWERSTEP; i++)
    {
        powerSteps[i].SPIPortConnect(&powerStepSPI);
        powerSteps[i].configStepMode(STEP_FS_128);

        powerSteps[i].setMaxSpeed(10000);
        powerSteps[i].setFullSpeed(2000);
        powerSteps[i].setAcc(2000);
        powerSteps[i].setDec(2000);
        powerSteps[i].setSlewRate(SR_980V_us);
        powerSteps[i].setOCThreshold(15); // 5A for 0.1ohm shunt resistor
        powerSteps[i].setOCShutdown(OC_SD_ENABLE);
        powerSteps[i].setPWMFreq(PWM_DIV_1, PWM_MUL_0_75);
        powerSteps[i].setVoltageComp(VS_COMP_DISABLE);
        powerSteps[i].setSwitchMode(SW_USER);
        powerSteps[i].setOscMode(EXT_24MHZ_OSCOUT_INVERT);
        //powerSteps[i].setOscMode(INT_16MHZ);
        powerSteps[i].setRunKVAL(16);
        powerSteps[i].setAccKVAL(16);
        powerSteps[i].setDecKVAL(16);
        powerSteps[i].setHoldKVAL(4);
        powerSteps[i].setParam(ALARM_EN, 0x8F); // disable ADC UVLO (divider not populated),
        // disable stall detection (not configured),
        // disable switch (not using as hard stop)
        delay(1);
        powerSteps[i].getStatus(); // clears error flags
    }

    digitalWrite(W5500_RESET_PIN, HIGH);
    delay(1);
    digitalWrite(W5500_RESET_PIN, LOW);
    delay(1);
    digitalWrite(W5500_RESET_PIN, HIGH);
    Ethernet.begin(mac, myIp);
    Udp.begin(inPort);
}

void sendOneDatum(char* address, int32_t data) {
    OSCMessage newMes(address);
    newMes.add((int32_t)data);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}

void sendTwoData(char* address, int32_t data1, int32_t data2) {
    OSCMessage newMes(address);
    newMes.add(data1).add(data2);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}

void setDestIp(OSCMessage& msg, int addrOffset) {
    bool bIpUpdated = (destIp[3] != Udp.remoteIP()[3]);
    destIp = Udp.remoteIP();
    sendTwoData("/newDestIp", destIp[3], bIpUpdated);
}

#pragma region kval_commands_osc_listener
void setKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int hold = constrain(msg.getInt(1), 0, 255);
    int run = constrain(msg.getInt(2), 0, 255);
    int acc = constrain(msg.getInt(3), 0, 255);
    int dec = constrain(msg.getInt(4), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setHoldKVAL(hold);
        powerSteps[target - TARGET_MOTOR_FIRST].setRunKVAL(run);
        powerSteps[target - TARGET_MOTOR_FIRST].setAccKVAL(acc);
        powerSteps[target - TARGET_MOTOR_FIRST].setDecKVAL(dec);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setHoldKVAL(hold);
            powerSteps[i].setRunKVAL(run);
            powerSteps[i].setAccKVAL(acc);
            powerSteps[i].setDecKVAL(dec);
        }
    }
}

void setHoldKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setHoldKVAL(kvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setHoldKVAL(kvalInput);
        }
    }
}
void setRunKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);

    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setRunKVAL(kvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setRunKVAL(kvalInput);
        }
    }
}
void setAccKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setAccKVAL(kvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setAccKVAL(kvalInput);
        }
    }
}
void setDecKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setDecKVAL(kvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setDecKVAL(kvalInput);
        }
    }
}

void getKVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        getKVAL(target);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            getKVAL(i + 1);
        }
    }
}
void getKVAL(uint8_t target) {
    OSCMessage newMes("/kval");
    newMes.add((int32_t)target);
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getHoldKVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getRunKVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getAccKVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getDecKVAL());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}
#pragma endregion

#pragma region tval_commands_osc_listener
void setTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int hold = constrain(msg.getInt(1), 0, 255);
    int run = constrain(msg.getInt(2), 0, 255);
    int acc = constrain(msg.getInt(3), 0, 255);
    int dec = constrain(msg.getInt(4), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setHoldTVAL(hold);
        powerSteps[target - TARGET_MOTOR_FIRST].setRunTVAL(run);
        powerSteps[target - TARGET_MOTOR_FIRST].setAccTVAL(acc);
        powerSteps[target - TARGET_MOTOR_FIRST].setDecTVAL(dec);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setHoldTVAL(hold);
            powerSteps[i].setRunTVAL(run);
            powerSteps[i].setAccTVAL(acc);
            powerSteps[i].setDecTVAL(dec);
        }
    }
}

void setHoldTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setHoldTVAL(tvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setHoldTVAL(tvalInput);
        }
    }
}
void setRunTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, 255);

    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setRunTVAL(tvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setRunTVAL(tvalInput);
        }
    }
}
void setAccTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setAccTVAL(tvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setAccTVAL(tvalInput);
        }
    }
}
void setDecTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, 255);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setDecTVAL(tvalInput);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setDecTVAL(tvalInput);
        }
    }
}

void getTVAL(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        getTVAL(target);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            getTVAL(i + 1);
        }
    }
}
void getTVAL(uint8_t target) {
    OSCMessage newMes("/tval");
    newMes.add((int32_t)target);
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getHoldTVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getRunTVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getAccTVAL());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getDecTVAL());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}
#pragma endregion

#pragma region speed_commands_osc_listener

void setSpdProfile(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float max = msg.getFloat(1);
    float min = msg.getFloat(2);
    float acc = msg.getFloat(3);
    float dec = msg.getFloat(4);

    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMaxSpeed(max);
        powerSteps[target - TARGET_MOTOR_FIRST].setMinSpeed(min);
        powerSteps[target - TARGET_MOTOR_FIRST].setAcc(acc);
        powerSteps[target - TARGET_MOTOR_FIRST].setDec(dec);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMaxSpeed(max);
            powerSteps[i].setMinSpeed(min);
            powerSteps[i].setAcc(acc);
            powerSteps[i].setDec(dec);
        }
    }
}

void setMaxSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMaxSpeed(stepsPerSecond);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMaxSpeed(stepsPerSecond);
        }
    }
}
void setMinSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMinSpeed(stepsPerSecond);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMinSpeed(stepsPerSecond);
        }
    }
}
void setFullstepSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setFullSpeed(stepsPerSecond);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setFullSpeed(stepsPerSecond);
        }
    }
}
void setAcc(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float stepsPerSecondPerSecond = msg.getFloat(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setAcc(stepsPerSecondPerSecond);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setAcc(stepsPerSecondPerSecond);
        }
    }
}
void setDec(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    float stepsPerSecondPerSecond = msg.getFloat(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setDec(stepsPerSecondPerSecond);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setDec(stepsPerSecondPerSecond);
        }
    }
}

void setSpdProfileRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int max = msg.getInt(1);
    int min = msg.getInt(2);
    int acc = msg.getInt(3);
    int dec = msg.getInt(4);

    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMaxSpeedRaw(max);
        powerSteps[target - TARGET_MOTOR_FIRST].setMinSpeedRaw(min);
        powerSteps[target - TARGET_MOTOR_FIRST].setAccRaw(acc);
        powerSteps[target - TARGET_MOTOR_FIRST].setDecRaw(dec);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMaxSpeedRaw(max);
            powerSteps[i].setMinSpeedRaw(min);
            powerSteps[i].setAccRaw(acc);
            powerSteps[i].setDecRaw(dec);
        }
    }
}

void setMaxSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int t = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMaxSpeedRaw(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMaxSpeedRaw(t);
        }
    }
}
void setMinSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int t = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMinSpeedRaw(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMinSpeedRaw(t);
        }
    }
}
void setFullstepSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int t = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setFullSpeedRaw(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setFullSpeedRaw(t);
        }
    }
}
void setAccRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int t = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setAccRaw(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setAccRaw(t);
        }
    }
}
void setDecRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int t = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setDecRaw(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setDecRaw(t);
        }
    }
}

void getSpdProfile(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        getSpdProfile(target);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            getSpdProfile(i);
        }
    }
}
void getSpdProfile(uint8_t target) {
    OSCMessage newMes("/spd");
    newMes.add((int32_t)target);
    newMes.add((float)powerSteps[target - TARGET_MOTOR_FIRST].getMaxSpeed());
    newMes.add((float)powerSteps[target - TARGET_MOTOR_FIRST].getMinSpeed());
    newMes.add((float)powerSteps[target - TARGET_MOTOR_FIRST].getAcc());
    newMes.add((float)powerSteps[target - TARGET_MOTOR_FIRST].getDec());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}

void getSpdProfileRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        getSpdProfileRaw(target);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            getSpdProfileRaw(i);
        }
    }
}
void getSpdProfileRaw(uint8_t target) {
    OSCMessage newMes("/spdRaw");
    newMes.add((int32_t)target);
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getMaxSpeedRaw());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getMinSpeedRaw());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getAccRaw());
    newMes.add((int32_t)powerSteps[target - TARGET_MOTOR_FIRST].getDecRaw());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}
#pragma endregion

#pragma region operational_commands_osc_listener

void getPos(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        sendTwoData("/pos", target, powerSteps[target - TARGET_MOTOR_FIRST].getPos());
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            sendTwoData("/pos", i + TARGET_MOTOR_FIRST, powerSteps[i].getPos());
        }
    }
}
void getMark(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        sendTwoData("/mark", target, powerSteps[target - TARGET_MOTOR_FIRST].getMark());
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            sendTwoData("/mark", i + TARGET_MOTOR_FIRST, powerSteps[i].getMark());
        }
    }
}

void run(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);

    float stepsPerSec = msg.getFloat(1);
    boolean dir = stepsPerSec > 0;
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].run(dir, abs(stepsPerSec));
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].run(dir, abs(stepsPerSec));
        }
    }
}

void runRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);

    int32_t speed = msg.getInt(1);
    boolean dir = speed > 0;
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].runRaw(dir, abs(speed));
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].runRaw(dir, abs(speed));
        }
    }
}

void move(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int32_t steps = msg.getInt(1);
    boolean dir = steps > 0;
    steps = abs(steps);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].move(dir, steps);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].move(dir, steps);
        }
    }
}
void goTo(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int32_t pos = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goTo(pos);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goTo(pos);
        }
    }
}
void goToDir(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);

    boolean dir = msg.getInt(1) > 0;
    int32_t pos = msg.getInt(2);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goToDir(dir, pos);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goToDir(dir, pos);
        }
    }
}
// todo: action??????
void goUntil(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    bool action = msg.getInt(1)>0;
    float stepsPerSec = msg.getFloat(2);
    bool dir = stepsPerSec > 0;
    stepsPerSec = abs(stepsPerSec);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goUntil(action, dir, stepsPerSec);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goUntil(action, dir, stepsPerSec);
        }
    }
}
void goUntilRaw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    bool action = msg.getInt(1) > 0;
    int32_t speed = msg.getInt(2);
    bool dir = speed > 0;
    speed = abs(speed);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goUntilRaw(action, dir, speed);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goUntilRaw(action, dir, speed);
        }
    }
}

void releaseSw(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);

    uint8_t action = msg.getInt(1);
    uint8_t dir = constrain(msg.getInt(2), 0, 1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].releaseSw(action, dir);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].releaseSw(action, dir);
        }
    }
}
void goHome(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goHome();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goHome();
        }
    }
}
void goMark(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].goMark();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].goMark();
        }
    }
}
void setMark(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int32_t newMark = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setMark(newMark);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setMark(newMark);
        }
    }
}
void setPos(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    int32_t newPos = msg.getInt(1);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setPos(newPos);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setPos(newPos);
        }
    }
}
void resetPos(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].resetPos();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].resetPos();
        }
    }
}
void resetDev(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].resetDev();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].resetDev();
        }
    }
}
void softStop(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].softStop();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].softStop();
        }
    }
}
void hardStop(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].hardStop();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].hardStop();
        }
    }
}
void softHiZ(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].softHiZ();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].softHiZ();
        }
    }
}
void hardHiZ(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].hardHiZ();
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].hardHiZ();
        }
    }
}
#pragma endregion

#pragma region PowerSTEP01_config_osc_listener

void getStatus(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        sendTwoData("/status", target, powerSteps[target - TARGET_MOTOR_FIRST].getStatus());
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            sendTwoData("/status", i + TARGET_MOTOR_FIRST, powerSteps[i].getStatus());
        }
    }
}

void getStatusList(OSCMessage& msg, int addrOffset) {
    OSCMessage newMes("/statusList");
    for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
        newMes.add((int32_t)powerSteps[i].getStatus());
    }

    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}

void configStepMode(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t stepMode = constrain(msg.getInt(1), STEP_FS, STEP_FS_128);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].configStepMode(stepMode);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].configStepMode(stepMode);
        }
    }
}

void getStepMode(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        sendTwoData("/stepMode", target, powerSteps[target - TARGET_MOTOR_FIRST].getStepMode());
    }
    else if (target == TARGET_MOTOR_ALL) {
        OSCMessage newMes("/stepMode");
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            newMes.add((int32_t)powerSteps[i].getStepMode());
        }
        Udp.beginPacket(destIp, outPort);
        newMes.send(Udp);
        Udp.endPacket();
        newMes.empty();
    }
}

void getSwMode(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        sendTwoData("/swMode", target, powerSteps[target - TARGET_MOTOR_FIRST].getSwitchMode());
    }
    else if (target == TARGET_MOTOR_ALL) {
        OSCMessage newMes("/swMode");
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            newMes.add((int32_t)powerSteps[i].getSwitchMode());
        }
        Udp.beginPacket(destIp, outPort);
        newMes.send(Udp);
        Udp.endPacket();
        newMes.empty();
    }
}

void setSwMode(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t switchMode = msg.getInt(1) > 0;
    switchMode = (switchMode) ? SW_USER : SW_HARD_STOP;
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setSwitchMode(switchMode);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setSwitchMode(switchMode);
        }
    }
}

void setVoltageMode(OSCMessage& msg, int addrOffset) {
    uint8_t target = msg.getInt(0);
    uint8_t stepMode = constrain(msg.getInt(1), STEP_FS, STEP_FS_128);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        powerSteps[target - TARGET_MOTOR_FIRST].setVoltageMode(stepMode);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setVoltageMode(stepMode);
        }
    }
}

void setCurrentMode(OSCMessage& msg, int addrOffset) {
    int t = 0;
    uint8_t target = msg.getInt(0);
    uint8_t stepMode = constrain(msg.getInt(1), STEP_FS, STEP_FS_16);
    if (TARGET_MOTOR_FIRST <= target && target <= TARGET_MOTOR_LAST) {
        target -= 1;
        powerSteps[target].setCurrentMode(stepMode);
        powerSteps[target].setHoldTVAL(t);
        powerSteps[target].setRunTVAL(t);
        powerSteps[target].setAccTVAL(t);
        powerSteps[target].setDecTVAL(t);
    }
    else if (target == TARGET_MOTOR_ALL) {
        for (uint8_t i = 0; i < NUM_POWERSTEP; i++) {
            powerSteps[i].setCurrentMode(stepMode);
            powerSteps[i].setHoldTVAL(t);
            powerSteps[i].setRunTVAL(t);
            powerSteps[i].setAccTVAL(t);
            powerSteps[i].setDecTVAL(t);
        }
    }
}

#pragma endregion PowerSTEP01_config_osc_listener

void OSCMsgReceive() {

    OSCMessage msgIN;
    int size;
    if ((size = Udp.parsePacket()) > 0) {
        while (size--)
            msgIN.fill(Udp.read());

        if (!msgIN.hasError()) {
            msgIN.route("/setDestIp", setDestIp);
            //msgIN.route("/setSwFlag", setSwFlag);
            //msgIN.route("/setBusyFlag", setBusyFlag);

            //msgIN.route("/getStatus", getStatus);
           // msgIN.route("/getSw", getSw);
            //msgIN.route("/getBusy", getBusy);
            //msgIN.route("/getDir", getDir);

            msgIN.route("/configStepMode", configStepMode);
            msgIN.route("/getStepMode", getStepMode);

            msgIN.route("/setVoltageMode", setVoltageMode);
            msgIN.route("/setCurrentMode", setCurrentMode);

            msgIN.route("/setSpdProfile", setSpdProfile);
            msgIN.route("/setMaxSpeed", setMaxSpeed);
            msgIN.route("/setMinSpeed", setMinSpeed);
            msgIN.route("/setFullstepSpeed", setFullstepSpeed);
            msgIN.route("/setAcc", setAcc);
            msgIN.route("/setDec", setDec);
            msgIN.route("/getSpdProfile", getSpdProfile);

            msgIN.route("/setSpdProfileRaw", setSpdProfileRaw);
            msgIN.route("/setMaxSpeedRaw", setMaxSpeedRaw);
            msgIN.route("/setMinSpeedRaw", setMinSpeedRaw);
            msgIN.route("/setFullstepSpeedRaw", setFullstepSpeedRaw);
            msgIN.route("/setAccRaw", setAccRaw);
            msgIN.route("/setDecRaw", setDecRaw);
            msgIN.route("/getSpdProfileRaw", getSpdProfileRaw);

            msgIN.route("/setKVAL", setKVAL);
            msgIN.route("/setAccKVAL", setAccKVAL);
            msgIN.route("/setDecKVAL", setDecKVAL);
            msgIN.route("/setRunKVAL", setRunKVAL);
            msgIN.route("/setHoldKVAL", setHoldKVAL);

            msgIN.route("/getKVAL", getKVAL);

            msgIN.route("/setTVAL", setTVAL);
            msgIN.route("/setAccTVAL", setAccTVAL);
            msgIN.route("/setDecTVAL", setDecTVAL);
            msgIN.route("/setRunTVAL", setRunTVAL);
            msgIN.route("/setHoldTVAL", setHoldTVAL);

            msgIN.route("/getTVAL", getTVAL);

            msgIN.route("/getSwMode", getSwMode);
            msgIN.route("/setSwMode", setSwMode);

            msgIN.route("/getPos", getPos);
            msgIN.route("/getMark", getMark);
            msgIN.route("/run", run);
            msgIN.route("/runRaw", runRaw);
            msgIN.route("/move", move);
            msgIN.route("/goTo", goTo);
            msgIN.route("/goToDir", goToDir);
            msgIN.route("/goUntil", goUntil);
            msgIN.route("/goUntilRaw", goUntilRaw);
            msgIN.route("/releaseSw", releaseSw);
            msgIN.route("/goHome", goHome);
            msgIN.route("/goMark", goMark);
            msgIN.route("/setMark", setMark);
            msgIN.route("/setPos", setPos);
            msgIN.route("/resetPos", resetPos);
            msgIN.route("/resetDev", resetDev);
            msgIN.route("/softStop", softStop);
            msgIN.route("/hardStop", hardStop);
            msgIN.route("/softHiZ", softHiZ);
            msgIN.route("/hardHiZ", hardHiZ);
        }
    }
}
void loop() {
    uint32_t currentTimeMillis = millis(),
               currentTimeMicros = micros();
    static uint32_t lastPollTime = 0;

    if ( (uint32_t)(currentTimeMillis - lastPollTime) >= POLL_DURATION )
    {
        //
        lastPollTime = currentTimeMillis;
    }
    OSCMsgReceive();
}
