#include <Wire.h>
#include <SparkFun_Qwiic_Relay.h>
#include <U8glib.h>
#include "SparkFun_Qwiic_Relay.h"
#include "U8glib.h"
#include <SoftwareSerial.h>
#include "TSYS01.h"
#include "MS5837.h"
#define rx 2
#define tx 3
#define RELAY_ADDR 0x18
#include <SPI.h>
#include <SD.h>

File myFile;

TSYS01 sensor1;

MS5837 sensor;

Qwiic_Relay relay(RELAY_ADDR);

SoftwareSerial myserial(rx, tx);

String inputstring = "";
String sensorstring = "";
boolean input_string_complete = false;
boolean sensor_string_complete = false;
int timing = 0;
int time0 = 0;
int time1 = 0;
float pH;

void setup() {

  Serial.begin(9600);
  myserial.begin(9600);
  inputstring.reserve(10);
  sensorstring.reserve(30);

  Serial.println("Starting");
  
  Wire.begin();
  
  sensor1.init();
  while (!sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }

  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  Serial.println("SD initialization done.");
  myFile = SD.open("Output.txt", FILE_WRITE);

  if (myFile) {
    Serial.print("Testing Connection");
    myFile.println("Success");
    myFile.close();
    Serial.println("Success");
  } else {
    Serial.println("Error opening Output.txt");
  }

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
}

void serialEvent() {
  inputstring = Serial.readStringUntil(13);
  input_string_complete = true;
}

void loop() {

if (time0 >= 1000) {

  relay.turnRelayOn();

  delay(100);

  relay.turnRelayOff();

  delay(100);

  relay.turnRelayOn();

  delay(100);

  relay.turnRelayOff();

  time0 = -720000;

  } else { 
  myFile = SD.open("Output.txt", FILE_WRITE);
  if (timing == 170) {
    sensor.read();
    sensor1.read();

    Serial.print("Pressure: ");
    Serial.print(sensor.pressure());
    Serial.println(" mbar");

    myFile.print("Pressure: ");
    myFile.print(sensor.pressure());
    myFile.println(" mbar");

    Serial.print("Temperature: ");
    Serial.print(sensor1.temperature());
    Serial.println(" deg C");

    myFile.print("Temperature: ");
    myFile.print(sensor1.temperature());
    myFile.println(" deg C");

    Serial.print("Depth: ");
    Serial.print(sensor.depth());
    Serial.println(" m");

    myFile.print("Depth: ");
    myFile.print(sensor.depth());
    myFile.println(" m");

    Serial.print("Altitude: ");
    Serial.print(sensor.altitude());
    Serial.println(" m above mean sea level");
    Serial.println("");
    Serial.println("--------------");

    myFile.print("Altitude: ");
    myFile.print(sensor.altitude());
    myFile.println(" m above mean sea level");
    myFile.println("");
    myFile.println("--------------");
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

  //pH Print
  if (sensor_string_complete == true) {
    Serial.print("pH: ");
    Serial.println(sensorstring);
    myFile.print("pH: ");
    myFile.println(sensorstring);
    sensorstring = "";
    sensor_string_complete = false;
  }

  delay(1);
  timing = timing + 1;
  time0 = time0 + 1;
  myFile.close();
  }
}
