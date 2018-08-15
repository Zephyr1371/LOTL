/*  Name: Physical layer wrapper class
    Author: Zach Warner
    Date: 12 May 2018

    Purpose:  This class is part of the Low Overhead Transport Layer library which allows for reading, writing, and requesting data from a device.
    This class is a wrapper class for the Serial functions in Arduino. The wraper class can be changed to other serial classes if used
    on different devices
*/
#include "Arduino.h"


namespace LOTL {
class SerialPhysicalLayer {
  //Private Variables
  private:
  //Holds the port used for communication
  HardwareSerial* port;

  public:

  //Constructor
  SerialPhysicalLayer(HardwareSerial &serialPort);
  SerialPhysicalLayer(HardwareSerial &serialPort, int baudRate);

}
}  //End of namespace

