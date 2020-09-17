void loadConfig() {
    uint8_t i;
    sdInitializeSucceeded = SD.begin(SD_CS_PIN);

    File file = SD.open(filename);
    configFileOpenSucceeded = (file != false);
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = 49 * JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(13) + JSON_OBJECT_SIZE(14) + 830;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, file);
    configFileParseSucceeded = (error == false);
    file.close();
    SD.end();

    // Information
    const char* _configName = doc["information"]["configName"] | "Default";

    // Network
    JsonObject network = doc["network"];
    JsonArray network_myIp = network["myIp"];
    JsonArray network_destIp = network["destIp"];
    JsonArray network_dns = network["dns"];
    JsonArray network_gateway = network["gateway"];
    JsonArray network_subnet = network["subnet"];
    for (i = 0; i < 4; i++) {
        myIp[i] = network_myIp[i] | myIp[i];
        destIp[i] = network_destIp[i] | destIp[i];
        dns[i] = network_dns[i] | dns[i];
        gateway[i] = network_gateway[i] | gateway[i];
        subnet[i] = network_subnet[i] | subnet[i];
    }
    isMyIpAddId = network["isMyIpAddId"] | true;
    inPort = network["inPort"] | 50000;
    outPort = network["outPort"] | 50100;
    isOutPortAddId = network["isOutPortAddId"] | true;
    JsonArray network_mac = network["mac"];
    for (i = 0; i < 6; i++) {
        mac[i] = network_mac[i] | mac[i];
    }
    isMacAddId = network["isMacAddId"] | true;
    bootedMsgEnable = network["bootedMsgEnable"] | true;
    isDestIpSet = network["canSendMsgBeforeDestIp"] | false;

    // Alarm and Report
    JsonObject alarmAndReport = doc["alarmAndReport"];
    JsonArray alarmAndReport_reportBUSY = alarmAndReport["reportBUSY"];
    JsonArray alarmAndReport_reportFLAG = alarmAndReport["reportFLAG"];
    JsonArray alarmAndReport_reportHiZ = alarmAndReport["reportHiZ"];
    JsonArray alarmAndReport_reportHomeSwStatus = alarmAndReport["reportHomeSwStatus"];
    JsonArray alarmAndReport_reportDir = alarmAndReport["reportDir"];
    JsonArray alarmAndReport_reportMotorStatus = alarmAndReport["reportMotorStatus"];
    JsonArray alarmAndReport_reportSwEvn = alarmAndReport["reportSwEvn"];
    JsonArray alarmAndReport_reportCommandError = alarmAndReport["reportCommandError"];
    JsonArray alarmAndReport_reportUVLO = alarmAndReport["reportUVLO"];
    JsonArray alarmAndReport_reportThermalStatus = alarmAndReport["reportThermalStatus"];
    JsonArray alarmAndReport_reportOCD = alarmAndReport["reportOCD"];
    JsonArray alarmAndReport_reportStall = alarmAndReport["reportStall"];
    JsonArray alarmAndReport_reportLimitSwStatus = alarmAndReport["reportLimitSwStatus"];
    JsonArray alarmAndReport_OCThreshold = alarmAndReport["OCThreshold"];
    for (i = 0; i < NUM_OF_MOTOR; i++)
    {
        reportBUSY[i] = alarmAndReport_reportBUSY[i] | false;
        reportFLAG[i] = alarmAndReport_reportFLAG[i] | false;
        reportHiZ[i] = alarmAndReport_reportHiZ[i] | false;
        reportHomeSwStatus[i] = alarmAndReport_reportHomeSwStatus[i] | false;
        reportDir[i] = alarmAndReport_reportDir[i] | false;
        reportMotorStatus[i] = alarmAndReport_reportMotorStatus[i] | false;
        reportSwEvn[i] = alarmAndReport_reportSwEvn[i] | false;
        reportCommandError[i] = alarmAndReport_reportCommandError[i] | true;
        reportUVLO[i] = alarmAndReport_reportUVLO[i] | true;
        reportThermalStatus[i] = alarmAndReport_reportThermalStatus[i] | true;
        reportOCD[i] = alarmAndReport_reportOCD[i] | true;
        reportStall[i] = alarmAndReport_reportStall[i] | true;
        reportLimitSwStatus[i] = alarmAndReport_reportLimitSwStatus[i] | false;
        overCurrentThreshold[i] = alarmAndReport_OCThreshold[i] | 15;
    }

    // Driver settings
    JsonObject driverSettings = doc["driverSettings"];
    JsonArray driverSettings_stepMode = driverSettings["stepMode"];
    JsonArray driverSettings_homeSwMode = driverSettings["homeSwMode"];
    JsonArray driverSettings_limitSwMode = driverSettings["limitSwMode"];
    JsonArray driverSettings_isCurrentMode = driverSettings["isCurrentMode"];
    JsonArray driverSettings_slewRate = driverSettings["slewRate"];
    JsonArray driverSettings_electromagnetBrakeEnable = driverSettings["electromagnetBrakeEnable"];
    uint16_t slewRateVal[6] = { SR_114V_us, SR_220V_us, SR_400V_us, SR_520V_us, SR_790V_us, SR_980V_us };
    for (i = 0; i < NUM_OF_MOTOR; i++) {
        microStepMode[i] = driverSettings_stepMode[i] | STEP_SEL_1_128;
        homeSwMode[i] = driverSettings_homeSwMode[i] | true; // true: SW_USER, false: SW_HARDSTOP
        limitSwMode[i] = driverSettings_limitSwMode[i] | true;
        isCurrentMode[i] = driverSettings_isCurrentMode[i] | false;
        uint8_t slewRateNum = constrain((driverSettings_slewRate[i] | 5), 0, 5); // default SR_980V_us
        slewRate[i] = slewRateVal[slewRateNum];
        electromagnetBrakeEnable[i] = driverSettings_electromagnetBrakeEnable[i] | false;
    }

    // Speed profile
    JsonObject speedProfile = doc["speedProfile"];
    JsonArray speedProfile_acc = speedProfile["acc"];
    JsonArray speedProfile_dec = speedProfile["dec"];
    JsonArray speedProfile_maxSpeed = speedProfile["maxSpeed"];
    JsonArray speedProfile_fullStepSpeed = speedProfile["fullStepSpeed"];
    for (i = 0; i < NUM_OF_MOTOR; i++) {
        acc[i] = speedProfile_acc[i] | 1000.;
        dec[i] = speedProfile_dec[i] | 1000.;
        maxSpeed[i] = speedProfile_maxSpeed[i] | 650.;
        fullStepSpeed[i] = speedProfile_fullStepSpeed[i] | 15610.;
    }

    // Voltage mode
    JsonObject voltageMode = doc["voltageMode"];
    JsonArray voltageMode_KVAL_HOLD = voltageMode["KVAL_HOLD"];
    JsonArray voltageMode_KVAL_RUN = voltageMode["KVAL_RUN"];
    JsonArray voltageMode_KVAL_ACC = voltageMode["KVAL_ACC"];
    JsonArray voltageMode_KVAL_DEC = voltageMode["KVAL_DEC"];
    JsonArray voltageMode_INT_SPEED = voltageMode["INT_SPEED"];
    JsonArray voltageMode_ST_SLP = voltageMode["ST_SLP"];
    JsonArray voltageMode_FN_SLP_ACC = voltageMode["FN_SLP_ACC"];
    JsonArray voltageMode_FN_SLP_DEC = voltageMode["FN_SLP_DEC"];
    JsonArray voltageMode_STALL_TH = voltageMode["STALL_TH"];
    JsonArray voltageMode_lowSpeedOptimize = voltageMode["lowSpeedOptimize"];
    for (i = 0; i < NUM_OF_MOTOR; i++) {
        kvalHold[i] = voltageMode_KVAL_HOLD[i] | 0;
        kvalRun[i] = voltageMode_KVAL_RUN[i] | 16;
        kvalAcc[i] = voltageMode_KVAL_ACC[i] | 16;
        kvalDec[i] = voltageMode_KVAL_DEC[i] | 16;
        intersectSpeed[i] = voltageMode_INT_SPEED[i] | 0x0408;
        startSlope[i] = voltageMode_ST_SLP[i] | 0x19;
        accFinalSlope[i] = voltageMode_FN_SLP_ACC[i] | 0x29;
        decFinalSlope[i] = voltageMode_FN_SLP_DEC[i] | 0x29;
        stallThreshold[i] = voltageMode_STALL_TH[i] | 0x1F;
        lowSpeedOptimize[i] = voltageMode_lowSpeedOptimize[i] | 20.;
    }

    // Current mode
    JsonObject currentMode = doc["currentMode"];
    JsonArray currentMode_TVAL_HOLD = currentMode["TVAL_HOLD"];
    JsonArray currentMode_TVAL_RUN = currentMode["TVAL_RUN"];
    JsonArray currentMode_TVAL_ACC = currentMode["TVAL_ACC"];
    JsonArray currentMode_TVAL_DEC = currentMode["TVAL_DEC"];
    JsonArray currentMode_T_FAST = currentMode["T_FAST"];
    JsonArray currentMode_TON_MIN = currentMode["TON_MIN"];
    JsonArray currentMode_TOFF_MIN = currentMode["TOFF_MIN"];
    for (i = 0; i < NUM_OF_MOTOR; i++) {
        tvalHold[i] = currentMode_TVAL_HOLD[i] | 0;
        tvalRun[i] = currentMode_TVAL_RUN[i] | 16;
        tvalAcc[i] = currentMode_TVAL_ACC[i] | 16;
        tvalDec[i] = currentMode_TVAL_DEC[i] | 16;
        fastDecaySetting[i] = currentMode_T_FAST[i] | 25;
        minOnTime[i] = currentMode_TON_MIN[i] | 41;
        minOffTime[i] = currentMode_TOFF_MIN[i] | 41;
    }

    // Servo mode
    JsonObject servoMode = doc["servoMode"];
    JsonArray servoMode_kP = servoMode["kP"];
    JsonArray servoMode_kI = servoMode["kI"];
    JsonArray servoMode_kD = servoMode["kD"];
    for (i = 0; i < NUM_OF_MOTOR; i++) {
        kP[i] = servoMode_kP[i] | 0.06;
        kI[i] = servoMode_kI[i] | 0.0;
        kD[i] = servoMode_kD[i] | 0.0;
    }
}

// for debug
void setDefaultVal() {
    inPort = 50000;
    outPort = 50100;
    isOutPortAddId = true;
    isMacAddId = true;
    isDestIpSet = false;

    for (uint8_t i = 0; i < NUM_OF_MOTOR; i++)
    {
        //busy[i] = false;
        //flag[i] = false;
        //HiZ[i] = false;
        //homeSwState[i] = false;
        //dir[i] = false;
        //uvloStatus[i] = false;
        //motorStatus[i] = 0;
        //thermalStatus[i] = 0;

        //reportBUSY[i] = false;
        //reportFLAG[i] = false;
        //reportHiZ[i] = false;
        //reportHomeSwStatus[i] = false;
        //reportDir[i] = false;
        //reportMotorStatus[i] = false;
        //reportSwEvn[i] = false;
        //reportCommandError[i] = true;
        //reportUVLO[i] = true;
        //reportThermalStatus[i] = true;
        //reportOCD[i] = true;
        //reportStall[i] = true;
        //overCurrentThreshold[i] = 15; //

        //limitSwState[i] = false;
        //reportLimitSwStatus[i] = false;
        //limitSwMode[i] = false;

        //microStepMode[i] = STEP_SEL_1_128;
        //homeSwMode[i] = true;
        //isCurrentMode[i] = false;
        //slewRate[i] = SR_980V_us;
        //electromagnetBrakeEnable[i] = false;

        //acc[i] = 1000.;
        //dec[i] = 1000.;
        //maxSpeed[i] = 650.;
        //fullStepSpeed[i] = 15610.;

        //kvalHold[i] = tvalHold[i] = 0;
        //kvalRun[i] = tvalRun[i] = 16;
        //kvalAcc[i] = tvalAcc[i] = 16;
        //kvalDec[i] = tvalDec[i] = 16;

        //intersectSpeed[i] = 0x0408; // INT_SPEED
        //startSlope[i] = 0x19;// ST_SLP
        //accFinalSlope[i] = 0x29; // FN_SLP_ACC
        //decFinalSlope[i] = 0x29; // FN_SLP_DEC
        //lowSpeedOptimize[i] = 20.;
        //fastDecaySetting[i] = 0x19; // T_FAST
        //minOnTime[i] = 0x29; // TON_MIN
        //minOffTime[i] = 0x29; // TOFF_MIN

        //targetPosition[i] = 0;
        //kP[i] = 0.06f;
        //kI[i] = 0.0f;
        //kD[i] = 0.0f;
        //isServoMode[i] = false;

    }
}


