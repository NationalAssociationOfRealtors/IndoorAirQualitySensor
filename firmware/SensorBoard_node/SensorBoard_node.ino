#include <RFM69.h>  // from lowpowerlab
#include <SPI.h>
#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "SparkFunMPL3115A2.h"
#include <Adafruit_Sensor.h>  // from Adafruit's github
#include <Adafruit_TSL2561_U.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// define node parameters
char node[] = "3";
#define NODEID        3 // same sa above - must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define GATEWAYID     1
#define NETWORKID     101
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED             9

// define objects
RFM69 radio;
MPL3115A2 myPressure;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// define IAQ core global variables
#define iaqaddress 0x5A
uint16_t co2;
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
float bar;
int sound;
float co;
float no2;

char dataPacket[150];

ISR(WDT_vect)  // Interrupt service routine for WatchDog Timer
{
  wdt_disable();  // disable watchdog
}

void setup()
{
  pinMode(10, OUTPUT);
  Serial.begin(115200);
  Serial.println("Setup");

  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  
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


void sleep()
{
  //Serial.println("Going to Sleep");
  Serial.flush(); // empty the send buffer, before continue with; going to sleep

  digitalWrite(A1, LOW); // turn off UV sensor
  
  radio.sleep();
  
  cli();          // stop interrupts
  MCUSR = 0;
  WDTCSR  = (1<<WDCE | 1<<WDE);     // watchdog change enable
  WDTCSR  = 1<<WDIE | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (0<<WDP0); // set  prescaler to 4 second
  sei();  // enable global interrupts

  byte _ADCSRA = ADCSRA;  // save ADC state
  ADCSRA &= ~(1 << ADEN);

  asm("wdr");
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();       
  //PORTD |= (1<<PORTD4); //Activate pullup on pin 4
  //PCICR |= (1<<PCIE2);
  //PCMSK2 |= (1<<PCINT20);
  sleep_enable();  
  sleep_bod_disable();
  sei();       
  sleep_cpu();   
    
  sleep_disable();   
  sei();  

  ADCSRA = _ADCSRA; // restore ADC state (enable ADC)
  delay(1);
  
  //ADCSRA &= (1 << ADEN);
}


void loop() 
{
  sleep();

  long t1 = millis();
  readSensors();
  
  Serial.println(dataPacket);
  Serial.println(strlen(dataPacket));
  delay(50);

  // send datapacket
  radio.sendWithRetry(GATEWAYID, dataPacket, strlen(dataPacket), 5, 100);  // send data, retry 5 times with delay of 100ms between each retry
  dataPacket[0] = (char)0; // clearing first byte of char array clears the array

  long t2 = millis();
  Serial.println(t2-t1);
  /*
  // blink LED after sending to give visual indication on the board
  digitalWrite(9, HIGH);
  delay(50);
  digitalWrite(9, LOW);
  */
  fadeLED();
  long t3 = millis();
  Serial.println(t3-t2);
}



void readSensors()
{
  
   //T/RH
  sht31.begin(0x44);
  float temp = sht31.readTemperature();
  float rh = sht31.readHumidity();

  
  //light
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


  // IAQ core
  Wire.requestFrom(iaqaddress, 9);
  co2 = (Wire.read()<< 8 | Wire.read()); 
  statu = Wire.read();
  resistance = (Wire.read()& 0x00)| (Wire.read()<<16)| (Wire.read()<<8| Wire.read());
  tvoc = (Wire.read()<<8 | Wire.read());

  

  // ADMP401 mic for sound level
  sound = analogRead(A0);  // read sound levels


  // MPL3115A2 air pressure sensor
  myPressure.begin(); // Get pressure sensor online
  //Configure the sensor
  //myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 
  delay(10);
  
  float pressure = myPressure.readPressure();
  float temperature = myPressure.readTempF();

  //const int station_elevation_ft = 5374; //Must be obtained with a GPS unit
  //float station_elevation_m = station_elevation_ft * 0.3048; //I'm going to hard code this
  const int station_elevation_m = 181; //Chicago
  //1 pascal = 0.01 millibars
  pressure /= 100; //pressure is now in millibars

  float part1 = pressure - 0.3; //Part 1 of formula
  
  const float part2 = 8.42288 / 100000.0;
  float part3 = pow((pressure - 0.3), 0.190284);
  float part4 = (float)station_elevation_m / part3;
  float part5 = (1.0 + (part2 * part4));
  float part6 = pow(part5, (1.0/0.190284));
  float altimeter_setting_pressure_mb = part1 * part6; //Output is now in adjusted millibars
  bar = altimeter_setting_pressure_mb * 0.02953;



  // MICS 4514 CO/NO2 gas sensor
  // Read analog values, print them out, and wait
  vnox_value = analogRead(VNOX_PIN);
  vred_value = analogRead(VRED_PIN);
  co = readCO();
  no2 = readNO2();




  // define character arrays for all variables
  //char _i[4];
  char _t[7];
  char _h[7];
  char _c[7];
  char _o[7];
  char _g[7];
  char _l[7];
  char _n[7];
  char _s[7];
  char _v[7];
  

  // convert all flaoting point and integer variables into character arrays
  //dtostrf(nodeID, 2, 0, _i);
  dtostrf(temp, 3, 1, _t);  // this function converts float into char array. 3 is minimum width, 2 is decimal precision
  dtostrf(rh, 3, 1, _h);
  dtostrf(co2, 3, 0, _c);
  dtostrf(co, 1, 0, _o); 
  dtostrf(bar, 3, 2, _g);
  dtostrf(lux, 1, 0, _l);
  dtostrf(no2, 1, 0, _n);
  dtostrf(sound, 1, 0, _s);
  dtostrf(tvoc, 1, 0, _v);
  delay(50);
  
  dataPacket[0] = 0;  // first value of dataPacket should be a 0
  
  // create datapacket by combining all character arrays into a large character array
  strcat(dataPacket, "i:");
  strcat(dataPacket, node);
  strcat(dataPacket, ",t:");
  strcat(dataPacket, _t);
  strcat(dataPacket, ",h:");
  strcat(dataPacket, _h);
  strcat(dataPacket, ",c:");
  strcat(dataPacket, _c);
  strcat(dataPacket, ",o:");
  strcat(dataPacket, _o);
  strcat(dataPacket, ",g:");
  strcat(dataPacket, _g);
  strcat(dataPacket, ",l:");
  strcat(dataPacket, _l);
  strcat(dataPacket, ",n:");
  strcat(dataPacket, _n);
  strcat(dataPacket, ",s:");
  strcat(dataPacket, _s);
  strcat(dataPacket, ",v:");
  strcat(dataPacket, _v);
  delay(50);
}




float readCO()
{
  float adc = analogRead(VNOX_PIN);
  float Rs = 47000.0/(1023.0/adc);
  float R0 = 82000.0;
  float ratio = Rs/R0;
  float concentration = ((-181.4)*log(ratio))-97.469;
  //Serial.println(adc);
  //Serial.println(Rs);
  //Serial.println(R0);
  //Serial.println(ratio);
  if(concentration>0)
  {
    return concentration;
  }
  else
  {
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
  //Serial.println(adc);
  //Serial.println(Rs);
  //Serial.println(R0);
  //Serial.println(ratio);
  if(concentration>0)
  {
    return concentration;
  }
  else
  {
    return 0;
  }
}



// interpolate the UV intensity (from voltage vs intensity graph in datasheet)
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



void fadeLED()
{
  int brightness = 0;
  int fadeAmount = 5;
  for(int i=0; i<510; i=i+5)  // 255 is max analog value, 255 * 2 = 510
  {
    analogWrite(9, brightness);  // pin 9 is LED
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;  // increment brightness level by 5 each time (0 is lowest, 255 is highest)
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255)
    {
      fadeAmount = -fadeAmount;
    }
    // wait for 20-30 milliseconds to see the dimming effect
    delay(10);
  }
  digitalWrite(9, LOW); // switch LED off at the end of fade
}


