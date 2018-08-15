#include "Arduino.h"
#include "LOTL.h"
#include "TimerOne.h"

LOTL::LOTLSerialSlave slave = LOTL::LOTLSerialSlave(Serial, 5, 5, 5, 1, 5, 1);

void setup() {
  Timer1.initialize(10000);             //Run the interrupt every 5 ms
  Timer1.attachInterrupt(LOTL_ISR);    //Attach the received interrupt function

  //Init the slave device and open the serial port
  slave.begin(9600);  

  FillInDataTest();
}

void loop() {
  //ByteWriteTest();
  //slave.SendByte(0);
}

void LOTL_ISR(void){
  slave.ReceiveData();
}

void ByteWriteTest(void){
  int i;
  for(i = 0 ; i < 10; i++){
    slave.SetByte(0, i+'A');
    slave.SetByte(1, 9-i + 'A');
    delay(500);
  }
}

void FillInDataTest(void){
  slave.SetByte(0, 'H');
  slave.SetByte(1, 'e');
  slave.SetByte(2, 'l');
  slave.SetByte(3, 'l');
  slave.SetByte(4, 'o');

  slave.SetShort(0, 1);
  slave.SetShort(1, 10);
  slave.SetShort(2, 255);
  slave.SetShort(3, 1023);
  slave.SetShort(4, -1);

  slave.SetInt(0, 1);
  slave.SetInt(1, 255);
  slave.SetInt(2, 1023);
  slave.SetInt(3, 0);
  slave.SetInt(4, -1);

  slave.SetLong(0, 1000000);

  slave.SetFloat(0, 1);
  slave.SetFloat(1, 1.5);
  slave.SetFloat(2, 0);
  slave.SetFloat(3, 1000);
  slave.SetFloat(4, -1);

  slave.SetString(0, "World");
}

