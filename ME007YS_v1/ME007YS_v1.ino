    /*
    *@File  : DFRobot_Distance_ME007YS.ino
    *@Brief : This example use ME007YS ultrasonic sensor to measure distance
    *         With initialization completed, We can get distance value
    *@Copyright [DFRobot](https://www.dfrobot.com),2016
    *           GUN Lesser General Pulic License
    *@version V1.0
    *@data  2019-8-28
    */
#include <SoftwareSerial.h>

const int RXpin = 11
const int TXpin = 10
const int CTRLpin = 13

SoftwareSerial mySerial(RXpin, TXpin); // RX, TX

unsigned char data[4]={};
float distance;

void setup()
{
    pinMode(CTRLpin, OUTPUT);
    digitalWrite(CTRLpin, HIGH);
    
    Serial.begin(57600);
    mySerial.begin(9600);
}
  
void loop() {
    do {
        for(int i=0;i<4;i++) {
            data[i]=mySerial.read();
        }
    } while (mySerial.read()==0xff);

    mySerial.flush();
    
    if(data[0]==0xff) {
        int sum;
        sum=(data[0]+data[1]+data[2])&0x00FF;
        
        if(sum==data[3]) {
            distance=(data[1]<<8)+data[2];
            if(distance>280) {
                Serial.print("distance=");
                Serial.print(distance/10);
                Serial.println("cm");
            }else {
                Serial.println("Below the lower limit");
            }
        }else Serial.println("ERROR");
    }
    delay(100);
}
