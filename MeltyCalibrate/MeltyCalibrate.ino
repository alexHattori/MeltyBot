#include <EEPROM.h>
#include <PinChangeInt.h>
#include <Servo.h>

//Transmitter Stuff
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

int pins[] = {chan1, chan2, chan3};
long mins[] = {chan1Min, chan2Min, chan3Min};
long maxs[] = {chan1Max, chan2Max, chan3Max};
long neuts[] = {chan1N, chan2N, chan3N};

volatile long dutyCycles[] = {lastDC1, lastDC2, lastDC3};
volatile long startTime[] = {riseStart1, riseStart2, riseStart3};
volatile int lastReading[] = {LOW, LOW, LOW};

//Transmitter End

Servo output;

float radius = .0321437f;        //radius in m
float angle;                //CurrentAngle
long lastRead;              //Last time (millis) math was done

float runningAverage;       //RunningAverage for accelerometer reading
int numAvg = 4;             //Number of times to sample accelerometer

bool toggle;                //For calibration
bool toggleEnd;

int numOffs;                //Number of offsets
float incr = 0.001f;          //Offset increment

float maxSpeed;
//Transmitter Reading
void chanISR() {
  for (int i = 0; i < numChannels; i++) {
    int reading = digitalRead(pins[i]);
    if (reading < lastReading[i]) {
      dutyCycles[i] = micros() - startTime[i];
      lastReading[i] = reading;
      break;
    }
    else if (reading > lastReading[i]) {
      startTime[i] = micros();
      lastReading[i] = reading;
      break;
    }
    lastReading[i] = reading;
  }
}
double readTransmitter(int pinNumber) {
  double diff = maxs[pinNumber] - mins[pinNumber];
  double transVal = (double)(dutyCycles[pinNumber] - neuts[pinNumber]) / diff * 2.0;

  if (transVal > 1) {
    transVal = 1.0;
  }
  if (transVal < -1) {
    transVal = -1;
  }
  if (abs(transVal) < 0.05) {
    transVal = 0;
  }
  return transVal;
}
int convertToAnalog(double value) {
  value += 1;
  value *= 500;
  value += 1000;

  if (value > 2000) {
    value = 2000;
  }
  if (value < 1200) {
    value = 900;
  }
  return (int)value;
}
int convertToAnalog(int pinNumber) {
  return convertToAnalog(readTransmitter(pinNumber));
}

//Accelerometer Reading
double captureAccel() {
  int sum = 0;
  for (int i = 0; i <numAvg; i++) {
    sum += analogRead(A0);
    delay(20);
  }
  float avg = (float)sum / (float)numAvg;
  
  if (runningAverage == 0.0f) {
    runningAverage = avg;
  }
  else {
    runningAverage = runningAverage * 0.6f + avg*0.4f;
  }
  }

void setup() {
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  for (int i = 0; i < numChannels; i++) {
    dutyCycles[i] = 0;
    startTime[i] = micros();
    attachPinChangeInterrupt(pins[i], chanISR, CHANGE);
  }
  Serial.begin(9600);
  output.attach(3);
  angle = 0.0f;
  lastRead = millis();

  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(7, LOW);

  runningAverage = 0.0f;
  
  toggle  = false;
  toggleEnd = false;
  numOffs = 0;

  maxSpeed = 0.0f;
}

void loop() {
  //Debugging Channels
  //  Serial.print("Chan1: ");
  //  Serial.print(dutyCycles[0]);
  //  Serial.print(" Chan2: ");
  //  Serial.print(dutyCycles[1]);
  //  Serial.print(" Chan3: ");
  //  Serial.println(dutyCycles[2]);

  //FailSafe
  for (int i = 0; i < numChannels; i++) {
    long curTime = micros();
    if (curTime - startTime[i] > 140000) {
      dutyCycles[i] = (long)900;
    }
  }

  //Using Transmitter Info
  int valPut = convertToAnalog(0);   //Throttle Signal
  if (convertToAnalog(1) > 1600) {
    toggle = true;
  }
  if (convertToAnalog(1) < 1600 && toggle == true) {
    toggleEnd = true;
    toggle = false;
  }
  if (toggleEnd) {
    numOffs++;
    radius += incr;
    toggleEnd = false;
    toggle = false;
    EEPROM.write(5, numOffs);
  }
  if (valPut > 1500) {
    valPut = 1500;
  }
  output.writeMicroseconds(valPut);


  double averaged = captureAccel(); //Averaged ADC read value

  float accel = (float)abs((averaged - 503)) *0.5f;          //Acceleration in gs
  float acc = accel*9.8f;
  float curSpeed = sqrt(acc/radius);

  Serial.println(angle);
 
  long curTime = millis();
  long timeSinceLast = (curTime - lastRead);
  
  angle += curSpeed * timeSinceLast*.001f;

  lastRead = curTime;
  
  while (angle > 2 * 3.141592653589793) {
    angle -= 2 * 3.141592653589793;
  }
  
  if (angle > 0.0 && angle < 2.09439510239f) {
    digitalWrite(8, HIGH);
  }
  if (angle >= 2.09439510239f) {
    digitalWrite(8, LOW);
  }

}

