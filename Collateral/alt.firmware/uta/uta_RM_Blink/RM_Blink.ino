/*
  Blink
  Turns top left front panel LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// In the Radio Music Hardware,  Bank LEDs are 3,4,5,6, Reset LED is 11. 
// Choose an LED and give it a name:
int led = 3;

void app_stop(){
	
}

// the setup routine runs once when you press reset:
void app_setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
}

// the loop routine runs over and over again forever:
void app_loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}
