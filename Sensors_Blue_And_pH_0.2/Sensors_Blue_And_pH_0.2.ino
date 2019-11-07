#include <Wire.h>
#include <SoftwareSerial.h>
#include <SparkFun_Qwiic_Relay.h>
#include <U8glib.h>
#include "SparkFun_Qwiic_Relay.h"
#include "U8glib.h"
#include "TSYS01.h"
#include "MS5837.h"
#define RELAY_ADDR 0x18 // Alternate address 0x19
#define rx 2
#define tx 3

TSYS01 sensor1;

MS5837 sensor;

Qwiic_Relay relay(RELAY_ADDR); 

SoftwareSerial myserial(rx, tx);

int burndelay = 0;
String inputstring = "";
String sensorstring = "";
boolean input_string_complete = false;
boolean sensor_string_complete = false;
int timing = 0;

void setup() {

  Wire.begin(); 
  Serial.begin(9600);
  myserial.begin(9600);
  inputstring.reserve(10);
  sensorstring.reserve(30);  

  Serial.println("Starting");
  sensor1.init();

  while (!sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }

  if(!relay.begin())
    Serial.println("Check connections to Qwiic Relay.");
  else
    Serial.println("Ready to flip some switches.");
    float version = relay.singleRelayVersion();
    Serial.print("Firmware Version: ");
    Serial.println(version);

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
}

void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
  Serial.println("bruh");
}

void loop() {

if (burndelay >= 300000) {

  relay.turnRelayOn(); //turn the relay on

  delay(1000); // stay on for 1 second

  relay.turnRelayOff(); //turns relay off

  delay(1000); // delay before second (failsafe) run

  relay.turnRelayOn(); // turns relay on a second time 

  delay(1000); //leave on for one second

  relay.turnRelayOff();

  burndelay = 7200000; //sets timer to wait another 2 hours before going off again (at one second per 1 time counter) 

} else {

  if (timing == 1000) {
    sensor.read();
    sensor1.read();

    Serial.print("Pressure: "); 
    Serial.print(sensor.pressure()); 
    Serial.println(" mbar");
  
    Serial.print("Temperature: "); 
    Serial.print(sensor1.temperature()); 
    Serial.println(" deg C");
  
    Serial.print("Depth: "); 
    Serial.print(sensor.depth()); 
    Serial.println(" m");
  
    Serial.print("Altitude: "); 
    Serial.print(sensor.altitude()); 
    Serial.println(" m above mean sea level");
    Serial.println("");
    timing = 0;
  }

  if (input_string_complete == true) {                //if a string from the PC has been received in its entirety
    myserial.print(inputstring);                      //send that string to the Atlas Scientific product
    myserial.print('\r');                             //add a <CR> to the end of the string
    inputstring = "";                                 //clear the string
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
  }
  
  if (myserial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }

  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
    Serial.print("pH:");
    Serial.println(sensorstring);                     //send that string to the PC's serial monitor
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }
  delay(1);
  timing = timing +1;
  burndelay = burndelay +1; // Should add one second to the counter change the delay to change the time increment IE 10000 (10 seconds) 60000 (a minute)   
  }
}
