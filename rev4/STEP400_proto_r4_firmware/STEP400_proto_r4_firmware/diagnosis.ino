void diagnosis(uint8_t inByte) {
	switch (inByte)
	{
	case 'c':
		printLoadedConfig();
		break;
	case 's':
		printCurrentState();
		break;
	case 'b':
		//sendBootMsg();
		break;
	default:
		break;
	}
}

void printCurrentState() {
	SerialUSB.print("Firmware name:");
	String version = COMPILE_DATE;
	version += String(" ") + String(COMPILE_TIME) + String(" ") + String(FIRMWARE_NAME);
	SerialUSB.println(version);
	showHeader("DIP Switch");
	SerialUSB.println(getMyId(), BIN); // have to fix
	showHeader("Ethernet");
	SerialUSB.print( F("Ethernet link status: ") );
	SerialUSB.println( Ethernet.linkStatus() );
	SerialUSB.print(F("Ethernet hardware status: "));
	SerialUSB.println(Ethernet.hardwareStatus());
	showHeader("microSD");
	showBoolResult(F("SD library initialize succeeded"), sdInitializeSucceeded);
	showBoolResult(F("SD config file open succeeded"), configFileOpenSucceeded);
	showBoolResult(F("SD config file parse succeeded"), configFileParseSucceeded);
	showHeader("PowerSTEP01");
	SerialUSB.print(F("STATUS"));
	for (uint8_t i = 0; i < NUM_OF_MOTOR; i++)
	{
		SerialUSB.print(F(", "));
		SerialUSB.print(stepper[i].getStatus());
	}
	SerialUSB.println();
}


#pragma region print_config
void printLoadedConfig() {
	SerialUSB.print("configName:");
	SerialUSB.println(configName.c_str());
	showBoolResult(F("SD library initialize succeeded"), sdInitializeSucceeded);
	showBoolResult(F("SD config file open succeeded"),configFileOpenSucceeded);
	showBoolResult(F("SD config file parse succeeded"), configFileParseSucceeded);
	//
	//showBoolResult(F(""), );
	showHeader("Network");
	showIpAddress(F("My Ip"), myIp);
	showIpAddress(F("Dest Ip"), destIp);
	showIpAddress(F("DNS"), dns);
	showIpAddress(F("Gateway"), gateway);
	showIpAddress(F("Subnet mask"), subnet);
	showBoolResult(F("isMyIpAddId"), isMyIpAddId);
	showBoolResult(F("isMacAddId"), isMacAddId);
	showBoolResult(F("bootedMsgEnable"), bootedMsgEnable);
	showBoolResult(F("isDestIpSet"), isDestIpSet);

	showHeader("Report & Alarm");
	show4Bool(F("reportBUSY"), reportBUSY);
	show4Bool(F("reportFLAG"), reportFLAG);
	show4Bool(F("reportHiZ"), reportHiZ);
	show4Bool(F("reportHomeSwStatus"), reportHomeSwStatus);
	show4Bool(F("reportDir"), reportDir);
	show4Bool(F("reportMotorStatus"), reportMotorStatus);
	show4Bool(F("reportSwEvn"), reportSwEvn);
	show4Bool(F("reportCommandError"), reportCommandError);
	show4Bool(F("reportUVLO"), reportUVLO);
	show4Bool(F("reportThermalStatus"), reportThermalStatus);
	show4Bool(F("reportOCD"), reportOCD);
	show4Bool(F("reportStall"), reportStall);
	show4Bool(F("reportLimitSwStatus"), reportLimitSwStatus);
	show4Bool(F("reportOCD"), reportOCD);
	show4Bytes(F("OCThreshold"), overCurrentThreshold);

	showHeader("driverSettings");
	show4Bytes(F("microStepMode"), microStepMode);
	show4Bool(F("homeSwMode"), homeSwMode);
	show4Bool(F("limitSwMode"), limitSwMode);
	show4Bool(F("isCurrentMode"), isCurrentMode);
	show4Bytes(F("slewRate"), slewRateNum);
	show4Bool(F("electromagnetBrakeEnable"), electromagnetBrakeEnable);

	showHeader("speedProfile");
	show4Floats(F("acc"), acc);
	show4Floats(F("dec"), dec);
	show4Floats(F("maxSpeed"), maxSpeed);
	show4Floats(F("fullStepSpeed"), fullStepSpeed);

	showHeader("Voltage mode");
	show4Bytes(F("kvalHold"), kvalHold);
	show4Bytes(F("kvalRun"), kvalRun);
	show4Bytes(F("kvalAcc"), kvalAcc);
	show4Bytes(F("kvalDec"), kvalDec);
	show4Int16s(F("intersectSpeed"), intersectSpeed);
	show4Bytes(F("startSlope"), startSlope);
	show4Bytes(F("accFinalSlope"), accFinalSlope);
	show4Bytes(F("decFinalSlope"), decFinalSlope);
	show4Bytes(F("stallThreshold"), stallThreshold);
	show4Floats(F("lowSpeedOptimize"), lowSpeedOptimize);

	showHeader("Current mode");
	show4Bytes(F("tvalHold"), tvalHold);
	show4Bytes(F("tvalRun"), tvalRun);
	show4Bytes(F("tvalAcc"), tvalAcc);
	show4Bytes(F("tvalDec"), tvalDec);
	show4Bytes(F("fastDecaySetting"), fastDecaySetting);
	show4Bytes(F("minOnTime"), minOnTime);
	show4Bytes(F("minOffTime"), minOffTime);

	showHeader("Servo mode");
	show4Floats(F("kP"), kP);
	show4Floats(F("kI"), kI);
	show4Floats(F("kD"), kD);

	showHeader("Print config END");
}

void printTitle(String title) {
	SerialUSB.print(title);
	SerialUSB.print("\t: ");
}
void showBoolResult(String title, bool val) {
	String res = (val) ? "Yes" : "No";
	printTitle(title);
	SerialUSB.println(res);
}

void show4Bool(String title, bool* val) {
	printTitle(title);
	SerialUSB.print(" ");
	for (uint8_t i = 0; i < 4; i++)
	{
		String res = (val[i]) ? " Yes" : " No";
		SerialUSB.print(res);
		if (i < 3) SerialUSB.print(",\t");
	}
	SerialUSB.println();
}
void show4Bytes(String title, uint8_t* val) {
	printTitle(title);
	for (uint8_t i = 0; i < 4; i++)
	{
		SerialUSB.print(val[i]);
		if (i < 3) SerialUSB.print(",\t");
	}
	SerialUSB.println();
}
void show4Int16s(String title, uint16_t* val) {
	printTitle(title);
	for (uint8_t i = 0; i < 4; i++)
	{
		SerialUSB.print(val[i]);
		if (i < 3) SerialUSB.print(",\t");
	}
	SerialUSB.println();
}
void show4Floats(String title, float* val) {
	printTitle(title);
	for (uint8_t i = 0; i < 4; i++)
	{
		SerialUSB.print(val[i]);
		if (i < 3) SerialUSB.print(",\t");
	}
	SerialUSB.println();
}
void showIpAddress(String title, IPAddress ip) {
	printTitle(title);
	for (uint8_t i = 0; i < 4; i++)
	{
		SerialUSB.print(ip[i]);
		if (i < 3) SerialUSB.print(".");
	}
	SerialUSB.println();
}

void showHeader(String header) {
	String line = " -------------- ";
	SerialUSB.print(line);
	SerialUSB.print(header);
	SerialUSB.println(line);
}
#pragma endregion 
