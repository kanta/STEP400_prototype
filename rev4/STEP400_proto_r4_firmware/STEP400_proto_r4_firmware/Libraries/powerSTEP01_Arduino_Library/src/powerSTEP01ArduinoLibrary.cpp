#include <SPI.h>
#include "powerSTEP01ArduinoLibrary.h"

int powerSTEP::_numBoards;

// Constructors
powerSTEP::powerSTEP(int position, int CSPin, int resetPin, int busyPin)
{
  _CSPin = CSPin;
  _position = position;
  _resetPin = resetPin;
  _busyPin = busyPin;
  _numBoards++;
  _SPI = &SPI;
}

powerSTEP::powerSTEP(int position, int CSPin, int resetPin)
{
  _CSPin = CSPin;
  _position = position;
  _resetPin = resetPin;
  _busyPin = -1;
  _numBoards++;
  _SPI = &SPI;
}

void powerSTEP::SPIPortConnect(SPIClass *SPIPort)
{
  _SPI = SPIPort;
}

int powerSTEP::busyCheck(void)
{
  if (_busyPin == -1)
  {
    if (getParam(STATUS) & 0x0002) return 0;
    else                           return 1;
  }
  else 
  {
    if (digitalRead(_busyPin) == HIGH) return 0;
    else                               return 1;
  }
}
