void diagnosisSetup() {
	digitalWrite(ledPin, HIGH);
	SerialUSB.begin(9600);
	//while (!SerialUSB) continue;
	SerialUSB.println("Diagnosis mode.");
	digitalWrite(ledPin, LOW);
	// DIP switch test
	// SD
	// Ethernet
	// PowerSTEP01
}

void diagnosisLoop() {
	uint8_t inByte;
	if (SerialUSB.available() > 0)
	{
		inByte = SerialUSB.read();
		switch (inByte)
		{
		case 'p':
			printLoadedConfig();
			break;
		case 'b':
			sendBootMsg();
			break;
		default:
			break;
		}
	}
}

#pragma region print_config
void printLoadedConfig() {
	SerialUSB.print("Project name:");
	String version = COMPILE_DATE;
	version += String(" ") + String(COMPILE_TIME) + String(" ") + String(PROJECT_NAME);
	SerialUSB.println(version);
	SerialUSB.print("configName:");
	SerialUSB.println(String(configName));// have to be fixed
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
//	show4Bytes(F("slewRate"), slewRate);
	show4Bool(F("electromagnetBrakeEnable"), electromagnetBrakeEnable);

	showHeader("speedProfile");

	showHeader("Voltage mode");
	show4Bytes(F("kvalHold"), kvalHold);
	show4Bytes(F("kvalRun"), kvalRun);
	show4Bytes(F("kvalAcc"), kvalAcc);
	show4Bytes(F("kvalDec"), kvalDec);
	//intersectSpeed
	show4Bytes(F("startSlope"), startSlope);
	show4Bytes(F("accFinalSlope"), accFinalSlope);
	show4Bytes(F("decFinalSlope"), decFinalSlope);
	show4Bytes(F("stallThreshold"), stallThreshold);
	// lo_opt

	showHeader("Current mode");
	show4Bytes(F("tvalHold"), tvalHold);
	show4Bytes(F("tvalRun"), tvalRun);
	show4Bytes(F("tvalAcc"), tvalAcc);
	show4Bytes(F("tvalDec"), tvalDec);
	show4Bytes(F("fastDecaySetting"), fastDecaySetting);
	show4Bytes(F("minOnTime"), minOnTime);
	show4Bytes(F("minOffTime"), minOffTime);

	showHeader("Servo mode");

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
