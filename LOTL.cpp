#include "LOTL.h"

namespace LOTL {

//Constructor
//Constructor defines how many variables of each type that the slave has
LOTLSerialSlave::LOTLSerialSlave(HardwareSerial &serialPort, const byte numBytes, const byte numShorts, const byte numInts, const byte numLongs, const byte numFloats, const byte numStrings) {
  this->slaveBytes = new byte[numBytes];
  this->slaveShorts = new short[numShorts];
  this->slaveInts = new int[numInts];
  this->slaveLongs = new long[numLongs];
  this->slaveFloats = new float[numFloats];
  this->slaveStrings = new char*[numStrings];

  this->slaveBytesLength = numBytes;
  this->slaveShortsLength = numShorts;
  this->slaveIntsLength = numInts;
  this->slaveFloatsLength = numFloats;
  this->slaveStringsLength = numStrings;

  this->port = &serialPort;
}


//Send and Receive
//Start character; command character; type character; index character; data; end character
byte LOTLSerialSlave::SendStartData(byte index, char typeChar) {
  //Send the start character
  this->port->write(this->startCommandChar);
  //Send the command type char
  this->port->write(this->responseCommandChar);
  //Send the data type character
  this->port->write(typeChar);
  //Send the index
  this->port->write(index);
  return 0;
}

byte LOTLSerialSlave::SendByte(byte index) {
  LOTLSerialSlave::SendStartData(index, byteTypeChar);
  //Send the data
  this->port->write(this->slaveBytes[index]);
  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::SendShort(byte index) {
  LOTLSerialSlave::SendStartData(index, shortTypeChar);
  //Send the data
  short data = this->slaveShorts[index];

  //Split the data into 2 bytes
  byte msb = (byte)(data >> 8);
  byte lsb = (byte)(data & 0x00FF);

  this->port->write(msb);
  this->port->write(lsb);

  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::SendInt(byte index) {
  LOTLSerialSlave::SendStartData(index, intTypeChar);
  //Send the data
  //Split the data into 8 bytes

  union u_IntToBytes {
    byte bytes[8];
    int intData;
  } u;

  u.intData = this->slaveInts[index];

  for (int i = 0; i < 4; i++) {
    this->port->write(u.bytes[i]);
  }

  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::SendLong(byte index) {
  LOTLSerialSlave::SendStartData(index, longTypeChar);
  //Send the data

  //Split the data into 8 bytes
  union u_longToBytes {
    byte bytes[8];
    long longData;
  } u;

  u.longData = this->slaveLongs[index];

  for (int i = 0; i < 8; i++) {
    this->port->write(u.bytes[i]);
  }

  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::SendFloat(byte index) {
  LOTLSerialSlave::SendStartData(index, floatTypeChar);
  //Send the data

  //Use a union to access the floats bytes
  union u_floatToBytes {
    byte bytes[4];
    float floatData;
  } u;

  u.floatData = this->slaveFloats[index];

  for (int i = 0; i < 4; i++) {
    this->port->write(u.bytes[i]);
  }

  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::SendString(byte index) {
  LOTLSerialSlave::SendStartData(index, stringTypeChar);
  //Send the data
  int i = 0;
  //Support up to 10 character strings
  while ((this->slaveStrings[index][i] != '\0') && (i < 9 )) {
    this->port->write(this->slaveStrings[index][i]);
    i++;
  }

  //If the index is 9 (10th character not sent) then send a null terminator to make sure string is terminated
  if (i == 9) this->port->write('\0');

  //Send the end character
  this->port->write(this->endCommandChar);
  return 0;
}

byte LOTLSerialSlave::ReceiveData(void) {
  if (this->state == 0) {
    //Checking for start character
    while (this->port->available()) {
      char c = this->port->read();
      //If a start character is received
      if (c == '<') {
        this->state = 1;
        break;
      }
    }
  }   //end of state 0 - check for start character if serial available

  if (this->state == 1) {
    //check for communication data
    //Only run once there are 3 bytes of serial data
    if (this->port->available() >= 3) {
      this->dataCommand = this->port->read();
      this->dataType = this->port->read();
      this->dataIndex = this->port->read();
      //Calculate how many bytes need to be read
      if (this->dataCommand == this->writeCommandChar) {
        if (this->dataType == this->byteTypeChar) this->bytesToRead = 1;
        else if (this->dataType == this->shortTypeChar) this->bytesToRead = 2;
        else if (this->dataType == this->intTypeChar) this->bytesToRead = 4;
        else if (this->dataType == this->longTypeChar) this->bytesToRead = 8;
        else if (this->dataType == this->floatTypeChar) this->bytesToRead = 4;
        else {
          //string and data frame not yet supported - error
          this->bytesToRead = 0;
        }

        //Change the state
        this->state = 2;
      }
      else if(this->dataCommand == this->readCommandChar){
        //Move to a response state
        this->state = 3;
      }
    }
  } //end of state 1 - read in command characters

  if (this->state == 2) {
    //Read in the data and write to register
    if(this->port->available() >= this->bytesToRead){
      //Read in the data
      byte data[this->bytesToRead];
      int i;
      for(i = 0; i < this->bytesToRead; i++){
        data[i] = this->port->read();
      }

      //Change the state when it exits
      this->state = 4;

      //Assemble the data and write it to the correct place
      if(this->dataType == this->byteTypeChar){
        if(slaveBytesLength <= this->dataIndex){
          this->slaveBytes[this->dataIndex] = data[0];
        }
        else{
          this->error = this->INDEX_OUT_OF_RANGE;
        }
      }
      else if(this->dataType == this->shortTypeChar){
        if(slaveShortsLength <= this->dataIndex){
          short assembledShort = ((short)(data[0]) << 8) | data[1];
          this->slaveShorts[this->dataIndex] = assembledShort;
        }
        else{
          this->error = this->INDEX_OUT_OF_RANGE;
        }
      }
      else if(this->dataType == this->intTypeChar){
        if(slaveIntsLength <= this->dataIndex){
          union u_BytesToInt{
            byte bytes[4];
            int assembledInt;
          } u;
          for (int j = 0; j < 4; j++){
            u.bytes[j] = data[j];
          }
          this->slaveInts[this->dataIndex] = u.assembledInt;
        }
        else{
          this->error = this->INDEX_OUT_OF_RANGE;
        }
      }
      else if(this->dataType == this->longTypeChar){
        if(slaveLongsLength <= this->dataIndex){
          union u_BytesToLong{
            byte bytes[8];
            long assembledLong;
          } u;
          for(int j; j < 8; j++){
            u.bytes[j] = data[j];
          }
          this->slaveLongs[this->dataIndex] = u.assembledLong;
        }
        else{
          this->error = this->INDEX_OUT_OF_RANGE;
        }
      }
      else if(this->dataType == this->floatTypeChar){
        if(slaveFloatsLength <= this->dataIndex){
          union u_BytesToFloat{
            byte bytes[4];
            float assembledFloat;
          } u;
          for(int j = 0; j < 4; j++){
            u.bytes[j] = data[j];
          }
          this->slaveFloats[this->dataIndex] = u.assembledFloat;
        }
        else{
          this->error = this->INDEX_OUT_OF_RANGE;
        }
      }
    }
  }   //end of state 2 - read in data for writing

  if(this->state == 3){
    //Respond with the required data to the request
    if(this->dataType == byteTypeChar){
      if(this->dataIndex <= this->slaveBytesLength){
        LOTLSerialSlave::SendByte(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else if(this->dataType == shortTypeChar){
      if(this->dataIndex <= this->slaveShortsLength){
        LOTLSerialSlave::SendShort(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else if(this->dataType == intTypeChar){
      if(this->dataIndex <= this->slaveIntsLength){
        LOTLSerialSlave::SendInt(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else if(this->dataType == longTypeChar){
      if(this->dataIndex <= this->slaveLongsLength){
        LOTLSerialSlave::SendLong(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else if(this->dataType == floatTypeChar){
      if(this->dataIndex <= this->slaveFloatsLength){
        LOTLSerialSlave::SendFloat(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else if(this->dataType == stringTypeChar){
      if(this->dataIndex <= this->slaveStringsLength){
        LOTLSerialSlave::SendString(this->dataIndex);
      }
      else{
        this->error = this->INDEX_OUT_OF_RANGE;
      }
    }
    else{
      this->error = INVALID_COMMAND;
    }
    this->state = 4;
  }
  if(state == 4){
    //Check for end of command character
    //May be unneeded
    //Flushes any extra data waiting for an end of command character
    while(this->port->available()){
      char c = this->port->read();
      if(c == this->endCommandChar){
        this->state = 0;
        break;
      }
    }
  }
  if(state > 4){
    //Undefined state 
    this->state = 0;
  }
}

//Called in the setup loop
//Have to call outside of the class constructor as there is an error caused as it is called too early
byte LOTLSerialSlave::begin(int baudRate){
  this->port->begin(baudRate);
  return 0;
}

//Getters and Setters
byte LOTLSerialSlave::GetByte(byte index) {
  if ((index < 0) | (index >= this->slaveBytesLength)) return 0;
  return this->slaveBytes[index];
}

short LOTLSerialSlave::GetShort(byte index) {
  if ((index < 0) | (index >= this->slaveShortsLength)) return 0;
  return this->slaveShorts[index];
}

int LOTLSerialSlave::GetInt(byte index) {
  if ((index < 0) | (index >= this->slaveIntsLength)) return 0;
  return this->slaveInts[index];
}

long LOTLSerialSlave::GetLong(byte index) {
  if ((index < 0) | (index >= this->slaveLongsLength)) return 0;
  return this->slaveLongs[index];
}

float LOTLSerialSlave::GetFloat(byte index) {
  if ((index < 0) | (index >= this->slaveFloatsLength)) return 0;
  return this->slaveFloats[index];
}

//Note this returns the actual string in memory - shallow copy
char* LOTLSerialSlave::GetString(byte index) {
  if ((index < 0) | (index > this->slaveStringsLength)) return 0;
  return this->slaveStrings[index];
}

byte LOTLSerialSlave::SetByte(byte index, byte data) {
  if ((index < 0) | (index > this->slaveBytesLength)) return 1;
  this->slaveBytes[index] = data;
  return 0;
}

byte LOTLSerialSlave::SetShort(byte index, short data) {
  if ((index < 0) | (index > this->slaveShortsLength)) return 1;
  this->slaveShorts[index] = data;
  return 0;
}

byte LOTLSerialSlave::SetInt(byte index, int data) {
  if ((index < 0) | (index > this->slaveIntsLength)) return 1;
  this->slaveInts[index] = data;
  return 0;
}

byte LOTLSerialSlave::SetLong(byte index, long data) {
  if ((index < 0) | (index > this->slaveLongsLength)) return 1;
  this->slaveLongs[index] = data;
  return 0;
}

byte LOTLSerialSlave::SetFloat(byte index, float data) {
  if ((index < 0) | (index > this->slaveFloatsLength)) return 1;
  this->slaveFloats[index] = data;
  return 0;
}

//Note: the string is shallow copied
byte LOTLSerialSlave::SetString(byte index, char* data) {
  if ((index < 0) | (index > this->slaveStringsLength)) return 1;
  this->slaveStrings[index] = data;
  return 0;
}


} //end of namespace
