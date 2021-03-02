    /*
    *@File  : DFRobot_Distance_ME007YS.ino
    *@Brief : This example use ME007YS ultrasonic sensor to measure distance
    *         With initialization completed, We can get distance value
    *@Copyright [DFRobot](https://www.dfrobot.com),2016
    *           GUN Lesser General Pulic License
    *@version V1.0
    *@data    2019-08-28
    *
    *@Copyright [pawawa]
    *           GNU Lesser General Public License
    *@version v2.0
    *@date    2021-02-27
    */
#include <SoftwareSerial.h>

const int RXpin = 2;
const int TXpin = 3;
const int MUXpin = 5;

SoftwareSerial mySerial(RXpin, TXpin); // RX, TX

unsigned char data[4]={};
float distance[2] = {-1, -1};
int toggle = 0;

void setup()
{
  pinMode(MUXpin, OUTPUT);
  digitalWrite(MUXpin, toggle);
  
  // start the Serial Port to ME007YS  
  mySerial.begin(9600);
  mySerial.flush();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("*** ME007YS_v2 started ***");
}

int idx = 0;
int chk = 0x00;
unsigned long tim0 = 0;
unsigned long tim1;

 /*
  * read 1 byte per each loop.As soon as this latest byte equals the checksum
  * of the three previous bytes, a valid reading is done.
  */
void loop() {
  data[idx] = mySerial.read();

  if (data[(idx+1)%4] == 0xff) {
    chk=(data[(idx+1)%4] + data[(idx+2)%4]+data[(idx+3)%4]) & 0x00FF;
    
    if (data[idx] == chk) {
      mySerial.flush();

      distance[toggle] = (data[(idx+2)%4]<<8)+data[(idx+3)%4];
    }
  }

  tim1 = millis();
  if ((tim1-tim0) > 1000) {
    Serial.print(toggle);
    if(distance[toggle] > 280) {
      Serial.print(" - distance=");
      Serial.print(distance[toggle]/10);
      Serial.println("cm");
    }else {
      Serial.println(" - below the lower limit");
    }
    
    tim0 = tim1;
    toggle = 1 - toggle;
    digitalWrite(MUXpin, toggle);
    mySerial.flush();
  }      
  
  idx = ++idx % 4;
}
