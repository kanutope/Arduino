const int RX1 = 2;
const int TX1 = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(RX1, INPUT);
  pinMode(TX1, OUTPUT);
  digitalWrite(TX1, HIGH);

  Serial.begin(57600);
}

// about 2 museconds / loop.
// 9600 baud => 1bit / 100 musec; 2 stopbits = 200 musec;
// 1bit = 104,17 musec;

unsigned long timWAIT = -1;

int state = 0;
int WAIT = 0;
int RDY = 1;

void loop() {
  unsigned long tim;
  int lvl;

  lvl = digitalRead(RX1);
  if ((lvl == HIGH) && (state == WAIT)) {
    if (timWAIT == -1) {
      timWAIT = micros();  
    } else {
      tim = micros();
      if ((tim - timWAIT) > 200) {
        state = RDY;
      }
    }
  }
  
  delayMicroseconds(20);
}
