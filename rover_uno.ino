
#include <SoftwareSerial.h>

SoftwareSerial link(11, 12); 


const uint8_t ENA = 5, IN1 = 7, IN2 = 8;  
const uint8_t ENB = 6, IN3 = 9, IN4 = 10; 

const uint8_t TRIG = 3, ECHO = 4;

const int STOP_DIST = 20;  
const int AVOID_DIST = 30;  
const unsigned long FAILSAFE_MS = 700;  

int speedPwm = 180;
char cmd = 'S';
bool autoMode = false;
long dist = 999;
unsigned long lastCmd = 0, lastSend = 0;

void drive(int l, int r) { 
  digitalWrite(IN1, l > 0); digitalWrite(IN2, l < 0); analogWrite(ENA, abs(l));
  digitalWrite(IN3, r > 0); digitalWrite(IN4, r < 0); analogWrite(ENB, abs(r));
}

long readDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long d = pulseIn(ECHO, HIGH, 25000UL);  
  if (d == 0) return 999;
  return d * 0.034 / 2;
}

void setup() {
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  link.begin(9600);
  Serial.begin(9600);
  drive(0, 0);
}

void handleManual() {
  switch (cmd) {
    case 'F': if (dist < STOP_DIST) drive(0, 0); else drive(speedPwm, speedPwm); break;
    case 'B': drive(-speedPwm, -speedPwm); break;
    case 'L': drive(-speedPwm, speedPwm); break;  
    case 'R': drive(speedPwm, -speedPwm); break;   
    default:  drive(0, 0);
  }
}

void handleAuto() {
  if (dist > AVOID_DIST) {
    drive(speedPwm, speedPwm);
  } else {
    drive(0, 0); delay(150);
    drive(-speedPwm, -speedPwm); delay(350);       
    if (random(2)) drive(speedPwm, -speedPwm);    
    else drive(-speedPwm, speedPwm);
    delay(300 + random(300));
    drive(0, 0);
  }
}

void loop() {
  while (link.available()) {
    char c = link.read();
    if (c >= '1' && c <= '9') speedPwm = map(c - '0', 1, 9, 90, 255);
    else if (c == 'A') { autoMode = true; }
    else if (c == 'M') { autoMode = false; cmd = 'S'; }
    else if (c == 'F' || c == 'B' || c == 'L' || c == 'R' || c == 'S') { cmd = c; }
    lastCmd = millis();
  }

  dist = readDistance();

  if (autoMode) handleAuto();
  else if (millis() - lastCmd > FAILSAFE_MS) drive(0, 0); 
  else handleManual();

  if (millis() - lastSend > 300) {  
    link.print("D:"); link.println(dist);
    lastSend = millis();
  }
}
