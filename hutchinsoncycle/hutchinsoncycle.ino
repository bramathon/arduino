#include <PID_v1.h>

int cycles = 0;
int ssrPin = 5;
double soakTemp = 340;
double roomTemp = 25;

double Setpoint, Input, Output;

PID myPID(&Input, &Output, &Setpoint,0.2,0.8,0.05,DIRECT);

//ALL TIMES IN MS

int WindowSize = 500;
unsigned long cycleStart;
unsigned long cycleTime;
unsigned long rampTime = 300000;  //5 minutes
unsigned long soakTime = 300000; //5 minutes

unsigned long windowStartTime;

int debug = 0;
void setup() {
  // put your setup code here, to run once:
  if(debug) Serial.begin(115200);
  pinMode(ssrPin,OUTPUT);
  pinMode(13,OUTPUT);
  Setpoint = 260;
  myPID.SetOutputLimits(0,WindowSize);
  myPID.SetMode(AUTOMATIC);
  cycleStart = millis();
  windowStartTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  cycleTime = millis()-cycleStart;
  
  if (cycleTime < rampTime)  {
    Setpoint = roomTemp + (soakTemp-roomTemp)*cycleTime/rampTime;
    /* The setpoint starts at 0, then increase linearly
    as a function of the ratio of the rampTime
    */}
  else if (cycleTime < (rampTime + soakTime))  {
    Setpoint = soakTemp;
  }
  else if (cycleTime < (rampTime + soakTime + rampTime))  {
    float ratio = (float)(cycleTime-(rampTime+soakTime))/(float)rampTime;
    Setpoint = roomTemp + (soakTemp-roomTemp)*(1.0-ratio);
  }
  else  {
    //reset
    Setpoint = roomTemp;
    cycleStart = millis();
    cycles = cycles + 1;
  }
  myPID.Compute();
  
  double celcius = analogRead(A2)/2.046;
  Input = celcius;
  
  unsigned long now = millis();
  if(now - windowStartTime>WindowSize){
    windowStartTime += WindowSize;
  }
  
  if(Output > (now - windowStartTime)) {
    digitalWrite(ssrPin,HIGH);
    digitalWrite(13,HIGH);
  }
  else {
    digitalWrite(ssrPin,LOW);
    digitalWrite(13,LOW);
  }
  if(debug)  {
  Serial.print("Temperature:  ");
  Serial.print(celcius);
  Serial.print("  Setpoint:  ");
  Serial.print(Setpoint);
  Serial.print("  Output:  ");
  Serial.println(Output);
  }
  delay(20);

}
