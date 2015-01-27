#include <PID_v1.h>
#include <FileIO.h>
#include <Process.h>

const double e = 2.718281828;
const double pi = 3.1415926;

/////////////////
// Phant Stuff //
/////////////////
// URL to phant server (only change if you're not using data.sparkfun
String phantURL = "https://data.sparkfun.com/input/";
// Public key (the one you see in the URL):
String publicKey = "YGv5Xz8d2ZsONZa4LX2x";
// Private key, which only someone posting to the stream knows
String privateKey = "Rb8mwgoBlNTXzlZpGgmd";
// How many data fields are in your stream?
const int NUM_FIELDS = 6;
// What are the names of your fields?
//http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]¤t=[value]&power=[value]&resistance=[value]&setpoint=[value]&temperature=[value]&time=[value]&voltage=[value]
String fieldName[NUM_FIELDS] = {"curr","resistance","setpt","temp","time","volt"};
// We'll use this array later to store our field data
String fieldData[NUM_FIELDS];

int cycles = 0;
int pwrPin = 3;
int point = 0;
double soakTemp = 260;
double roomTemp = 25;
double current;
double power;
double resistance;
double voltage;
double piTime = 0 ;

double Setpoint, Input, Output;
// tuning, try different kd
// kd = 0.2, 0.4, 0.6, 0.8, 1.0, 1.2
//double kd = {0.2, 0.4, 0.6, 0.8, 1.0, 1.2};

PID myPID(&Input, &Output, &Setpoint,0.2,0.8,0.0,DIRECT);

//ALL TIMES IN MS

unsigned long cycleStart;
unsigned long cycleTime;
unsigned long rampTime = 120000;  //2 minutes
unsigned long soakTime = 120000; //2 minutes
unsigned long coolTime = 1200000; //20 minutes
//add a more reasonable cool time

int debug = 1;
int phant = 0;
void setup() {
  delay(30000);
  // put your setup code here, to run once:
  Bridge.begin();
  if(debug) Serial.begin(115200);
  FileSystem.begin();
  pinMode(pwrPin,OUTPUT);
  pinMode(13,OUTPUT);
  analogReference(EXTERNAL);
  Setpoint = 25;
  myPID.SetOutputLimits(0,650.0); //output in units of (255^2)/100 (div by 100 for scale)
  myPID.SetMode(AUTOMATIC);
  cycleStart = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  cycleTime = millis()-cycleStart;
  
  if (cycleTime <= rampTime)  {
    // updated this to use a tanh ramp function
    //Setpoint = roomTemp + (soakTemp-roomTemp)*cycleTime/rampTime;
    piTime = (float(cycleTime)/float(rampTime))*-4.0*pi+2.0*pi;
    Setpoint = roomTemp + (soakTemp-roomTemp)*(1.0/(1.0+pow(e,piTime)));
    /* The setpoint starts at 0, then increase linearly
    as a function of the ratio of the rampTime
    */}
  else if (cycleTime < (rampTime + soakTime))  {
    Setpoint = soakTemp;
  }
  else if (cycleTime < (rampTime + soakTime + rampTime))  {
    float ratio = (float)(cycleTime-(rampTime+soakTime))/(float)rampTime;
    Setpoint = roomTemp + (soakTemp-roomTemp)*(1.0-ratio);
    //ratio of current ramp = (current time in cycle - ramptime+soaktime)/ramptime
    //Setpoint = bottom temp + (top temp - bot temp)*(1-ratio)
  }
  else if (cycleTime < (2*rampTime + soakTime + coolTime))  {
    //additional time for a cool cycle
    Setpoint = roomTemp;
  }
  else  {
    //reset
    Setpoint = roomTemp;
    cycleStart = millis();
    cycles = cycles + 1;
    point = 0; //reset the point counter because why not.
  }
  myPID.Compute();
  
  double celcius = readRTD(5);
  Input = celcius;
  int v_out = 10*sqrt(Output);
  analogWrite(pwrPin,v_out);
  
  current = analogRead(A4)/56.9;
  voltage = v_out/4.16;
  resistance = voltage/current;
  power = voltage*current;
  
  //{"power", "resistance","setpoint","temperature","time","voltage"};
  fieldData[0] = String(current);
  fieldData[1] = String(resistance);
  fieldData[2] = String(Setpoint);
  fieldData[3] = String(celcius);
  int seconds = cycleTime/1000;
  fieldData[4] = String(seconds); // phant fails after 100s. Independent of number of writes. This may be related to the length of the string
  fieldData[5] = String(voltage);
  // Open the file. Only one file can be open at a time. 
  // now post to phant
  // we can only post 100 points every 15 minutes, but it's plenty for our purposes. Conservatively once every 5 (1 in 50) seconds
  // during the active phase and once every 60 (1 in 600) seconds in the cooldown.
  if (phant)  {
    if (cycleTime < (rampTime + soakTime + rampTime)) {//in the hot phase
      if ((point % 50) == 0)  { //every 50th point
        PostData();
      }
    }
    else { // in the cool phase
      if ((point % 600) == 0)  { //every 600th point
        PostData();
      }
    }
  }
  SaveData();
  
//  if(debug)  {
//  Serial.print("Temperature:  ");
//  Serial.print(celcius);
//  Serial.print("  Setpoint:  ");
//  Serial.print(Setpoint);
//  Serial.print("  Voltage:  ");
//  Serial.print(voltage);
//  Serial.print("  Current:  ");
//  Serial.print(current); 
//  Serial.print("  Resistance:  ");
//  Serial.print(resistance); 
//  Serial.print("  Power:  ");
//  Serial.println(power); //for voltage
//  }
  point = point + 1;
  delay(50);
  //100ms delay means approximately 3600 points in the 6 minute cycle
}

double readRTD(int pin)  {
  // Read an RTD sensor and translate to degress celcius
  double sensor = analogRead(pin);
  if (sensor < 400) sensor = 400; //anything under 400 return an imaginary number
  double rtdVoltage = sensor*4.6/1024.0;
  double rtdOhms = 4700.0*(4.6/rtdVoltage-1);
  double celcius = 3383.66 - (2.17965E-9)*sqrt(2.77436E24-(3.64461E20)*rtdOhms);
  return celcius;
}

void PostData()
{
  Process phant; // Used to send command to Shell, and view response
  String curlCmd; // Where we'll put our curl command
  String curlData[NUM_FIELDS]; // temp variables to store curl data

  // Construct curl data fields
  // Should look like: --data "fieldName=fieldData"
  for (int i=0; i<NUM_FIELDS; i++)
  {
    curlData[i] = "--data \"" + fieldName[i] + "=" + fieldData[i] + "\" ";
  }

  // Construct the curl command:
  curlCmd = "curl ";
  curlCmd += "-k --header "; // Put our private key in the header.
  curlCmd += "\"Phant-Private-Key: "; // Tells our server the key is coming
  curlCmd += privateKey; 
  curlCmd += "\" "; // Enclose the entire header with quotes.
  for (int i=0; i<NUM_FIELDS; i++)
    curlCmd += curlData[i]; // Add our data fields to the command
  curlCmd += phantURL + publicKey; // Add the server URL, including public key

  // Send the curl command:
  if (debug) Serial.print("Sending command: ");
  if (debug) Serial.println(curlCmd); // Print command for debug
    // Read out the response:
  if (debug) Serial.print("Response: ");
  // Use the phant process to read in any response from Linux:
  phant.runShellCommand(curlCmd); // Send command through Shell
  while (phant.available()>0)
  {
    char c = phant.read();
    if (debug) Serial.print(c);
  }
  Serial.println();
  Serial.flush();
}

void SaveData ()  {
  // works when dataFile is not called.
  File dataFile = FileSystem.open("/mnt/sda1/datalogjan22.csv", FILE_APPEND);
  //This is screwing with the phant command somehow
  String dataString = fieldData[0] + "," + fieldData[1] + "," + fieldData[2] + "," + fieldData[3] + "," + fieldData[4] + ","+ fieldData[5];
  //String dataString = "Test";
  dataFile.println(dataString);
  Serial.println(dataString);
//  if (dataFile) {
//    dataFile.println(dataString);
//    dataFile.close();
//    // print to the serial port too:
//    if(debug) Serial.println(dataString);
//  }
//  else {
//    if(debug) Serial.println("error opening datalog.txt");
//  }
  dataFile.close();
}
