/*
 Name:		STEP400_proto_r4_firmware.ino

 target:    Arduino Zero
 Created:	2020/04/08 10:24:41
 Author:	kanta
*/

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function
//#include <cstdint>
#include <SPI.h>
#include <OSCMessage.h>
#include <Ethernet.h>
#include <Adafruit_SleepyDog.h>
#include <Ponoor_PowerSTEP01Library.h>

#define COMPILE_DATE __DATE__
#define COMPILE_TIME __TIME__
constexpr auto PROJECT_NAME = "STEP400proto_r4";

#define ledPin	13u
const uint8_t dipSwPin[8] = { A5,SCL,7u,SDA,2u,9u,3u,0u };
const uint8_t limitSwPin[4] = { 1u,5u,8u,A1 };
#define SD_CS_PIN	4u
#define SD_DETECT_PIN   A4
#define SETUP_SW_PIN    31u

#define NUM_OF_MOTOR   (4)
#define POWERSTEP_MISO	6u	// D6 /SERCOM3/PAD[2] miso
#define POWERSTEP_MOSI	11u	// D11/SERCOM3/PAD[0] mosi
#define POWERSTEP_SCK	12u	// D12/SERCOM3/PAD[3] sck
SPIClass powerStepSPI(&sercom3, POWERSTEP_MISO, POWERSTEP_SCK, POWERSTEP_MOSI, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);// MISO/SCK/MOSI pins

#define POWERSTEP_CS_PIN A0
#define POWERSTEP_RESET_PIN A2
powerSTEP stepper[] = {
    powerSTEP(3, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(2, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(1, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN),
    powerSTEP(0, POWERSTEP_CS_PIN, POWERSTEP_RESET_PIN)
};

#define MOTOR_ID_ALL    255
#define MOTOR_ID_FIRST  1
#define MOTOR_ID_LAST   4

#define TVAL_LIMIT_VAL  128 // approx. 5A
// KVAL and TVAL storage.
uint8_t kvalHold[NUM_OF_MOTOR], kvalRun[NUM_OF_MOTOR], kvalAcc[NUM_OF_MOTOR], kvalDec[NUM_OF_MOTOR];
uint8_t tvalHold[NUM_OF_MOTOR], tvalRun[NUM_OF_MOTOR], tvalAcc[NUM_OF_MOTOR], tvalDec[NUM_OF_MOTOR];
bool isCurrentMode[NUM_OF_MOTOR];;

byte mac[] = { 0x60, 0x95, 0xCE, 0x10, 0x02, 0x00 },
    myId = 0;
#define IP_OFFSET	100
IPAddress myIp(10, 0, 0, IP_OFFSET);
IPAddress destIp(10, 0, 0, 10);
unsigned int outPort = 50100;
unsigned int inPort = 50000;
EthernetUDP Udp;
boolean isDestIpSet = false;

#define W5500_RESET_PIN A3

#define STATUS_POLL_PERIOD   10 // [ms]

// these values will be initialized at setup()
bool busy[NUM_OF_MOTOR];
bool flag[NUM_OF_MOTOR];
bool HiZ[NUM_OF_MOTOR];
bool homeSwState[NUM_OF_MOTOR];
bool dir[NUM_OF_MOTOR];
bool uvloStatus[NUM_OF_MOTOR];
uint8_t motorStatus[NUM_OF_MOTOR];
uint8_t thermalStatus[NUM_OF_MOTOR];

bool reportBUSY[NUM_OF_MOTOR];
bool reportFLAG[NUM_OF_MOTOR];
bool reportHiZ[NUM_OF_MOTOR];
bool reportHomeSwStatus[NUM_OF_MOTOR];
bool reportDir[NUM_OF_MOTOR];
bool reportMotorStatus[NUM_OF_MOTOR];

bool reportSwEvn[NUM_OF_MOTOR];
bool reportCommandError[NUM_OF_MOTOR];
bool reportUVLO[NUM_OF_MOTOR];
bool reportThermalStatus[NUM_OF_MOTOR];
bool reportOCD[NUM_OF_MOTOR];
bool reportStall[NUM_OF_MOTOR];

bool limitSwState[NUM_OF_MOTOR];
bool reportLimitSwStatus[NUM_OF_MOTOR];
bool limitSwMode[NUM_OF_MOTOR];

// servo mode
uint32_t lastServoUpdateTime;
int32_t targetPosition[NUM_OF_MOTOR]; // these values will be initialized at setup()
float kP[NUM_OF_MOTOR], kI[NUM_OF_MOTOR], kD[NUM_OF_MOTOR];
boolean isServoMode[NUM_OF_MOTOR];
constexpr auto position_tolerance = 0; // steps

// Tx, Rx LED
bool rxLedEnabled = false, txLedEnabled = false;
uint32_t RXL_blinkStartTime, TXL_blinkStartTime;
#define RXL_TXL_BLINK_DURATION	30 // ms

void setUSBPriority()
{
    const auto irqn = USB_IRQn;
    NVIC_DisableIRQ(irqn);
    NVIC_SetPriority(irqn, 2);
    NVIC_EnableIRQ(irqn);
}

void setup() {
    //setUSBPriority();
    pinMode(ledPin, OUTPUT);
    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(W5500_RESET_PIN, OUTPUT);
    pinMode(SD_DETECT_PIN, INPUT_PULLUP);
    pinMode(SETUP_SW_PIN, INPUT_PULLUP);

    for (auto i=0; i < NUM_OF_MOTOR; i++)
    {
        pinMode(limitSwPin[i], INPUT_PULLUP);
    }

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

    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++)
    {
        busy[i] = false;
        flag[i] = false;
        HiZ[i] = false;
        homeSwState[i] = false;
        dir[i] = false;
        uvloStatus[i] = false;
        motorStatus[i] = 0;
        thermalStatus[i] = 0;

        reportBUSY[i] = false;
        reportFLAG[i] = false;
        reportHiZ[i] = false;
        reportHomeSwStatus[i] = false;
        reportDir[i] = false;
        reportMotorStatus[i] = false;
        reportSwEvn[i] = false;
        reportCommandError[i] = false;
        reportUVLO[i] = false;
        reportThermalStatus[i] = false;
        reportOCD[i] = false;
        reportStall[i] = false;

        limitSwState[i] = false;
        reportLimitSwStatus[i] = false;

        targetPosition[i] = 0;
        kP[i] = 0.06f;
        kI[i] = 0.0f;
        kD[i] = 0.0f;
        isServoMode[i] = false;

        isCurrentMode[i] = false;

        stepper[i].SPIPortConnect(&powerStepSPI);
        resetMotorDriver(i + MOTOR_ID_FIRST);
        digitalWrite(ledPin, HIGH);
        delay(5);
        digitalWrite(ledPin, LOW);
        delay(5);
    }

    // Configure W5500
    digitalWrite(W5500_RESET_PIN, HIGH);
    myId = getMyId();
    delay(1);
    resetEthernet();
}

uint8_t getMyId() {
    uint8_t _id = 0;
    for (auto i = 0; i < 8; ++i)
    {
        pinMode(dipSwPin[i], INPUT_PULLUP);
        _id |= (!digitalRead(dipSwPin[i])) << i;
    }
    return _id;
}

void resetEthernet() {
    digitalWrite(W5500_RESET_PIN, LOW);
    digitalWrite(ledPin, HIGH);
    delay(10); // This delay is necessary to refresh the network connection.
    digitalWrite(W5500_RESET_PIN, HIGH);
    digitalWrite(ledPin, LOW);
    delay(1);
    mac[5] = IP_OFFSET + myId;
    myIp[3] = IP_OFFSET + myId;
    outPort = 50000 + IP_OFFSET + myId;
    Ethernet.begin(mac, myIp);
    Udp.begin(inPort);
}

void resetMotorDriver(uint8_t deviceID) {
    if (MOTOR_ID_FIRST <= deviceID && deviceID <= MOTOR_ID_LAST) {
        deviceID -= MOTOR_ID_FIRST;
        stepper[deviceID].hardHiZ();
        stepper[deviceID].configStepMode(STEP_FS_128);
        stepper[deviceID].setMaxSpeed(650.);
        stepper[deviceID].setLoSpdOpt(true);
        stepper[deviceID].setMinSpeed(20.);
        stepper[deviceID].setFullSpeed(2000.);
        stepper[deviceID].setAcc(2000.);
        stepper[deviceID].setDec(2000.);
        stepper[deviceID].setSlewRate(SR_980V_us);
        stepper[deviceID].setOCThreshold(15); // 5A for 0.1ohm shunt resistor
        stepper[deviceID].setOCShutdown(OC_SD_ENABLE);
        stepper[deviceID].setPWMFreq(PWM_DIV_1, PWM_MUL_0_75);
        stepper[deviceID].setVoltageComp(VS_COMP_DISABLE);
        stepper[deviceID].setSwitchMode(SW_USER);
        stepper[deviceID].setOscMode(EXT_16MHZ_OSCOUT_INVERT);
        stepper[deviceID].setRunKVAL(16);
        stepper[deviceID].setAccKVAL(16);
        stepper[deviceID].setDecKVAL(16);
        stepper[deviceID].setHoldKVAL(0);
        stepper[deviceID].setParam(STALL_TH, 0x1F);
        stepper[deviceID].setParam(ALARM_EN, 0xEF); // Enable alarms except ADC UVLO
        kvalHold[deviceID] = tvalHold[deviceID] = stepper[deviceID].getHoldKVAL();
        kvalRun[deviceID] = tvalRun[deviceID] = stepper[deviceID].getRunKVAL();
        kvalAcc[deviceID] = tvalAcc[deviceID] = stepper[deviceID].getAccKVAL();
        kvalDec[deviceID] = tvalDec[deviceID] = stepper[deviceID].getDecKVAL();

        delay(1);
        stepper[deviceID].getStatus(); // clears error flags
    }
}


void turnOnRXL() {
    digitalWrite(PIN_LED_RXL, LOW); // turn on
    RXL_blinkStartTime = millis();
    rxLedEnabled = true;
}

void turnOnTXL() {
    digitalWrite(PIN_LED_TXL, LOW); // turn on
    TXL_blinkStartTime = millis();
    txLedEnabled = true;
}


void sendOneInt(char* address, int32_t data) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add((int32_t)data);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void sendTwoInt(char* address, int32_t data1, int32_t data2) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add(data1).add(data2);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void sendIdFloat(char* address, int32_t id, float data) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add(id).add(data);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void sendOneString(char* address, const char* data) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add(data);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

#pragma region config_commands_osc_listener
void setDestIp(OSCMessage& msg, int addrOffset) {
    bool bIpUpdated = false;
    OSCMessage newMes("/destIp");
    for (auto i = 0; i < 4; i++)
    {
        bIpUpdated |= (destIp[i] != Udp.remoteIP()[i]);
        newMes.add(Udp.remoteIP()[i]);
    }
    destIp = Udp.remoteIP();
    newMes.add(bIpUpdated);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    isDestIpSet = true;
    turnOnTXL();
}

void getVersion(OSCMessage& msg, int addrOffset) {
    String version = COMPILE_DATE;
    version += String(" ") + String(COMPILE_TIME) + String(" ") + String(PROJECT_NAME);
    sendOneString("/version", version.c_str());
}

// reset the motor driver chip and setup it
void resetMotorDriver(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        resetMotorDriver(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            resetMotorDriver(i);
        }
    }
}

// simply send reset command to the driverchip via SPI
void resetDev(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].resetDev();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].resetDev();
        }
    }
}

void enableBusyReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportBUSY[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportBUSY[i] = bEnable;
        }
    }
}

void enableHizReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportHiZ[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportHiZ[i] = bEnable;
        }
    }
}
void enableHomeSwReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportHomeSwStatus[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportHomeSwStatus[i] = bEnable;
        }
    }
}
void enableLimitSwReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportLimitSwStatus[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportLimitSwStatus[i] = bEnable;
        }
    }
}
void enableDirReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportDir[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportDir[i] = bEnable;
        }
    }
}
void enableMotorStatusReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportMotorStatus[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportMotorStatus[i] = bEnable;
        }
    }
}
void enableSwEventReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportSwEvn[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportSwEvn[i] = bEnable;
        }
    }
}
void enableCommandErrorReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportCommandError[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportCommandError[i] = bEnable;
        }
    }
}
void enableUvloReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportUVLO[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportUVLO[i] = bEnable;
        }
    }
}
void enableThermalStatusReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportThermalStatus[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportThermalStatus[i] = bEnable;
        }
    }
}
void enableOverCurrentReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportOCD[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportOCD[i] = bEnable;
        }
    }
}
void enableStallReport(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        reportStall[motorID - 1] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            reportStall[i] = bEnable;
        }
    }
}

void getHomeSw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getHomeSw(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getHomeSw(i + 1);
        }
    }
}
void getHomeSw(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/homeSw");
    newMes.add(motorID).add(homeSwState[motorID - MOTOR_ID_FIRST]).add(dir[motorID - MOTOR_ID_FIRST]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
void getLimitSw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getLimitSw(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getLimitSw(i + 1);
        }
    }
}
void getLimitSw(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/limitSw");
    newMes.add(motorID).add(limitSwState[motorID - MOTOR_ID_FIRST]).add(dir[motorID - MOTOR_ID_FIRST]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void getBusy(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/busy", motorID, busy[motorID - MOTOR_ID_FIRST]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/busy", i + MOTOR_ID_FIRST, busy[i]);
        }
    }
}
void getUvlo(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/uvlo", motorID, uvloStatus[motorID - MOTOR_ID_FIRST]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/uvlo", i + 1, uvloStatus[i]);
        }
    }
}

void getMotorStatus(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/motorStatus", motorID, motorStatus[motorID - MOTOR_ID_FIRST]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/motorStatus", i + 1, motorStatus[i]);
        }
    }
}

void getThermalStatus(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/thermalStatus", motorID, thermalStatus[motorID - MOTOR_ID_FIRST]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/thermalStatus", i + 1, thermalStatus[i]);
        }
    }
}

void getStatus(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/status", motorID, stepper[motorID - MOTOR_ID_FIRST].getStatus());
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/status", i + MOTOR_ID_FIRST, stepper[i].getStatus());
        }
    }
}

void getStatusList(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/statusList");
    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
        newMes.add((int32_t)stepper[i].getStatus());
    }

    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void setMicrostepMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t microstepMode = constrain(msg.getInt(1), STEP_FS, STEP_FS_128); // 0-7
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].configStepMode(microstepMode);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].configStepMode(microstepMode);
        }
    }
}

void getMicrostepMode(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/microstepMode", motorID, stepper[motorID - MOTOR_ID_FIRST].getStepMode());
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/microstepMode", i + MOTOR_ID_FIRST, stepper[i].getStepMode());
        }
    }
}

void getHomeSwMode(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/homeSwMode", motorID, stepper[motorID - MOTOR_ID_FIRST].getSwitchMode() > 0);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/homeSwMode", i + MOTOR_ID_FIRST, stepper[i].getSwitchMode() > 0);
        }
    }
}

void setHomeSwMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t switchMode = (msg.getInt(1) > 0) ? SW_USER : SW_HARD_STOP;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setSwitchMode(switchMode);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setSwitchMode(switchMode);
        }
    }
}
void getLimitSwMode(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/limitSwMode", motorID, limitSwMode[motorID - MOTOR_ID_FIRST]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/limitSwMode", i + MOTOR_ID_FIRST, limitSwMode[i] > 0);
        }
    }
}

void setLimitSwMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t switchMode = (msg.getInt(1) > 0) ? SW_USER : SW_HARD_STOP;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        limitSwMode[motorID - MOTOR_ID_FIRST] = switchMode > 0;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            limitSwMode[i] = switchMode > 0;
        }
    }
}
// STALL_TH register is 5bit in PowerSTEP01, 7bit in L6470
void setStallThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t threshold = msg.getInt(1) & 0x1F; // 5bit
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setParam(STALL_TH, threshold);
        getStallThreshold(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setParam(STALL_TH, threshold);
            getStallThreshold(i + 1);
        }
    }

}
void getStallThreshold(uint8_t motorId) {
    if (!isDestIpSet) { return; }
    uint8_t stall_th_raw = stepper[motorId - MOTOR_ID_FIRST].getParam(STALL_TH) & 0x1F;
    float threshold = (stall_th_raw + 1) * 312.5;
    sendIdFloat("/stallThreshold", motorId, threshold);
}
void getStallThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getStallThreshold(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getStallThreshold(i + MOTOR_ID_FIRST);
        }
    }
}
// OCD_TH register is 5bit in PowerSTEP01, 4bit in L6470
void setOverCurrentThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t threshold = msg.getInt(1) & 0x1F; // 5bit
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setParam(OCD_TH, threshold);
        getOverCurrentThreshold(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setParam(OCD_TH, threshold);
            getOverCurrentThreshold(i + 1);
        }
    }
}
void getOverCurrentThreshold(uint8_t motorId) {
    if (!isDestIpSet) { return; }
    uint8_t ocd_th_raw = stepper[motorId - MOTOR_ID_FIRST].getParam(OCD_TH) & 0x1F;
    float threshold = (ocd_th_raw + 1) * 312.5;
    sendIdFloat("/overCurrentThreshold", motorId, threshold);
}
void getOverCurrentThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getOverCurrentThreshold(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getOverCurrentThreshold(i + MOTOR_ID_FIRST);
        }
    }
}

void setLowSpeedOptimizeThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setMinSpeed(stepsPerSecond);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMinSpeed(stepsPerSecond);
        }
    }
}
void getLowSpeedOptimizeThreshold(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getLowSpeedOptimizeThreshold(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getLowSpeedOptimizeThreshold(i + 1);
        }
    }
}
void getLowSpeedOptimizeThreshold(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    bool optimizationEnabled = (stepper[motorID - MOTOR_ID_FIRST].getParam(MIN_SPEED) & (1 << 12)) > 0;
    OSCMessage newMes("/lowSpeedOptimizeThreshold");
    newMes.add((int32_t)motorID);
    newMes.add(stepper[motorID - MOTOR_ID_FIRST].getMinSpeed());
    newMes.add(optimizationEnabled);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
#pragma endregion config_commands_osc_listener

#pragma region kval_commands_osc_listener
void setKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int hold = constrain(msg.getInt(1), 0, 255);
    int run = constrain(msg.getInt(2), 0, 255);
    int acc = constrain(msg.getInt(3), 0, 255);
    int dec = constrain(msg.getInt(4), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (!isCurrentMode[motorID]) {
            stepper[motorID].setHoldKVAL(hold);
            stepper[motorID].setRunKVAL(run);
            stepper[motorID].setAccKVAL(acc);
            stepper[motorID].setDecKVAL(dec);
        }
        kvalHold[motorID] = hold;
        kvalRun[motorID] = run;
        kvalAcc[motorID] = acc;
        kvalDec[motorID] = dec;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (!isCurrentMode[i]) {
                stepper[i].setHoldKVAL(hold);
                stepper[i].setRunKVAL(run);
                stepper[i].setAccKVAL(acc);
                stepper[i].setDecKVAL(dec);
            }
            kvalHold[i] = hold;
            kvalRun[i] = run;
            kvalAcc[i] = acc;
            kvalDec[i] = dec;
        }
    }
}

void setHoldKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        if (!isCurrentMode[motorID]) {
            stepper[motorID - MOTOR_ID_FIRST].setHoldKVAL(kvalInput);
        }
        kvalHold[motorID - MOTOR_ID_FIRST] = kvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (!isCurrentMode[i]) {
                stepper[i].setHoldKVAL(kvalInput);
            }
            kvalHold[i] = kvalInput;
        }
    }
}
void setRunKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        if (!isCurrentMode[motorID]) {
            stepper[motorID - MOTOR_ID_FIRST].setRunKVAL(kvalInput);
        }
        kvalRun[motorID - MOTOR_ID_FIRST] = kvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (!isCurrentMode[i]) {
                stepper[i].setRunKVAL(kvalInput);
            }
            kvalRun[i] = kvalInput;
        }
    }
}
void setAccKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        if (!isCurrentMode[motorID]) {
            stepper[motorID - MOTOR_ID_FIRST].setAccKVAL(kvalInput);
        }
        kvalAcc[motorID - MOTOR_ID_FIRST] = kvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (!isCurrentMode[i]) {
                stepper[i].setAccKVAL(kvalInput);
            }
            kvalAcc[i] = kvalInput;
        }
    }
}
void setDecKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t kvalInput = constrain(msg.getInt(1), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        if (!isCurrentMode[motorID]) {
            stepper[motorID - MOTOR_ID_FIRST].setDecKVAL(kvalInput);
        }
        kvalDec[motorID - MOTOR_ID_FIRST] = kvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (!isCurrentMode[i]) {
                stepper[i].setDecKVAL(kvalInput);
            }
            kvalDec[i] = kvalInput;
        }
    }
}

void getKval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getKval(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getKval(i + MOTOR_ID_FIRST);
        }
    }
}
void getKval(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/kval");
    newMes.add((int32_t)motorID);
    motorID -= MOTOR_ID_FIRST;
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getHoldKval());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getRunKval());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getAccKval());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getDecKval());
    newMes.add(kvalHold[motorID]).add(kvalRun[motorID]).add(kvalAcc[motorID]).add(kvalDec[motorID]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
#pragma endregion kval_commands_osc_listener

#pragma region tval_commands_osc_listener
void setTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int hold = constrain(msg.getInt(1), 0, TVAL_LIMIT_VAL);
    int run = constrain(msg.getInt(2), 0, TVAL_LIMIT_VAL);
    int acc = constrain(msg.getInt(3), 0, TVAL_LIMIT_VAL);
    int dec = constrain(msg.getInt(4), 0, TVAL_LIMIT_VAL);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (isCurrentMode[motorID]) {
            stepper[motorID].setHoldTVAL(hold);
            stepper[motorID].setRunTVAL(run);
            stepper[motorID].setAccTVAL(acc);
            stepper[motorID].setDecTVAL(dec);
        }
        tvalHold[motorID] = hold;
        tvalRun[motorID] = run;
        tvalAcc[motorID] = acc;
        tvalDec[motorID] = dec;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isCurrentMode[i]) {
                stepper[i].setHoldTVAL(hold);
                stepper[i].setRunTVAL(run);
                stepper[i].setAccTVAL(acc);
                stepper[i].setDecTVAL(dec);
            }
            tvalHold[i] = hold;
            tvalRun[i] = run;
            tvalAcc[i] = acc;
            tvalDec[i] = dec;
        }
    }
}

void setHoldTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, TVAL_LIMIT_VAL);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (isCurrentMode[motorID]) {
            stepper[motorID].setHoldTVAL(tvalInput);
        }
        tvalHold[motorID] = tvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isCurrentMode[i]) {
                stepper[i].setHoldTVAL(tvalInput);
            }
            tvalHold[i] = tvalInput;
        }
    }
}
void setRunTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, TVAL_LIMIT_VAL);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (isCurrentMode[motorID]) {
            stepper[motorID].setRunTVAL(tvalInput);
        }
        tvalRun[motorID] = tvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isCurrentMode[i]) {
                stepper[i].setRunTVAL(tvalInput);
            }
            tvalRun[i] = tvalInput;
        }
        
    }
}
void setAccTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, TVAL_LIMIT_VAL);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (isCurrentMode[motorID]) {
            stepper[motorID].setAccTVAL(tvalInput);
        }
        tvalAcc[motorID] = tvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isCurrentMode[i]) {
                stepper[i].setAccTVAL(tvalInput);
            }
            tvalAcc[i] = tvalInput;
        }       
    }
}
void setDecTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t tvalInput = constrain(msg.getInt(1), 0, TVAL_LIMIT_VAL);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        if (isCurrentMode[motorID]) {
            stepper[motorID].setDecTVAL(tvalInput);
        }
        tvalDec[motorID] = tvalInput;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isCurrentMode[i]) {
                stepper[i].setDecTVAL(tvalInput);
            }
            tvalDec[i] = tvalInput;
        }
    }
}

void getTval(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getTval(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getTval(i + MOTOR_ID_FIRST);
        }
    }
}
void getTval(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/tval");
    newMes.add((int32_t)motorID);
    motorID -= MOTOR_ID_FIRST;
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getHoldTVAL());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getRunTVAL());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getAccTVAL());
    //newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getDecTVAL());
    newMes.add(tvalHold[motorID]).add(tvalRun[motorID]).add(tvalAcc[motorID]).add(tvalDec[motorID]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
float TvalToCurrent(uint8_t tval) {
    return (tval + 1) * 7.8f;
}
void getTval_mA(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/tval_mA");
    newMes.add((int32_t)motorID);
    motorID -= MOTOR_ID_FIRST;
    newMes.add(TvalToCurrent(tvalHold[motorID]));
    newMes.add(TvalToCurrent(tvalRun[motorID]));
    newMes.add(TvalToCurrent(tvalAcc[motorID]));
    newMes.add(TvalToCurrent(tvalDec[motorID]));
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
void getTval_mA(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getTval_mA(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getTval_mA(i + 1);
        }
    }
}
#pragma endregion tval_commands_osc_listener

#pragma region speed_commands_osc_listener

void setSpeedProfile(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float acc = msg.getFloat(1);
    float dec = msg.getFloat(2);
    float maxSpeed = msg.getFloat(3);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setAcc(acc);
        stepper[motorID - MOTOR_ID_FIRST].setDec(dec);
        stepper[motorID - MOTOR_ID_FIRST].setMaxSpeed(maxSpeed);

    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAcc(acc);
            stepper[i].setDec(dec);
            stepper[i].setMaxSpeed(maxSpeed);
        }
    }
}

void setMaxSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setMaxSpeed(stepsPerSecond);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMaxSpeed(stepsPerSecond);
        }
    }
}
// MIN_SPEED register is set by setLowSpeedOptimizeThreshold function.

void setFullstepSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float stepsPerSecond = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setFullSpeed(stepsPerSecond);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setFullSpeed(stepsPerSecond);
        }
    }
}
void setAcc(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float stepsPerSecondPerSecond = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setAcc(stepsPerSecondPerSecond);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAcc(stepsPerSecondPerSecond);
        }
    }
}
void setDec(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float stepsPerSecondPerSecond = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setDec(stepsPerSecondPerSecond);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setDec(stepsPerSecondPerSecond);
        }
    }
}

void setSpeedProfileRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t acc = msg.getInt(1);
    uint16_t dec = msg.getInt(2);
    uint16_t maxSpeed = msg.getInt(3);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setAccRaw(acc);
        stepper[motorID - MOTOR_ID_FIRST].setDecRaw(dec);
        stepper[motorID - MOTOR_ID_FIRST].setMaxSpeedRaw(maxSpeed);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAccRaw(acc);
            stepper[i].setDecRaw(dec);
            stepper[i].setMaxSpeedRaw(maxSpeed);
        }
    }
}

void setMaxSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t t = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setMaxSpeedRaw(t);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMaxSpeedRaw(t);
        }
    }
}
void setMinSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t t = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setMinSpeedRaw(t);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMinSpeedRaw(t);
        }
    }
}

void setFullstepSpeedRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t t = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setFullSpeedRaw(t);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setFullSpeedRaw(t);
        }
    }
}

void setAccRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t t = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setAccRaw(t);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAccRaw(t);
        }
    }
}
void setDecRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t t = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setDecRaw(t);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setDecRaw(t);
        }
    }
}

void getSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float s;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        s = stepper[motorID - MOTOR_ID_FIRST].getSpeed();
        if (dir[motorID - MOTOR_ID_FIRST] == REV) { s *= -1.0; }
        sendIdFloat("/speed", motorID, s);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            s = stepper[i].getSpeed();
            if (dir[i] == REV) { s *= -1.0; }
            sendIdFloat("/speed", i + 1, s);
        }
    }
}

void getSpeedProfile(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getSpeedProfile(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getSpeedProfile(i);
        }
    }
}
void getSpeedProfile(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/speedProfile");
    newMes.add((int32_t)motorID);
    newMes.add((float)stepper[motorID - MOTOR_ID_FIRST].getAcc());
    newMes.add((float)stepper[motorID - MOTOR_ID_FIRST].getDec());
    newMes.add((float)stepper[motorID - MOTOR_ID_FIRST].getMaxSpeed());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void getSpeedProfileRaw(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getSpeedProfileRaw(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getSpeedProfileRaw(i);
        }
    }
}
void getSpeedProfileRaw(uint8_t motorID) {
    OSCMessage newMes("/speedProfileRaw");
    newMes.add((int32_t)motorID);
    newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getAccRaw());
    newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getDecRaw());
    newMes.add((int32_t)stepper[motorID - MOTOR_ID_FIRST].getMaxSpeedRaw());
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}
#pragma endregion speed_commands_osc_listener

#pragma region operational_commands_osc_listener

void getPosition(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/position", motorID, stepper[motorID - MOTOR_ID_FIRST].getPos());
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/position", i + MOTOR_ID_FIRST, stepper[i].getPos());
        }
    }
}
void getMark(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        sendTwoInt("/mark", motorID, stepper[motorID - MOTOR_ID_FIRST].getMark());
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            sendTwoInt("/mark", i + MOTOR_ID_FIRST, stepper[i].getMark());
        }
    }
}

void run(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);

    float stepsPerSec = 0;
    if (msg.isFloat(1)) { stepsPerSec = msg.getFloat(1); }
    else if (msg.isInt(1)) { stepsPerSec = (float)msg.getInt(1); }
    boolean dir = stepsPerSec > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].run(dir, abs(stepsPerSec));
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].run(dir, abs(stepsPerSec));
        }
    }
}

void runRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);

    int32_t speed = msg.getInt(1);
    boolean dir = speed > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].runRaw(dir, abs(speed));
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].runRaw(dir, abs(speed));
        }
    }
}

void move(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int32_t steps = msg.getInt(1);
    boolean dir = steps > 0;
    steps = abs(steps);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].move(dir, steps);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].move(dir, steps);
        }
    }
}
void goTo(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int32_t pos = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goTo(pos);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goTo(pos);
        }
    }
}
void goToDir(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);

    boolean dir = msg.getInt(1) > 0;
    int32_t pos = msg.getInt(2);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goToDir(dir, pos);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goToDir(dir, pos);
        }
    }
}

void goUntil(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool action = msg.getInt(1)>0;
    float stepsPerSec = 0;
    if (msg.isFloat(2)) { stepsPerSec = msg.getFloat(2); }
    else if (msg.isInt(2)) { stepsPerSec = (float)msg.getInt(2); }
    bool dir = stepsPerSec > 0;
    stepsPerSec = abs(stepsPerSec);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goUntil(action, dir, stepsPerSec);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goUntil(action, dir, stepsPerSec);
        }
    }
}
void goUntilRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool action = msg.getInt(1) > 0;
    int32_t speed = msg.getInt(2);
    bool dir = speed > 0;
    speed = abs(speed);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goUntilRaw(action, dir, speed);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goUntilRaw(action, dir, speed);
        }
    }
}

void releaseSw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);

    uint8_t action = msg.getInt(1);
    uint8_t dir = constrain(msg.getInt(2), 0, 1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].releaseSw(action, dir);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].releaseSw(action, dir);
        }
    }
}
void goHome(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goHome();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goHome();
        }
    }
}
void goMark(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].goMark();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].goMark();
        }
    }
}
void setMark(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int32_t newMark = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setMark(newMark);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMark(newMark);
        }
    }
}
void setPosition(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int32_t newPos = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setPos(newPos);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setPos(newPos);
        }
    }
}
void resetPos(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].resetPos();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].resetPos();
        }
    }
}
void softStop(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        isServoMode[motorID - MOTOR_ID_FIRST] = false;
        stepper[motorID - MOTOR_ID_FIRST].softStop();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            isServoMode[i] = false;
            stepper[i].softStop();
        }
    }
}
void hardStop(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        isServoMode[motorID - MOTOR_ID_FIRST] = false;
        stepper[motorID - MOTOR_ID_FIRST].hardStop();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            isServoMode[i] = false;
            stepper[i].hardStop();
        }
    }
}
void softHiZ(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        isServoMode[motorID - MOTOR_ID_FIRST] = false;
        stepper[motorID - MOTOR_ID_FIRST].softHiZ();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            isServoMode[i] = false;
            stepper[i].softHiZ();
        }
    }
}
void hardHiZ(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        isServoMode[motorID - MOTOR_ID_FIRST] = false;
        stepper[motorID - MOTOR_ID_FIRST].hardHiZ();
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            isServoMode[i] = false;
            stepper[i].hardHiZ();
        }
    }
}
#pragma endregion operational_commands_osc_listener

#pragma region servo_commands_osc_listener

void setTargetPosition(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    int32_t position = msg.getInt(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        targetPosition[motorID - MOTOR_ID_FIRST] = position;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            targetPosition[i] = position;
        }
    }
}

void setTargetPositionList(OSCMessage& msg, int addrOffset) {
    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
        targetPosition[i] = msg.getInt(i);
    }
}

void enableServoMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    bool bEnable = msg.getInt(1) > 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        if (bEnable) {
            targetPosition[motorID - MOTOR_ID_FIRST] = stepper[motorID - MOTOR_ID_FIRST].getPos();
            reportBUSY[motorID - MOTOR_ID_FIRST] = false;
            reportMotorStatus[motorID - MOTOR_ID_FIRST] = false;
            reportDir[motorID - MOTOR_ID_FIRST] = false;
            stepper[motorID - MOTOR_ID_FIRST].hardStop();
        }
        else {
            stepper[motorID - MOTOR_ID_FIRST].softStop();
        }
        isServoMode[motorID - MOTOR_ID_FIRST] = bEnable;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (bEnable) {
                targetPosition[i] = stepper[i].getPos();
                reportBUSY[i] = false;
                reportMotorStatus[i] = false;
                reportDir[i] = false;
                stepper[i].hardStop();
            }
            else {
                stepper[i].softStop();
            }
            isServoMode[i] = bEnable;
        }
    }
}

void setServoParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _kp = msg.getFloat(1), _ki = msg.getFloat(2), _kd = msg.getFloat(3);
    if (_kp <= 0.0) _kp = 0;
    if (_ki <= 0.0) _ki = 0;
    if (_kd <= 0.0) _kd = 0;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        kP[motorID - MOTOR_ID_FIRST] = _kp;
        kI[motorID - MOTOR_ID_FIRST] = _ki;
        kD[motorID - MOTOR_ID_FIRST] = _kd;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            kP[i] = _kp;
            kI[i] = _ki;
            kD[i] = _kd;
        }
    }
}

void getServoParam(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/servoParam");
    newMes.add(motorID).add(kP[motorID - 1]).add(kI[motorID - 1]).add(kD[motorID - 1]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void getServoParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getServoParam(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getServoParam(i + MOTOR_ID_FIRST);
        }
    }
}

#pragma endregion servo_commands_osc_listener


#pragma region PowerSTEP01_config_osc_listener

void setVoltageMode(uint8_t motorID) {
    motorID -= MOTOR_ID_FIRST;
    stepper[motorID].hardHiZ();
    stepper[motorID].setPWMFreq(PWM_DIV_1, PWM_MUL_0_75);
    stepper[motorID].setHoldKVAL(kvalHold[motorID]);
    stepper[motorID].setRunKVAL(kvalRun[motorID]);
    stepper[motorID].setAccKVAL(kvalAcc[motorID]);
    stepper[motorID].setDecKVAL(kvalDec[motorID]);
    stepper[motorID].setVoltageMode();
    isCurrentMode[motorID] = false;
}
void setVoltageMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        setVoltageMode(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            setVoltageMode(i + MOTOR_ID_FIRST);
        }
    }
}

void setCurrentMode(uint8_t motorID) {
    motorID -= MOTOR_ID_FIRST;
    stepper[motorID].hardHiZ();
    stepper[motorID].setPredictiveControl(CONFIG_PRED_ENABLE);
    stepper[motorID].setSwitchingPeriod(5);
    if ( stepper[motorID].getStepMode() > STEP_SEL_1_16 )
    {
        stepper[motorID].configStepMode(STEP_SEL_1_16);
    }
    stepper[motorID].setHoldTVAL(tvalHold[motorID]);
    stepper[motorID].setRunTVAL(tvalRun[motorID]);
    stepper[motorID].setAccTVAL(tvalAcc[motorID]);
    stepper[motorID].setDecTVAL(tvalDec[motorID]);
    stepper[motorID].setCurrentMode();
    isCurrentMode[motorID] = true;
}
void setCurrentMode(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        setCurrentMode(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            setCurrentMode(i + MOTOR_ID_FIRST);
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
            // some possible frequent messeages
            msgIN.route("/setTargetPosition", setTargetPosition);
            msgIN.route("/setTargetPositionList", setTargetPositionList);
            msgIN.route("/getPosition", getPosition);
            msgIN.route("/getSpeed", getSpeed);
            msgIN.route("/run", run);
            msgIN.route("/runRaw", runRaw);

            // motion
            msgIN.route("/move", move);
            msgIN.route("/goTo", goTo);
            msgIN.route("/goToDir", goToDir);
            msgIN.route("/goUntil", goUntil);
            msgIN.route("/goUntilRaw", goUntilRaw);
            msgIN.route("/releaseSw", releaseSw);
            msgIN.route("/goHome", goHome);
            msgIN.route("/goMark", goMark);
            msgIN.route("/setMark", setMark);
            msgIN.route("/getMark", getMark);
            msgIN.route("/setPosition", setPosition);
            msgIN.route("/resetPos", resetPos);
            msgIN.route("/resetDev", resetDev);
            msgIN.route("/softStop", softStop);
            msgIN.route("/hardStop", hardStop);
            msgIN.route("/softHiZ", softHiZ);
            msgIN.route("/hardHiZ", hardHiZ);

            // servo mode
            msgIN.route("/enableServoMode", enableServoMode);
            msgIN.route("/setServoParam", setServoParam);
            msgIN.route("/getServoParam", getServoParam);

            // speed
            msgIN.route("/setSpeedProfile", setSpeedProfile);
            msgIN.route("/setMaxSpeed", setMaxSpeed);
            msgIN.route("/setFullstepSpeed", setFullstepSpeed);
            msgIN.route("/setAcc", setAcc);
            msgIN.route("/setDec", setDec);
            msgIN.route("/getSpeedProfile", getSpeedProfile);

            // Kval
            msgIN.route("/setKval", setKval);
            msgIN.route("/setAccKval", setAccKval);
            msgIN.route("/setDecKval", setDecKval);
            msgIN.route("/setRunKval", setRunKval);
            msgIN.route("/setHoldKval", setHoldKval);
            msgIN.route("/getKval", getKval);

            //TVAL
            msgIN.route("/setTval", setTval);
            msgIN.route("/setAccTval", setAccTval);
            msgIN.route("/setDecTval", setDecTval);
            msgIN.route("/setRunTval", setRunTval);
            msgIN.route("/setHoldTval", setHoldTval);
            msgIN.route("/getTval", getTval);
            msgIN.route("/getTval_mA", getTval_mA);

            // config
            msgIN.route("/setDestIp", setDestIp);
            msgIN.route("/getVersion", getVersion);
            msgIN.route("/getStatus", getStatus);
            msgIN.route("/getStatusList", getStatusList);
            msgIN.route("/getHomeSw", getHomeSw);
            msgIN.route("/getBusy", getBusy);
            msgIN.route("/getUvlo", getUvlo);
            msgIN.route("/getMotorStatus", getMotorStatus);
            msgIN.route("/getThermalStatus", getThermalStatus);
            msgIN.route("/resetMotorDriver", resetMotorDriver);
            //msgIN.route("/enableFlagReport", enableFlagReport);
            msgIN.route("/enableBusyReport", enableBusyReport);
            msgIN.route("/enableHizReport", enableHizReport);
            msgIN.route("/enableHomeSwReport", enableHomeSwReport);
            msgIN.route("/enableDirReport", enableDirReport);
            msgIN.route("/enableMotorStatusReport", enableMotorStatusReport);
            msgIN.route("/enableSwEventReport", enableSwEventReport);
            msgIN.route("/enableCommandErrorReport", enableCommandErrorReport);
            msgIN.route("/enableUvloReport", enableUvloReport);
            msgIN.route("/enableThermalStatusReport", enableThermalStatusReport);
            msgIN.route("/enableOverCurrentReport", enableOverCurrentReport);
            msgIN.route("/enableStallReport", enableStallReport);
            //msgIN.route("/getDir", getDir);
            msgIN.route("/getLimitSw",getLimitSw);
            msgIN.route("/getLimitSwMode", getLimitSwMode);
            msgIN.route("/setLimitSwMode", setLimitSwMode);
            msgIN.route("/enableLimitSwReport", enableLimitSwReport);

            msgIN.route("/setMicrostepMode", setMicrostepMode);
            msgIN.route("/getMicrostepMode", getMicrostepMode);
            msgIN.route("/getHomeSwMode", getHomeSwMode);
            msgIN.route("/setHomeSwMode", setHomeSwMode);
            msgIN.route("/setStallThreshold", setStallThreshold);
            msgIN.route("/getStallThreshold", getStallThreshold);
            msgIN.route("/setOverCurrentThreshold", setOverCurrentThreshold);
            msgIN.route("/getOverCurrentThreshold", getOverCurrentThreshold);
            msgIN.route("/setLowSpeedOptimizeThreshold", setLowSpeedOptimizeThreshold);
            msgIN.route("/getLowSpeedOptimizeThreshold", getLowSpeedOptimizeThreshold);


            msgIN.route("/setSpeedProfileRaw",setSpeedProfileRaw);
            msgIN.route("/setMaxSpeedRaw", setMaxSpeedRaw);
            msgIN.route("/setMinSpeedRaw", setMinSpeedRaw);
            msgIN.route("/setFullstepSpeedRaw", setFullstepSpeedRaw);
            msgIN.route("/setAccRaw", setAccRaw);
            msgIN.route("/setDecRaw", setDecRaw);
            msgIN.route("/getSpeedProfileRaw",getSpeedProfileRaw);

            msgIN.route("/setVoltageMode", setVoltageMode);
            msgIN.route("/setCurrentMode", setCurrentMode);
            turnOnRXL();
        }
    }
}
void sendStatusDebug(char* address, int32_t data1, int32_t data2, int32_t data3){
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add(data1).add(data2).add(data3);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
}

void checkStatus() {
    uint32_t t;
    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++)
    {
        const auto status = stepper[i].getStatus();
        // HiZ, high for HiZ
        t = (status & STATUS_HIZ) > 0;
        if (HiZ[i] != t)
        {
            HiZ[i] = t;
            if (reportHiZ[i]) sendTwoInt("/HiZ", i + MOTOR_ID_FIRST, t);
        }
        // BUSY, low for busy
        t = (status & STATUS_BUSY) == 0;
        if (busy[i] != t)
        {
        	busy[i] = t;
        	if ( reportBUSY[i] ) sendTwoInt("/busy", i + MOTOR_ID_FIRST, t);
        }
        // DIR
        t = (status & STATUS_DIR) > 0;
        if (dir[i] != t)
        {
            dir[i] = t;
            if (reportDir[i]) sendTwoInt("/dir", i + MOTOR_ID_FIRST, t);
        }
        // SW_F, low for open, high for close
        t = (status & STATUS_SW_F) > 0;
        if (homeSwState[i] != t)
        {
            homeSwState[i] = t;
            if (reportHomeSwStatus[i]) getHomeSw(i + 1);
        }
        // SW_EVN, active high, latched
        t = (status & STATUS_SW_EVN) > 0;
        if (t && reportSwEvn[i]) sendOneInt("/swEvent", i + MOTOR_ID_FIRST);
        // MOT_STATUS
        t = (status & STATUS_MOT_STATUS) >> 5;
        if (motorStatus[i] != t) {
            motorStatus[i] = t;
            if (reportMotorStatus[i]) sendTwoInt("/motorStatus", i + MOTOR_ID_FIRST, motorStatus[i]);
        }
        // CMD_ERROR, active high, latched
        t = (status & STATUS_CMD_ERROR) > 0;
        if (t && reportCommandError[i]) sendOneInt("commandError", i + MOTOR_ID_FIRST);
        // UVLO, active low
        t = (status & STATUS_UVLO) == 0;
        if (t != uvloStatus[i])
        {
            uvloStatus[i] = !uvloStatus[i];
            if (reportUVLO[i]) sendTwoInt("/uvlo", i + MOTOR_ID_FIRST, uvloStatus[i]);
        }
        // TH_STATUS
        t = (status & STATUS_TH_STATUS) >> 11;
        if (thermalStatus[i] != t) {
            thermalStatus[i] = t;
            if (reportThermalStatus[i]) sendTwoInt("/thermalStatus", i + MOTOR_ID_FIRST, thermalStatus[i]);
        }
        // OCD, active low, latched
        t = (status & STATUS_OCD) == 0;
        if (t && reportOCD[i]) sendOneInt("/overCurrent", i + 1);
        // STALL A&B, active low, latched
        t = (status & (STATUS_STALL_A | STATUS_STALL_B)) >> 14;
        if ((t != 3) && reportStall[i]) sendOneInt("/stall", i + MOTOR_ID_FIRST);
    }
}

void checkLimitSw() {
    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++)
    {
        if (limitSwState[i] == digitalRead(limitSwPin[i]))
        {
            limitSwState[i] = !limitSwState[i];
            if ( limitSwState[i] && (limitSwMode[i] == SW_HARD_STOP) )
            {
                stepper[i].hardStop();
            }
            if (reportLimitSwStatus[i])
            {
                sendTwoInt("/limitSw", i + MOTOR_ID_FIRST, limitSwState[i]);
            }
        }
    }
}

void checkLED(uint32_t _currentTimeMillis) {
    if (rxLedEnabled)
    {
        if ((uint32_t)(_currentTimeMillis - RXL_blinkStartTime) >= RXL_TXL_BLINK_DURATION)
        {
            rxLedEnabled = false;
            digitalWrite(PIN_LED_RXL, HIGH); // turn off
        }
    }
    if (txLedEnabled)
    {
        if ((uint32_t)(_currentTimeMillis - TXL_blinkStartTime) >= RXL_TXL_BLINK_DURATION)
        {
            txLedEnabled = false;
            digitalWrite(PIN_LED_TXL, HIGH); // turn off
        }
    }
}

void updateServo(uint32_t currentTimeMicros) {
    static float eZ1[NUM_OF_MOTOR] = { 0,0,0,0 },
        eZ2[NUM_OF_MOTOR] = { 0,0,0,0 },
        integral[NUM_OF_MOTOR] = { 0,0,0,0 };
    float spd = 0.0;
    if ((uint32_t)(currentTimeMicros - lastServoUpdateTime) >= 100) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            if (isServoMode[i]) {
                int32_t error = targetPosition[i] - stepper[i].getPos();
                integral[i] += ((error + eZ1[i]) / 2.0f);
                if (integral[i] > 1500.0) integral[i] = 1500.0;
                else if (integral[i] < -1500.0) integral[i] = -1500.0;
                if (abs(error) > position_tolerance) {
                    double diff = error - eZ1[i];

                    spd = error * kP[i] + integral[i] * kI[i] + diff * kD[i];
                }
                eZ2[i] = eZ1[i];
                eZ1[i] = error;
                float absSpd = abs(spd);
                //if (absSpd < 1.) {
                //    spd = 0.0;
                //}
                stepper[i].run((spd > 0), absSpd);
            }
        }
        lastServoUpdateTime = currentTimeMicros;
    }
}

void loop() {
    uint32_t currentTimeMillis = millis(),
        currentTimeMicros = micros();
    static uint32_t lastPollTime = 0;

    if ((uint32_t)(currentTimeMillis - lastPollTime) >= STATUS_POLL_PERIOD)
    {
        checkStatus();
        checkLimitSw();
        checkLED(currentTimeMillis);
        uint8_t t = getMyId();
        if (myId != t) {
            myId = t;
            resetEthernet();
        }
        lastPollTime = currentTimeMillis;
    }
    OSCMsgReceive();

    updateServo(currentTimeMicros);
}