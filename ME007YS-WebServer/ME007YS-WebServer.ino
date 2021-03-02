 /*
  * Based on
  * ========
  *   SCP1000 Barometric Pressure Sensor Display
  *   from Tom Igoe, 31 July 2010
  *
  * @File  : DFRobot_Distance_ME007YS.ino
  * @Brief : This example use ME007YS ultrasonic sensor to measure distance
  *         With initialization completed, We can get distance value
  * @Copyright [DFRobot](https://www.dfrobot.com),2016
  *            GUN Lesser General Pulic License
  * 
  */

#include <Ethernet.h>
#include <SoftwareSerial.h>

// assign a MAC address for the Ethernet controller.
// fill in your address here:
byte mac[] = {
  0xA8, 0x61, 0x0A, 0xAE, 0x81, 0x77
};
// assign an IP address for the controller: 192.168.1.41
IPAddress ip(192, 168, 1, 41);
IPAddress gw(192, 168, 1, 1);
IPAddress msk(255, 255, 255, 0);
IPAddress dns(195, 130, 130, 5);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

const int RXpin = 2;
const int TXpin = 3;
const int MUXpin = 5;

SoftwareSerial mySerial(RXpin, TXpin); // RX, TX

unsigned char data[4]={0, 0, 0, 0};
float distance[2] = {-1, -1};
int toggle = 0;

void setup() {
  pinMode(MUXpin, OUTPUT);
  digitalWrite(MUXpin, toggle);
  
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet connection
  Ethernet.begin(mac, ip, dns, gw, msk);
  // start the Serial Port to ME007YS  
  mySerial.begin(9600);
  mySerial.flush();

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  // give the sensor and Ethernet shield time to set up:
  delay(1000);
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start listening for clients
  server.begin();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("*** ME007YS_WebServer started ***");
}


static int idx = 0;
static int chk = 0x00;
static unsigned long tim0 = 0;
static unsigned long tim1;

void listenForEthernetClients() {
  // listen for incoming clients
  char  buf[256];
  int   ptr;

  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    ptr = 0;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

      /*
        if (c == '\n') Serial.println("\\n");
        else if (c == '\r') Serial.print("\\r");
        else Serial.print(c);
*/

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          if (ptr == 0) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            
            // print the current readings, in HTML format:
            client.print("0 - Distance: ");
            client.print(distance[0]/10);
            client.print("cm");
            client.println("<br />");
            
            client.print("1 - Distance: ");
            client.print(distance[1]/10);
            client.print("cm");
            client.println("<br />");

            break;
          } else {
            buf[ptr] = '\0';
            Serial.print("strcasestr - ");
            Serial.println(strcasestr(buf, "get"));
            Serial.println(buf);
            ptr = 0;
          }
        } else if (c != '\r') {
          // you've gotten a character on the current line
          buf[ptr++] = c;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

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
      /*
      Serial.print(" - distance=");
      Serial.print(distance[toggle]/10);
      Serial.println("cm");
      */
    }else {
      /*
      Serial.println(" - below the lower limit");
      */
    }
    
    tim0 = tim1;
    toggle = 1 - toggle;
    digitalWrite(MUXpin, toggle);
    mySerial.flush();
  }      
  
  idx = ++idx % 4;

  // listen for incoming Ethernet connections:
  listenForEthernetClients();
}