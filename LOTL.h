/* This class is the Low Overhead Transport Layer
    It is meant to be used on a serial port to abstract
    individual variable data transfer.

    The protocol can send:
    Bytes , Booleans
    Shorts , Unsigned shorts
    Ints , Unsigned Ints
    Longs , Unsigned longs
    Floats
    Strings[up to 8 chars], Strings[up to 24chars]
    A data frame up to 255 bytes

    Currently the secondary variable type (unsigned) is not supported

    It can address up to 256 different variables of each type
    as long as the slave supports this. It runs on a master-slave configuration
    where the slave device has requests sent to it from the master which either
    set a variable or request the variable's value.
    The variables can store either the signed or unsigned version of the data
    under each variable array ( for memory usage)
    There are a few other requests that fall outside this such as requesting
    the variables name or units for presentation to the master.

    The slave can initialize itself by sending the master data on what variable
    fields it contains and any meta data about these fields such as name or units

    //Use 
*/

#ifndef _LOTL_H_
#define _LOTL_H_

#include "Arduino.h"


namespace LOTL {

class LOTLSerialSlave {
private:
    //Private variables
    HardwareSerial* port;

    //The variables which hold the last received data and state machine variables
    byte state = 0;
    byte dataIndex = 0;
    byte dataType = 0;
    byte dataCommand = 0;
    byte bytesToRead = 0;

    //Errors and codes
    byte error = 0;
    byte INDEX_OUT_OF_RANGE = 1;
    byte INVALID_COMMAND = 2;
    
    //The slaves bytes
    byte* slaveBytes;
    byte slaveBytesLength;

    //The slaves shorts
    short* slaveShorts;
    byte slaveShortsLength;
  
    //The slaves ints
    int* slaveInts;
    byte slaveIntsLength;

    //The slaves longs
    long* slaveLongs;
    byte slaveLongsLength;

    //The slaves floats
    float* slaveFloats;
    byte slaveFloatsLength;

    //The slaves strings
    char** slaveStrings;
    byte slaveStringsLength;

    //Data Type characters
    const char byteTypeChar = 'B';
    const char shortTypeChar = 'S';
    const char intTypeChar = 'I';
    const char longTypeChar = 'L';
    const char floatTypeChar = 'F';
    const char stringTypeChar = 's';
    const int frameTypeChar = 'f'; 

    //Command characters
    const char startCommandChar = '<';
    const char endCommandChar = '\n';
    const char writeCommandChar = ':';
    const char readCommandChar = '?';
    const char responseCommandChar = '!';
    const char errorCommandChar = '#';

public:

    //Constructor
    //Constructor defines how many variables of each type that the slave has
    LOTLSerialSlave(HardwareSerial &serialPort, const byte numBytes, const byte numShorts, const byte numInts, const byte numLongs, const byte numFloats, const byte numStrings);

    //Send and Receive
    byte SendStartData(byte index, char typeChar);
    byte SendByte(byte index);
    byte SendShort(byte index);
    byte SendInt(byte index);
    byte SendLong(byte index);
    byte SendFloat(byte index);
    byte SendString(byte index);
    byte SendFrame(byte len, byte* data);

    byte ReceiveData(void);

    //Public Methods
    byte begin(int baudRate);

    //Getters and Setters
    byte GetByte(byte index);
    short GetShort(byte index);
    int GetInt(byte index);
    long GetLong(byte index);
    float GetFloat(byte index);
    char* GetString(byte index);

    byte SetByte(byte index, byte data);
    byte SetShort(byte index, short data);
    byte SetInt(byte index, int data);
    byte SetLong(byte index, long data);
    byte SetFloat(byte index, float data);
    byte SetString(byte index, char* data);
    
};
}//End of namespace

#endif
