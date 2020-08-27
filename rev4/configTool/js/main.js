window.addEventListener('DOMContentLoaded', function() {

    var myIpInput = document.querySelectorAll("input[name='myIp']");
    var isMyIpAddIdInput = document.querySelector("input[name='isMyIpAddId']");
    var destIpInput = document.querySelectorAll("input[name='destIp']");
    var dnsInput = document.querySelectorAll("input[name='dns']");
    var gatewayInput = document.querySelectorAll("input[name='gateway']");
    var subnetInput = document.querySelectorAll("input[name='subnet']");
    var inPortInput = document.querySelector("input[name='inPort']");
    var outPortInput = document.querySelector("input[name='outPort']");
    var isOutPortAddIdInput = document.querySelector("input[name='isOutPortAddId']");
    var macInput = document.querySelectorAll("input[name='mac']");
    var isMacAddIdInput = document.querySelector("input[name='isMacAddId']");
    var reportBUSYInput = document.querySelector("input[name='reportBUSY']");
    var reportFLAGInput = document.querySelector("input[name='reportFLAG']");
    var reportHiZInput = document.querySelector("input[name='reportHiZ']");
    var reportHomeSwStatusInput = document.querySelector("input[name='reportHomeSwStatus']");
    var reportDirInput = document.querySelector("input[name='reportDir']");
    var reportMotorStatusInput = document.querySelector("input[name='reportMotorStatus']");
    var reportSwEvnInput = document.querySelector("input[name='reportSwEvn']");
    var reportCommandErrorInput = document.querySelector("input[name='reportCommandError']");
    var reportUVLOInput = document.querySelector("input[name='reportUVLO']");
    var reportThermalStatusInput = document.querySelector("input[name='reportThermalStatus']");
    var reportOCDInput = document.querySelector("input[name='reportOCD']");
    var reportStallInput = document.querySelector("input[name='reportStall']");
    var reportLimitSwStatusInput = document.querySelector("input[name='reportLimitSwStatus']");
    var OCThresholdInput = document.querySelector("input[name='OCThreshold']");
    var stepModeInput = document.querySelector("input[name='stepMode']");
    var homeSwModeInput = document.querySelector("input[name='homeSwMode']");
    var limitSwModeInput = document.querySelector("input[name='limitSwMode']");
    var isCurrentModeInput = document.querySelector("input[name='isCurrentMode']");
    var slewRateInput = document.querySelector("input[name='slewRate']");
    var accInput = document.querySelector("input[name='acc']");
    var decInput = document.querySelector("input[name='dec']");
    var maxSpeedInput = document.querySelector("input[name='maxSpeed']");
    var fullStepSpeedInput = document.querySelector("input[name='fullStepSpeed']");
    var KVAL_HOLDInput = document.querySelector("input[name='KVAL_HOLD']");
    var KVAL_RUNInput = document.querySelector("input[name='KVAL_RUN']");
    var KVAL_ACCInput = document.querySelector("input[name='KVAL_ACC']");
    var KVAL_DECInput = document.querySelector("input[name='KVAL_DEC']");
    var INT_SPEEDInput = document.querySelector("input[name='INT_SPEED']");
    var ST_SLPInput = document.querySelector("input[name='ST_SLP']");
    var FN_SLP_ACCInput = document.querySelector("input[name='FN_SLP_ACC']");
    var FN_SLP_DECInput = document.querySelector("input[name='FN_SLP_DEC']");
    var STALL_THInput = document.querySelector("input[name='STALL_TH']");
    var lowSpeedOptimizeInput = document.querySelector("input[name='lowSpeedOptimize']");
    var TVAL_HOLDInput = document.querySelector("input[name='TVAL_HOLD']");
    var TVAL_RUNInput = document.querySelector("input[name='TVAL_RUN']");
    var TVAL_ACCInput = document.querySelector("input[name='TVAL_ACC']");
    var TVAL_DECInput = document.querySelector("input[name='TVAL_DEC']");
    var T_FASTInput = document.querySelector("input[name='T_FAST']");
    var TON_MINInput = document.querySelector("input[name='TON_MIN']");
    var TOFF_MINInput = document.querySelector("input[name='TOFF_MIN']");
    var kPInput = document.querySelector("input[name='kP']");
    var kIInput = document.querySelector("input[name='kI']");
    var kDInput = document.querySelector("input[name='kD']");

    //var exportBtn = document.querySelector("button[name='export']");
    //var exportInput = document.querySelector("input[name='export']");
    var form = document.querySelector("form");
    var loadInput = document.querySelector("input[name='load']");

    form.addEventListener('submit', function() {
        var configObject = {
            network: {
                myIp: [
                Number(myIpInput[0].value), 
                Number(myIpInput[1].value), 
                Number(myIpInput[2].value), 
                Number(myIpInput[3].value)
                ],
                isMyIpAddId: isMyIpAddIdInput.checked,
                destIp: [
                Number(destIpInput[0].value), 
                Number(destIpInput[1].value), 
                Number(destIpInput[2].value), 
                Number(destIpInput[3].value)
                ],
                dns: [
                Number(dnsInput[0].value), 
                Number(dnsInput[1].value), 
                Number(dnsInput[2].value), 
                Number(dnsInput[3].value)
                ],
                gateway: [
                Number(gatewayInput[0].value), 
                Number(gatewayInput[1].value), 
                Number(gatewayInput[2].value), 
                Number(gatewayInput[3].value)
                ],
                subnet: [
                Number(subnetInput[0].value), 
                Number(subnetInput[1].value), 
                Number(subnetInput[2].value), 
                Number(subnetInput[3].value)
                ],
                inPort: Number(inPortInput.value),
                outPort: Number(outPortInput.value),
                isOutPortAddId: isOutPortAddIdInput.checked,
                mac: [
                Number(macInput[0].value), 
                Number(macInput[1].value), 
                Number(macInput[2].value), 
                Number(macInput[3].value), 
                Number(macInput[4].value), 
                Number(macInput[5].value)
                ],
                isMacAddId: isMacAddIdInput.checked
            }, 
            alarmReport: {
                reportBUSY: reportBUSYInput.checked,
                reportFLAG: reportFLAGInput.checked,
                reportHiZ: reportHiZInput.checked,
                reportHomeSwStatus: reportHomeSwStatusInput.checked,
                reportDir: reportDirInput.checked,
                reportMotorStatus: reportMotorStatusInput.checked,
                reportSwEvn: reportSwEvnInput.checked,
                reportCommandError: reportCommandErrorInput.checked,
                reportUVLO: reportUVLOInput.checked,
                reportThermalStatus: reportThermalStatusInput.checked,
                reportOCD: reportOCDInput.checked,
                reportStall: reportStallInput.checked,
                reportLimitSwStatus: reportLimitSwStatusInput.checked,
                OCThreshold: Number(OCThresholdInput.value),
            },
            driverSettings: {
                stepMode: Number(stepModeInput.value),
                homeSwMode: homeSwModeInput.checked,
                limitSwMode: limitSwModeInput.checked,
                isCurrentMode: isCurrentModeInput.checked,
                slewRate: Number(slewRateInput.value)
            },
            speedProfile: {
                acc: Number(accInput.value),
                dec: Number(decInput.value),
                maxSpeed: Number(maxSpeedInput.value),
                fullStepSpeed: Number(fullStepSpeedInput.value)
            }, 
            voltageMode: {
                KVAL_HOLD: Number(KVAL_HOLDInput.value),
                KVAL_RUN: Number(KVAL_RUNInput.value),
                KVAL_ACC: Number(KVAL_ACCInput.value),
                KVAL_DEC: Number(KVAL_DECInput.value),
                INT_SPEED: Number(INT_SPEEDInput.value),
                ST_SLP: Number(ST_SLPInput.value),
                FN_SLP_ACC: Number(FN_SLP_ACCInput.value),
                FN_SLP_DEC: Number(FN_SLP_DECInput.value),
                STALL_TH: Number(STALL_THInput.value),
                lowSpeedOptimize: Number(lowSpeedOptimizeInput.value)
            },
            currentMode: {
                TVAL_HOLD: Number(TVAL_HOLDInput.value),
                TVAL_RUN: Number(TVAL_RUNInput.value),
                TVAL_ACC: Number(TVAL_ACCInput.value),
                TVAL_DEC: Number(TVAL_DECInput.value),
                T_FAST: Number(T_FASTInput.value),
                TON_MIN: Number(TON_MINInput.value),
                TOFF_MIN: Number(TOFF_MINInput.value)
            },
            servoMode: {
                kP: Number(kPInput.value),
                kI: Number(kIInput.value),
                kD: Number(kDInput.value)
            }
        };
        var configJson = JSON.stringify(configObject, null, '\t');
        var configBlob = new Blob([configJson], {type: 'text/plain'});
        var a = document.createElement('a');
        a.href = URL.createObjectURL(configBlob);
        a.download = 'config.txt';
        a.click();
    }, false);

loadInput.addEventListener('change', function(e) {
    var fileReader = new FileReader();
    fileReader.readAsText(e.target.files[0]);
    console.log(e.target);
    fileReader.onload = function(e) {
        var jsonObject = JSON.parse(e.target.result);
        console.log(jsonObject);

        myIpInput[0].value = jsonObject.network.myIp[0];
        myIpInput[1].value = jsonObject.network.myIp[1];
        myIpInput[2].value = jsonObject.network.myIp[2];
        myIpInput[3].value = jsonObject.network.myIp[3];
        isMyIpAddIdInput.checked = jsonObject.network.isMyIpAddId
        destIpInput[0].value = jsonObject.network.destIp[0];
        destIpInput[1].value = jsonObject.network.destIp[1];
        destIpInput[2].value = jsonObject.network.destIp[2];
        destIpInput[3].value = jsonObject.network.destIp[3];
        gatewayInput[0].value = jsonObject.network.gateway[0];
        gatewayInput[1].value = jsonObject.network.gateway[1];
        gatewayInput[2].value = jsonObject.network.gateway[2];
        gatewayInput[3].value = jsonObject.network.gateway[3];
        subnetInput[0].value = jsonObject.network.subnet[0];
        subnetInput[1].value = jsonObject.network.subnet[1];
        subnetInput[2].value = jsonObject.network.subnet[2];
        subnetInput[3].value = jsonObject.network.subnet[3];
        inPortInput.value = jsonObject.network.inPort;
        outPortInput.value = jsonObject.network.outPort;
        isOutPortAddIdInput.checked = jsonObject.network.isOutPortAddId;
        macInput[0].value = jsonObject.network.mac[0];
        macInput[1].value = jsonObject.network.mac[1];
        macInput[2].value = jsonObject.network.mac[2];
        macInput[3].value = jsonObject.network.mac[3];
        macInput[4].value = jsonObject.network.mac[4];
        macInput[5].value = jsonObject.network.mac[5];
        isMacAddIdInput.checked = jsonObject.network.isMacAddId;

        reportBUSYInput.checked = jsonObject.alarmReport.reportBUSY;
        reportFLAGInput.checked = jsonObject.alarmReport.reportFLAG;
        reportHiZInput.checked = jsonObject.alarmReport.reportHiZ;
        reportHomeSwStatusInput.checked = jsonObject.alarmReport.reportHomeSwStatus;
        reportDirInput.checked = jsonObject.alarmReport.reportDir;
        reportMotorStatusInput.checked = jsonObject.alarmReport.reportMotorStatus;
        reportSwEvnInput.checked = jsonObject.alarmReport.reportSwEvn;
        reportCommandErrorInput.checked = jsonObject.alarmReport.reportCommandError;
        reportUVLOInput.checked = jsonObject.alarmReport.reportUVLO;
        reportThermalStatusInput.checked = jsonObject.alarmReport.reportThermalStatus;
        reportOCDInput.checked = jsonObject.alarmReport.reportOCD;
        reportStallInput.checked = jsonObject.alarmReport.reportStall;
        reportLimitSwStatusInput.checked = jsonObject.alarmReport.reportLimitSwStatus;
        OCThresholdInput.value = jsonObject.alarmReport.OCThreshold;

        stepModeInput.value = jsonObject.driverSettings.stepMode;
        homeSwModeInput.checked = jsonObject.driverSettings.homeSwMode;
        limitSwModeInput.checked = jsonObject.driverSettings.limitSwMode;
        isCurrentModeInput.checked = jsonObject.driverSettings.isCurrentMode;
        slewRateInput.value = jsonObject.driverSettings.slewRate;

        accInput.value = jsonObject.speedProfile.acc;
        decInput.value = jsonObject.speedProfile.dec;
        maxSpeedInput.value = jsonObject.speedProfile.maxSpeed;
        fullStepSpeedInput.value = jsonObject.speedProfile.fullStepSpeed;

        KVAL_HOLDInput.value = jsonObject.voltageMode.KVAL_HOLD;
        KVAL_RUNInput.value = jsonObject.voltageMode.KVAL_RUN;
        KVAL_ACCInput.value = jsonObject.voltageMode.KVAL_ACC;
        KVAL_DECInput.value = jsonObject.voltageMode.KVAL_DEC;
        INT_SPEEDInput.value = jsonObject.voltageMode.INT_SPEED;
        ST_SLPInput.value = jsonObject.voltageMode.ST_SLP;
        FN_SLP_ACCInput.value = jsonObject.voltageMode.FN_SLP_ACC;
        FN_SLP_DECInput.value = jsonObject.voltageMode.FN_SLP_DEC;
        STALL_THInput.value = jsonObject.voltageMode.STALL_TH;
        lowSpeedOptimizeInput.value = jsonObject.voltageMode.lowSpeedOptimize;

        TVAL_HOLDInput.value = jsonObject.currentMode.TVAL_HOLD;
        TVAL_RUNInput.value = jsonObject.currentMode.TVAL_RUN;
        TVAL_ACCInput.value = jsonObject.currentMode.TVAL_ACC;
        TVAL_DECInput.value = jsonObject.currentMode.TVAL_DEC;
        T_FASTInput.value = jsonObject.currentMode.T_FAST;
        TON_MINInput.value = jsonObject.currentMode.TON_MIN;
        TOFF_MINInput.value = jsonObject.currentMode.TOFF_MIN;

        kPInput.value = jsonObject.servoMode.kP;
        kIInput.value = jsonObject.servoMode.kI;
        kDInput.value = jsonObject.servoMode.kD;
    }

}, false)
});