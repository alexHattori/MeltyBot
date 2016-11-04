#include <EEPROM.h>
#include <PinChangeInt.h>
#include <Servo.h>


#define chan1 A1
#define chan2 A2
#define chan3 A3

#define chan1Min 1100
#define chan2Min 1192
#define chan3Min 1208

#define chan1Max 1908
#define chan2Max 1808
#define chan3Max 1804

#define chan1N 1332
#define chan2N 1500
#define chan3N 1496

#define numChannels 3

volatile long lastDC1;
volatile long riseStart1;

volatile long lastDC2;
volatile long riseStart2;

volatile long lastDC3;
volatile long riseStart3;

int pins[] = {chan1,chan2,chan3};
long mins[] = {chan1Min,chan2Min,chan3Min};
long maxs[] = {chan1Max,chan2Max,chan3Max};
long neuts[] = {chan1N,chan2N,chan3N};

volatile long dutyCycles[] = {lastDC1,lastDC2,lastDC3};
volatile long startTime[] = {riseStart1,riseStart2,riseStart3};
volatile int lastReading[] = {LOW,LOW,LOW};

Servo output;

float angle;
long lastBlink;

bool toggle;
bool toggleEnd;

void chanISR(){
  for(int i = 0; i<numChannels; i++){
    int reading = digitalRead(pins[i]);
    if(reading<lastReading[i]){
      dutyCycles[i] = micros()-startTime[i];
      lastReading[i] = reading;
      break;
    }
    else if(reading>lastReading[i]){
      startTime[i] = micros();
      lastReading[i] = reading;
      break;
    }
    lastReading[i] = reading;
  }
}
double readTransmitter(int pinNumber){
    double diff = maxs[pinNumber]-mins[pinNumber];
    double transVal = (double)(dutyCycles[pinNumber]-neuts[pinNumber])/diff*2.0;

    if(transVal>1){
      transVal = 1.0;
    }
    if(transVal<-1){
      transVal = -1;
    }
    if(abs(transVal)<0.05){
      transVal = 0;
    }
    return transVal;
}
int convertToAnalog(double value){
  value += 1;
  value*=500;
  value+=1000;

  if(value>2000){
    value = 2000;
  }
  if(value<1200){
    value = 900;
  }
  return (int)value;
}
int convertToAnalog(int pinNumber){
  return convertToAnalog(readTransmitter(pinNumber));
}

void setup() {
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  digitalWrite(11,HIGH);
  digitalWrite(12,LOW);
  for(int i = 0; i<numChannels; i++){
    dutyCycles[i] = 0;
    startTime[i] = micros();
    attachPinChangeInterrupt(pins[i],chanISR,CHANGE);
  }
  Serial.begin(9600);
  output.attach(3);
  angle = 0.0f;
  lastBlink = micros();

  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  digitalWrite(7,LOW);

  toggle  = false;
  toggleEnd = false;
}

void loop() {
  //Debugging Channels
//  Serial.print("Chan1: ");
//  Serial.print(dutyCycles[0]);
//  Serial.print(" Chan2: ");
//  Serial.print(dutyCycles[1]);
//  Serial.print(" Chan3: ");
//  Serial.println(dutyCycles[2]);

  for(int i = 0; i<numChannels;i++){
    long curTime = micros();
    if(curTime-startTime[i]>140000){
      dutyCycles[i] = (long)900;
    }
  }
  int valPut = convertToAnalog(0);
  if(convertToAnalog(1)>1600){
    toggle = true;
  }
  if(convertToAnalog(1)<1600&&toggle == true){
    toggleEnd = true;
    toggle = false;
  }
  if(toggleEnd){
    toggleEnd = false;
    toggle = false;
    EEPROM.write(1,analogRead(A0)/4);
    EEPROM.write(2,valPut/5);
  }
  output.writeMicroseconds(valPut);
  
  float curSpeed = 2000.0f*2.0f*3.141592653589793238462643f*60.0f;
  long curTime = micros();
  long timeSinceLast = (micros()-lastBlink);
   angle+=curSpeed*timeSinceLast*.000001f;
   Serial.println(angle);
   while(angle>2*3.141592653589793){
    angle-=2*3.141592653589793;
   }
   lastBlink = curTime;
   if(angle>0.0 && angle<0.174533){
      digitalWrite(8,HIGH);
   }
   if(angle>=0.174533 && angle<0.349066){
      digitalWrite(8,LOW);
   }
  
  }

