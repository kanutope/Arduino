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
IPAddress ip(172, 16, 1, 41);
IPAddress gw(172, 16, 1, 1);
IPAddress msk(255, 255, 255, 0);
IPAddress dns(195, 130, 130, 5);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

const int RXpin = 2;
const int TXpin = 3;
const int MUXpin = 5;
const int DBGpin = 6;

int DBG = 0;
#ifndef		dbgPrint
#define   dbgPrint(str)     if (DBG) Serial.print(str)
#define   dbgPrintln(str)   if (DBG) Serial.println(str)
#endif

SoftwareSerial mySerial(RXpin, TXpin); // RX, TX

unsigned char data[4]={0xAA, 0xAA, 0xAA, 0xAA};
float distance[2] = {-1, -1};
int toggle = 0;

void setup() {
  pinMode(MUXpin, OUTPUT);
  digitalWrite(MUXpin, toggle);

  pinMode(DBGpin, OUTPUT);
  digitalWrite(DBGpin, HIGH);     // force it HIGH
  pinMode(DBGpin, INPUT_PULLUP);
  DBG = digitalRead(DBGpin);

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
    dbgPrintln("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  // give the sensor and Ethernet shield time to set up:
  delay(1000);

  if (Ethernet.linkStatus() == LinkOFF) {
    dbgPrintln("Ethernet cable is not connected.");
  }

  // start listening for clients
  server.begin();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  dbgPrintln("*** ME007YS_WebServer started ***");
}


int idx = 0;
int chk = 0x00;
unsigned long tim0 = 0;
unsigned long tim1;

void listenForEthernetClients() {
  // listen for incoming clients
  const int depth = 8;
  char      *nil = (char *) "";

  char    buf[256];
  char    *cptr, *dup = (char *) NULL;
  char    *path[depth];
  int     i, iptr;


  for (i = 0; i < depth; ++i) path[i] = nil;

  EthernetClient client = server.available();

  if (client) {
    // an http request ends with a blank line
    iptr = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          if (iptr == 0) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();

            // print the current readings, in HTML format:
            if (strcmp(path[1], "waterput") == 0) {
              if (strcmp(path[2], "diepte") == 0) {
                i = atoi(path[3]);
                if ((0 < i) && (i< 3)) {
                  if (strcmp(path[0], "json") == 0) {
                    client.print("{ \"Diepte");
                    client.print(i);
                    client.print("\": ");
                    client.print(distance[i-1]/10);
                    client.println("}");
                  } else if (strcmp(path[0], "xml") == 0) {
                    client.println("<?xml version=\"1.0\" encoding=\"utf-8\"?>");

                    client.print("<LL control=\"");
                    for (int j = 0; j < 4; ++j) {
                      client.print("/");
                      client.print(path[j]);
                    }
                    
                    client.print("\" value=\"");
                    client.print(distance[i-1]/10);
                    client.println("\" Code=\"200\"/>");
                  } else {
                    client.println("<h1>Ongeldige protocol</h1>");
                    client.println("<br>Alleen JSON of XML wordt (momenteel) ondersteund.");
                  }
                } else {
                  client.println("<h1>Ongeldige index</h1>");
                  client.println("<br>Alleen '1' of '2' wordt (momenteel) ondersteund.");
                }
              } else {
                client.println("<h1>Ongeldige parameter</h1>");
                client.println("<br>Alleen 'diepte' wordt (momenteel) ondersteund.");
              }
            } else {
              client.println("<h1>Ongeldige unit</h1>");
              client.println("<br>Alleen 'waterput wordt (momenteel) ondersteund.");
            }

            break;
          } else {
            buf[iptr] = '\0';
            cptr = strcasestr(buf, "get ");

            if (cptr != (char *) NULL) {
              cptr = strchr(cptr, '/');       // start of URI
              *strchr(cptr, ' ') = '\0';      // null terminate URI

              i = 0;
              cptr = strtok((dup = strdup(cptr)), "/");
              while (cptr != (char *) NULL)  {
                path[i++] = cptr;
                cptr = strtok(NULL, "/");
              }
            }

            iptr = 0;
          }
        } else if (c != '\r') {
          // you've gotten a character on the current line
          buf[iptr++] = c;
        }
      }
    }

    free(dup);      // if 'ptr' is NULL, no operation is performed.
    delay(1);       // give the web browser time to receive the data
    client.stop();  // close the connection:
  }
}

unsigned long PERIOD = 2000;
 /*
  * read 1 byte per each loop.As soon as this latest byte equals the checksum
  * of the three previous bytes, a valid reading is done.
  */
 void loop() {
  DBG = digitalRead(DBGpin);

  data[idx] = mySerial.read();
  if (data[(idx+1)%4] == 0xff) {
    chk=(data[(idx+1)%4] + data[(idx+2)%4]+data[(idx+3)%4]) & 0x00FF;

    if (data[idx] == chk) {
      mySerial.flush();

      distance[toggle] = (data[(idx+2)%4]<<8)+data[(idx+3)%4];
    }
  }

  tim1 = millis();
  if ((tim1-tim0) > PERIOD) {
    if(distance[toggle] > 280) {
      dbgPrint(" - distance=");
      dbgPrint(distance[toggle]/10);
      dbgPrintln("cm");
    }else {
      dbgPrintln(" - below the lower limit");
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