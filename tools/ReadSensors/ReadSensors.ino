/*
  Sensor Board v0.2
  Measures Temperature, Relative Humidity, Light intensity,
  Air pressure (and derived altitude), Sound intensity,
  concentrations of CO, NO2, VOC (derived) and CO2 (derived)

  Currently untested: TSL2561 light sensor code
  Currently untested: ADMP401 mic code
  
  Designed at CRT Labs (July 2016)
  https://github.com/NationalAssociationOfRealtors/SensorBoard
  
  Modified: 8 September 2016
  by Akram Ali
 */

// define libraries
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "SparkFunMPL3115A2.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// define objects
MPL3115A2 myPressure;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// define IAQ core global variables
#define iaqaddress 0x5A
uint16_t predict;
uint8_t statu;
int32_t resistance;
uint16_t tvoc;

// define MICS-4514 global variables
#define PRE_PIN          8
#define VNOX_PIN         A1
#define VRED_PIN         A2
#define PRE_HEAT_SECONDS 10
int vnox_value = 0;
int vred_value = 0;

// define TSL2561 global variables
long lux;

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");

  pinMode(9, OUTPUT);  // pin 9 controls LED
  pinMode(5, OUTPUT);   // pin 5 controls power to all sensors
  digitalWrite(5, HIGH);  // turn all sensors on
  delay(10);

  pinMode(PRE_PIN, OUTPUT);  // set preheater pin for mics4514 gas sensor
  Serial.print("MICS-4514 Preheating...");
  // Wait for preheating
  digitalWrite(PRE_PIN, 1);
  delay(PRE_HEAT_SECONDS * 1000);
  digitalWrite(PRE_PIN, 0);
  Serial.println("Done");
  
}

// the loop function runs over and over again forever
void loop()
{
  digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(9, LOW);    // turn the LED off by making the voltage LOW
  
  Serial.println("---------------------");
  
  // ADMP401 mic for sound level
  Serial.print("Sound Level: ");
  Serial.print(analogRead(A0));  // read sound levels
  Serial.println();


  // TSL2561 light sensor
  tsl.begin(); 
  tsl.enableAutoRange(true);
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  sensors_event_t event;
  tsl.getEvent(&event);
  if (event.light)
    lux = event.light;
  else
    lux = -1;
  Serial.print("Light intensity (lux): ");
  Serial.println(lux);
  
  // MICS 4514 CO/NO2 gas sensor
  // Read analog values, print them out, and wait
  vnox_value = analogRead(VNOX_PIN);
  vred_value = analogRead(VRED_PIN);
  Serial.print("Vnox: ");
  Serial.print(vnox_value, DEC);
  Serial.print(" Vred: ");
  Serial.println(vred_value, DEC);
  readCO();
  readNO2();

  
  // MPL3115A2 air pressure sensor
  myPressure.begin(); // Get pressure sensor online
  //Configure the sensor
  //myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 
  delay(10);
  
  float pressure = myPressure.readPressure();
  Serial.print("MPL3115A2 Pressure(Pa): ");
  Serial.println(pressure, 2);

  float temperature = myPressure.readTempF();
  Serial.print("MPL3115A2 Temp(f): ");
  Serial.println(temperature, 2);

  //const int station_elevation_ft = 5374; //Must be obtained with a GPS unit
  //float station_elevation_m = station_elevation_ft * 0.3048; //I'm going to hard code this
  const int station_elevation_m = 1638; //Accurate for the roof on my house
  //1 pascal = 0.01 millibars
  pressure /= 100; //pressure is now in millibars

  float part1 = pressure - 0.3; //Part 1 of formula
  
  const float part2 = 8.42288 / 100000.0;
  float part3 = pow((pressure - 0.3), 0.190284);
  float part4 = (float)station_elevation_m / part3;
  float part5 = (1.0 + (part2 * part4));
  float part6 = pow(part5, (1.0/0.190284));
  float altimeter_setting_pressure_mb = part1 * part6; //Output is now in adjusted millibars
  float baroin = altimeter_setting_pressure_mb * 0.02953;

  Serial.print("MPL3115A2 Altimeter setting InHg: ");
  Serial.println(baroin, 2);

  
  // SHT31 T/RH sensor 
  sht31.begin(0x44);
  Serial.print("SHT31 Temperature: ");
  Serial.println(sht31.readTemperature());
  Serial.print("SHT31 Humidity: ");
  Serial.println(sht31.readHumidity());
  

  // IAQ core
  Wire.requestFrom(iaqaddress, 9);
  predict = (Wire.read()<< 8 | Wire.read()); 
  statu = Wire.read();
  resistance = (Wire.read()& 0x00)| (Wire.read()<<16)| (Wire.read()<<8| Wire.read());
  tvoc = (Wire.read()<<8 | Wire.read());
  checkStatus();
  
  Serial.print("CO2 concentration:");
  Serial.println(predict);
  //Serial.print("IAQ core Status: ");
  //Serial.println(statu, HEX);
  Serial.print("IAQ core Resistance: ");
  Serial.println(resistance);
  Serial.print("TVoC concentration: ");
  Serial.println(tvoc);

  
  Serial.println();
  
  //digitalWrite(5, LOW);  // turn sensors off
  delay(5000);
}



void checkStatus()
{
  if(statu == 0x10)
  {
    Serial.println("IAQ Core Warming up...");
  }
  else if(statu == 0x00)
  {
    Serial.println("IAQ Core Ready");  
  }
  else if(statu == 0x01)
  {
    Serial.println("IAQ Core Busy");  
  }
  else if(statu == 0x80)
  {
    Serial.println("IAQ Core Error");  
  }
  else
  Serial.println("No Status, check IAQ Core");  
}



float readCO()
{
  float adc = analogRead(VNOX_PIN);
  float Rs = 47000.0/(1023.0/adc);
  float R0 = 82000.0;
  float ratio = Rs/R0;
  float concentration = ((-181.4)*log(ratio))-97.469;
  Serial.print("CO concentration (ppb): ");
  //Serial.println(adc);
  //Serial.println(Rs);
  //Serial.println(R0);
  //Serial.println(ratio);
  if(concentration>0)
  {
    Serial.println(concentration);
    return concentration;
  }
  else
  {
    Serial.println("0");
    return 0;
  }
}

float readNO2()
{
  float adc = analogRead(VRED_PIN);
  float Rs = 22000.0/(1023.0/adc);
  float R0 = 350.0;
  float ratio = Rs/R0;
  float concentration = (124.28*log(ratio))-270.37;
  Serial.print("NO2 concentration (ppb): ");
  //Serial.println(adc);
  //Serial.println(Rs);
  //Serial.println(R0);
  //Serial.println(ratio);
  if(concentration>0)
  {
    Serial.println(concentration);
    return concentration;
  }
  else
  {
    Serial.println("0");
    return 0;
  }
}
