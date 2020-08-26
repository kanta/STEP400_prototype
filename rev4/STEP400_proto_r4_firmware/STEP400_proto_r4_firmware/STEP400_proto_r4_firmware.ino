/*
 Name:		STEP400_proto_r4_firmware.ino

 target:    Arduino Zero
 Created:   2020/04/08 10:24:41
 Author:    kanta
*/

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function
#include <SPI.h>
#include <OSCMessage.h>
#include <Ethernet.h>
#include <Ponoor_PowerSTEP01Library.h>


#define COMPILE_DATE __DATE__
#define COMPILE_TIME __TIME__
constexpr auto PROJECT_NAME = "STEP400proto_r4";
boolean debugMode = false;

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

#define TVAL_LIMIT_VAL  64 // approx. 5A
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

#define STATUS_POLL_PERIOD   1 // [ms]

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

uint16_t intersectSpeed[NUM_OF_MOTOR]; // INT_SPEED
uint8_t 
    startSlope[NUM_OF_MOTOR], // ST_SLP
    accFinalSlope[NUM_OF_MOTOR], // FN_SLP_ACC
    decFinalSlope[NUM_OF_MOTOR]; // FN_SLP_DEC
uint8_t
    fastDecaySetting[NUM_OF_MOTOR], // T_FAST
    minOnTime[NUM_OF_MOTOR], // TON_MIN
    minOffTime[NUM_OF_MOTOR]; // TOFF_MIN

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
        reportCommandError[i] = true;
        reportUVLO[i] = true;
        reportThermalStatus[i] = true;
        reportOCD[i] = true;
        reportStall[i] = true;

        limitSwState[i] = false;
        reportLimitSwStatus[i] = false;
        limitSwMode[i] = false;

        intersectSpeed[i] = 0x0408; // INT_SPEED
        startSlope[i] = 0x19;// ST_SLP
        accFinalSlope[i] = 0x29; // FN_SLP_ACC
        decFinalSlope[i] = 0x29; // FN_SLP_DEC
        fastDecaySetting[i] = 0x19; // T_FAST
        minOnTime[i] = 0x29; // TON_MIN
        minOffTime[i] = 0x29; // TOFF_MIN

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
        stepper[deviceID].resetDev();
        stepper[deviceID].configStepMode(STEP_FS_128);
        stepper[deviceID].setMaxSpeed(650.);
        stepper[deviceID].setLoSpdOpt(true);
        stepper[deviceID].setMinSpeed(20.); // Low speed optimazation threshold
        stepper[deviceID].setFullSpeed(15610.);
        stepper[deviceID].setParam(INT_SPD, intersectSpeed[deviceID]);
        stepper[deviceID].setParam(ST_SLP, startSlope[deviceID]);
        stepper[deviceID].setParam(FN_SLP_ACC, accFinalSlope[deviceID]);
        stepper[deviceID].setParam(FN_SLP_DEC, decFinalSlope[deviceID]);
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

void sendThreeInt(char* address, int32_t data1, int32_t data2, int32_t data3) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes(address);
    newMes.add(data1).add(data2).add(data3);
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
            msgIN.route("/getFullstepSpeed", getFullstepSpeed);
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
            msgIN.route("/setBemfParam", setBemfParam);
            msgIN.route("/getBemfParam", getBemfParam);
            msgIN.route("/setDecayModeParam", setDecayModeParam);
            msgIN.route("/getDecayModeParam", getDecayModeParam);
            msgIN.route("/setDebugMode", setDebugMode);
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
            if (debugMode) sendStatusDebug("/HiZ", i + MOTOR_ID_FIRST, t, status);
        }
        // BUSY, low for busy
        t = (status & STATUS_BUSY) == 0;
        if (busy[i] != t)
        {
        	busy[i] = t;
        	if ( reportBUSY[i] ) sendTwoInt("/busy", i + MOTOR_ID_FIRST, t);
            if (debugMode) sendStatusDebug("/busy", i + MOTOR_ID_FIRST, t, status);
        }
        // DIR
        t = (status & STATUS_DIR) > 0;
        if (dir[i] != t)
        {
            dir[i] = t;
            if (reportDir[i]) sendTwoInt("/dir", i + MOTOR_ID_FIRST, t);
            if (debugMode) sendStatusDebug("/dir", i + MOTOR_ID_FIRST, t, status);
        }
        // SW_F, low for open, high for close
        t = (status & STATUS_SW_F) > 0;
        if (homeSwState[i] != t)
        {
            homeSwState[i] = t;
            if (reportHomeSwStatus[i]) getHomeSw(i + 1);
            if (debugMode) sendStatusDebug("/homeSw", i + MOTOR_ID_FIRST, t, status);
        }
        // SW_EVN, active high, latched
        t = (status & STATUS_SW_EVN) > 0;
        if (t && reportSwEvn[i]) sendOneInt("/swEvent", i + MOTOR_ID_FIRST);
        if (debugMode && t) sendStatusDebug("/swEvent", i + MOTOR_ID_FIRST, t, status);
        // MOT_STATUS
        t = (status & STATUS_MOT_STATUS) >> 5;
        if (motorStatus[i] != t) {
            motorStatus[i] = t;
            if (reportMotorStatus[i]) sendTwoInt("/motorStatus", i + MOTOR_ID_FIRST, motorStatus[i]);
            if (debugMode) sendStatusDebug("/motorStatus", i + MOTOR_ID_FIRST, t, status);
        }
        // CMD_ERROR, active high, latched
        t = (status & STATUS_CMD_ERROR) > 0;
        if (t && reportCommandError[i]) sendOneInt("commandError", i + MOTOR_ID_FIRST);
        if (debugMode && t) sendStatusDebug("/commandError", i + MOTOR_ID_FIRST, t, status);
        // UVLO, active low
        t = (status & STATUS_UVLO) == 0;
        if (t != uvloStatus[i])
        {
            uvloStatus[i] = !uvloStatus[i];
            if (reportUVLO[i]) sendTwoInt("/uvlo", i + MOTOR_ID_FIRST, uvloStatus[i]);
            if (debugMode) sendStatusDebug("/ulvo", i + MOTOR_ID_FIRST, t, status);
        }
        // TH_STATUS
        t = (status & STATUS_TH_STATUS) >> 11;
        if (thermalStatus[i] != t) {
            thermalStatus[i] = t;
            if (reportThermalStatus[i]) sendTwoInt("/thermalStatus", i + MOTOR_ID_FIRST, thermalStatus[i]);
            if (debugMode) sendStatusDebug("/thermalStatus", i + MOTOR_ID_FIRST, t, status);
        }
        // OCD, active low, latched
        t = (status & STATUS_OCD) == 0;
        if (t && reportOCD[i]) sendOneInt("/overCurrent", i + 1);
        if (debugMode && t) sendStatusDebug("/overCurrent", i + MOTOR_ID_FIRST, t, status);

        // STALL A&B, active low, latched
        t = (status & (STATUS_STALL_A | STATUS_STALL_B)) >> 14;
        if ((t != 3) && reportStall[i]) sendOneInt("/stall", i + MOTOR_ID_FIRST);
        if (debugMode && (t != 3)) sendStatusDebug("/stall", i + MOTOR_ID_FIRST, t, status);
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
                sendThreeInt("/limitSw", i + MOTOR_ID_FIRST, limitSwState[i], dir[i]);
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