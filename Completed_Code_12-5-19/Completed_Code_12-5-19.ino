//******** Libraries ***********
#include <SparkFun_Qwiic_Relay.h>
#include <U8glib.h>
#include "SparkFun_Qwiic_Relay.h"
#include "U8glib.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#define rx 2
#define tx 3
#include "TSYS01.h"
#include "MS5837.h"
#define RELAY_ADDR 0x18
#include <SPI.h>
#include <SD.h>
//*******************************

int burndelay = 15; //CHANGE THIS --- The time in seconds before the relay executes.

TSYS01 sensor1;
MS5837 sensor0;

Qwiic_Relay relay(RELAY_ADDR); 
File myFile;
SoftwareSerial myserial(rx, tx);

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

  while (!sensor0.init()) {
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

  sensor0.setModel(MS5837::MS5837_30BA);
  sensor0.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void serialEvent() {
  inputstring = Serial.readStringUntil(13);
  input_string_complete = true;
}

void loop() {

  if (millis() == burndelay*1000){

    relay.turnRelayOn();
    delay(1000);
    relay.turnRelayOff();
    delay(1000);
    relay.turnRelayOn();
    delay(1000);
    relay.turnRelayOff();
  } else {
  if (timing == 1000) {
    sensor0.read();
    sensor1.read();

    myFile = SD.open("Output.txt", FILE_WRITE);
    myFile.print("Temperature: ");
    myFile.println(sensor1.temperature());
    Serial.print("Temperature: "); 
    Serial.print(sensor1.temperature()); 
    Serial.println(" deg C");

    myFile.print("Depth: ");
    myFile.println(sensor0.depth());
    Serial.print("Depth: "); 
    Serial.print(sensor0.depth()); 
    Serial.println(" m");
    Serial.println("");

    if (sensor_string_complete == true) {
      Serial.print("pH:");
      Serial.println(sensorstring);
      myFile.print("pH: ");
      myFile.println(sensorstring);
      myFile.close();
      sensorstring = "";
      sensor_string_complete = false;
    }
  timing = 0;
  }

  if (input_string_complete == true) {
    myserial.print(inputstring);
    myserial.print('\r');
    inputstring = "";
    input_string_complete = false;
  }
  
  if (myserial.available() > 0) {
    char inchar = (char)myserial.read();
    sensorstring += inchar;
    if (inchar == '\r') {
      sensor_string_complete = true;
    }
  }

  delay(1);
  timing = timing +1; 
  }
}
