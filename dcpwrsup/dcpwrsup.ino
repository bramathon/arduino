/*
 Fade

 This example shows how to fade an LED on pin 9
 using the analogWrite() function.

 This example code is in the public domain.
 */

int pwr = 3;           // the pin that the LED is attached to
int output = 50;    // how bright the LED is
int i =0;
//255 = 5055mV => 61.3V
//200 = 3922mV => 54.2V
//150 = 2983mV => 48.0V
//100 = 1961mV => 41.3V
//50 = 980.4mV => 31.1V
//25 = 490.2mV => 20.6V
//10 = 196.1mV => 9.7V
// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  Serial.begin(115200);
  pinMode(pwr, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // set the brightness of pin 9:
  analogWrite(pwr, output);
  i = analogRead(A4);
  //376 = 0.367V = 1.0A
  Serial.println(i);
  //LP the input, 10k and 10uF works nice
  //2.1A =117
  //1.0A = 59
  // wait for 30 milliseconds to see the dimming effect
  delay(300);
}

