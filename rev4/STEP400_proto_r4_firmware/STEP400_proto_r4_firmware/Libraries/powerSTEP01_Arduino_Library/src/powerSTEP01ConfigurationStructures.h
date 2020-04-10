#include "powerSTEP01ArduinoLibrary.h"

struct basicPowerSTEP01Configuration
{
  float maxSpeed;

  byte overCurrentThreshold;

  byte runKval;
  byte holdKval;

  void Reset()
  {
    maxSpeed = 500;
    
    overCurrentThreshold = 20;
    
    runKval = 64;
    holdKval = 32;
  }
};

struct powerSTEP01Configuration
{
  byte syncPinMode;
  byte syncDivisor;

  byte stepMode;

  float maxSpeed;
  float minSpeed;
  float fullStepsSpeed;
  float acceleration;
  float deceleration;

  int slewRate;

  byte overCurrentThreshold;
  int overCurrentShutdown;

  int pwmDivisor;
  int pwmMultiplier;

  int voltageCompensation;

  int switchMode;

  int clockSource;

  byte runKval;
  byte accelerationKval;
  byte decelerationKval;
  byte holdKval;

  byte alarmEn;

  void Reset()
  {
    syncPinMode = BUSY_PIN;
    syncDivisor = 0;
    
    stepMode = STEP_FS_128;
    
    maxSpeed = 500;
    minSpeed = 0;
    fullStepsSpeed = 500;
    acceleration = 1000;
    deceleration = 1000;
  
    slewRate = SR_520V_us;
  
    overCurrentThreshold = 20;
    overCurrentShutdown = OC_SD_DISABLE;
  
    pwmDivisor = PWM_DIV_1;
    pwmMultiplier = PWM_MUL_1;
  
    voltageCompensation = VS_COMP_DISABLE;
  
    switchMode = SW_HARD_STOP;
  
    clockSource = INT_16MHZ;
  
    runKval = 64;
    accelerationKval = 64;
    decelerationKval = 64;
    holdKval = 32;

    alarmEn = 0xEF;
  }
};


