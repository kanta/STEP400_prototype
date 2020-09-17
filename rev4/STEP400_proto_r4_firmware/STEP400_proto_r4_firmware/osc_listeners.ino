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

void getConfigName(OSCMessage& msg, int addrOffset) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/configName");
    newMes.add(configName).add(sdInitializeSucceeded).add(configFileOpenSucceeded).add(configFileParseSucceeded);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void getConfigRegister(uint8_t deviceID) {
    sendTwoInt("/configRegister", deviceID, stepper[deviceID-MOTOR_ID_FIRST].getParam(CONFIG));
}
void getConfigRegister(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getConfigRegister(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getConfigRegister(i + MOTOR_ID_FIRST);
        }
    }
}

// reset the motor driver chip and setup it
void resetMotorDriver(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        resetMotorDriver(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            resetMotorDriver(i + MOTOR_ID_FIRST);
        }
    }
}

void setDebugMode(OSCMessage& msg, int addrOffset) {
    debugMode = msg.getInt(0) > 0;
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
        reportBUSY[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportHiZ[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportHomeSwStatus[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportLimitSwStatus[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportDir[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportMotorStatus[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportSwEvn[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportCommandError[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportUVLO[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportThermalStatus[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportOCD[motorID - MOTOR_ID_FIRST] = bEnable;
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
        reportStall[motorID - MOTOR_ID_FIRST] = bEnable;
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
    sendThreeInt("/homeSw", motorID, homeSwState[motorID - MOTOR_ID_FIRST], dir[motorID - MOTOR_ID_FIRST]);
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
    uint8_t value = constrain(msg.getInt(1), STEP_FS, STEP_FS_128); // 0-7
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].configStepMode(value);
        microStepMode[motorID - MOTOR_ID_FIRST] = value;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].configStepMode(value);
            microStepMode[i] = value;
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
    uint16_t value = (msg.getInt(1) > 0) ? SW_USER : SW_HARD_STOP;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setSwitchMode(value);
        homeSwMode[motorID - MOTOR_ID_FIRST] = (value > 0);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setSwitchMode(value);
            homeSwMode[i] = (value > 0);
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
        overCurrentThreshold[motorID - MOTOR_ID_FIRST] = threshold;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setParam(OCD_TH, threshold);
            overCurrentThreshold[i] = threshold;
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
    float _minSpeed = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setMinSpeed(_minSpeed);
        lowSpeedOptimize[motorID] = _minSpeed;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMinSpeed(_minSpeed);
            lowSpeedOptimize[i] = _minSpeed;
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

void setBemfParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t intSpeed = constrain(msg.getInt(1), 0, 0x3FFF);
    uint8_t
        stSlp = constrain(msg.getInt(2), 0, 255),
        fnSlpAcc = constrain(msg.getInt(3), 0, 255),
        fnSlpDec = constrain(msg.getInt(4), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        intersectSpeed[motorID] = intSpeed;
        startSlope[motorID] = stSlp;
        accFinalSlope[motorID] = fnSlpAcc;
        decFinalSlope[motorID] = fnSlpDec;
        stepper[motorID].setParam(INT_SPD, intersectSpeed[motorID]);
        stepper[motorID].setParam(ST_SLP, startSlope[motorID]);
        stepper[motorID].setParam(FN_SLP_ACC, accFinalSlope[motorID]);
        stepper[motorID].setParam(FN_SLP_DEC, decFinalSlope[motorID]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            intersectSpeed[i] = intSpeed;
            startSlope[i] = stSlp;
            accFinalSlope[i] = fnSlpAcc;
            decFinalSlope[i] = fnSlpDec;
            stepper[i].setParam(INT_SPD, intersectSpeed[i]);
            stepper[i].setParam(ST_SLP, startSlope[i]);
            stepper[i].setParam(FN_SLP_ACC, accFinalSlope[i]);
            stepper[i].setParam(FN_SLP_DEC, decFinalSlope[i]);
        }
    }
}
void getBemfParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getBemfParam(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getBemfParam(i + MOTOR_ID_FIRST);
        }
    }
}
void getBemfParam(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/bemfParam");
    newMes.add((int32_t)motorID);
    motorID -= MOTOR_ID_FIRST;
    newMes.add(intersectSpeed[motorID]).add(startSlope[motorID]).add(accFinalSlope[motorID]).add(decFinalSlope[motorID]);
    Udp.beginPacket(destIp, outPort);
    newMes.send(Udp);
    Udp.endPacket();
    newMes.empty();
    turnOnTXL();
}

void setDecayModeParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint8_t
        tFast = constrain(msg.getInt(1), 0, 255),
        tOnMin = constrain(msg.getInt(2), 0, 255),
        tOffMin = constrain(msg.getInt(3), 0, 255);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        fastDecaySetting[motorID] = tFast;
        minOnTime[motorID] = tOnMin;
        minOffTime[motorID] = tOffMin;
        stepper[motorID].setParam(T_FAST, fastDecaySetting[motorID]);
        stepper[motorID].setParam(TON_MIN, minOnTime[motorID]);
        stepper[motorID].setParam(TOFF_MIN, minOffTime[motorID]);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            fastDecaySetting[i] = tFast;
            minOnTime[i] = tOnMin;
            minOffTime[i] = tOffMin;
            stepper[i].setParam(T_FAST, fastDecaySetting[i]);
            stepper[i].setParam(TON_MIN, minOnTime[i]);
            stepper[i].setParam(TOFF_MIN, minOffTime[i]);
        }
    }
}
void getDecayModeParam(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        getDecayModeParam(motorID);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            getDecayModeParam(i + MOTOR_ID_FIRST);
        }
    }
}
void getDecayModeParam(uint8_t motorID) {
    if (!isDestIpSet) { return; }
    OSCMessage newMes("/decayModeParam");
    newMes.add((int32_t)motorID);
    motorID -= MOTOR_ID_FIRST;
    newMes.add(fastDecaySetting[motorID]).add(minOnTime[motorID]).add(minOffTime[motorID]);
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
    return (tval + 1) * 78.125f;
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
            getTval_mA(i + MOTOR_ID_FIRST);
        }
    }
}
#pragma endregion tval_commands_osc_listener

#pragma region speed_commands_osc_listener

void setSpeedProfile(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _acc = msg.getFloat(1);
    float _dec = msg.getFloat(2);
    float _maxSpeed = msg.getFloat(3);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setAcc(_acc);
        stepper[motorID].setDec(_dec);
        stepper[motorID].setMaxSpeed(_maxSpeed);
        acc[motorID] = _acc;
        dec[motorID] = _dec;
        maxSpeed[motorID] = _maxSpeed;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAcc(_acc);
            stepper[i].setDec(_dec);
            stepper[i].setMaxSpeed(_maxSpeed);
            acc[i] = _acc;
            dec[i] = _dec;
            maxSpeed[i] = _maxSpeed;
        }
    }
}

void setMaxSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _maxSpeed = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setMaxSpeed(_maxSpeed);
        maxSpeed[motorID] = _maxSpeed;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setMaxSpeed(_maxSpeed);
            maxSpeed[i] = _maxSpeed;
        }
    }
}
// MIN_SPEED register is set by setLowSpeedOptimizeThreshold function.

void setFullstepSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _fullStepSpeed = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setFullSpeed(_fullStepSpeed);
        fullStepSpeed[motorID] = _fullStepSpeed;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setFullSpeed(_fullStepSpeed);
            fullStepSpeed[i] = _fullStepSpeed;
        }
    }
}
void getFullstepSpeed(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _fullStepSpeed;
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        _fullStepSpeed = stepper[motorID - MOTOR_ID_FIRST].getFullSpeed();
        sendIdFloat("/fullstepSpeed", motorID, _fullStepSpeed);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            _fullStepSpeed = stepper[i].getFullSpeed();
            sendIdFloat("/fullstepSpeed", i + 1, _fullStepSpeed);
        }
    }
}
void setAcc(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _acc = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setAcc(_acc);
        acc[motorID] = _acc;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAcc(_acc);
            acc[i] = _acc;
        }
    }
}
void setDec(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    float _dec = msg.getFloat(1);
    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        motorID -= MOTOR_ID_FIRST;
        stepper[motorID].setDec(_dec);
        dec[motorID] = _dec;
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setDec(_dec);
            dec[i] = _dec;
        }
    }
}

void setSpeedProfileRaw(OSCMessage& msg, int addrOffset) {
    uint8_t motorID = msg.getInt(0);
    uint16_t _accRaw = msg.getInt(1);
    uint16_t _decRaw = msg.getInt(2);
    uint16_t _maxSpeedRaw = msg.getInt(3);

    if (MOTOR_ID_FIRST <= motorID && motorID <= MOTOR_ID_LAST) {
        stepper[motorID - MOTOR_ID_FIRST].setAccRaw(_accRaw);
        stepper[motorID - MOTOR_ID_FIRST].setDecRaw(_decRaw);
        stepper[motorID - MOTOR_ID_FIRST].setMaxSpeedRaw(_maxSpeedRaw);
    }
    else if (motorID == MOTOR_ID_ALL) {
        for (uint8_t i = 0; i < NUM_OF_MOTOR; i++) {
            stepper[i].setAccRaw(_accRaw);
            stepper[i].setDecRaw(_decRaw);
            stepper[i].setMaxSpeedRaw(_maxSpeedRaw);
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
            sendIdFloat("/speed", i + MOTOR_ID_FIRST, s);
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
            getSpeedProfile(i + MOTOR_ID_FIRST);
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
            getSpeedProfileRaw(i + MOTOR_ID_FIRST);
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
    bool action = msg.getInt(1) > 0;
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
    newMes.add(motorID);
    motorID -= MOTOR_ID_FIRST;
    newMes.add(kP[motorID]).add(kI[motorID]).add(kD[motorID]);
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
    stepper[motorID].setParam(ST_SLP, startSlope[motorID]);
    stepper[motorID].setParam(FN_SLP_ACC, accFinalSlope[motorID]);
    stepper[motorID].setParam(FN_SLP_DEC, decFinalSlope[motorID]);
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
    if (stepper[motorID].getStepMode() > STEP_SEL_1_16)
    {
        stepper[motorID].configStepMode(STEP_SEL_1_16);
        microStepMode[motorID] = STEP_SEL_1_16;
    }
    stepper[motorID].setHoldTVAL(tvalHold[motorID]);
    stepper[motorID].setRunTVAL(tvalRun[motorID]);
    stepper[motorID].setAccTVAL(tvalAcc[motorID]);
    stepper[motorID].setDecTVAL(tvalDec[motorID]);
    stepper[motorID].setParam(T_FAST, fastDecaySetting[motorID]);
    stepper[motorID].setParam(TON_MIN, minOnTime[motorID]);
    stepper[motorID].setParam(TOFF_MIN, minOffTime[motorID]);
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
