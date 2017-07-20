// v0.56 - 7/20/2017
// Developed by Akram Ali

// www.crtlabs.org

#include <RFM69.h>  //  https://github.com/LowPowerLab/RFM69
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h> 
#include <CCS811.h> // https://github.com/AKstudios/CCS811-library
//#include <Adafruit_SHT31.h> //https://github.com/adafruit/Adafruit_SHT31
#include <Adafruit_Si7021.h> // https://github.com/adafruit/Adafruit_Si7021
#include <Adafruit_BMP280.h> // https://github.com/adafruit/Adafruit_BMP280_Library
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_TSL2591.h> // https://github.com/adafruit/Adafruit_TSL2591_Library
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "config.h"

// define node parameters
//char node[] = "25";
//#define NODEID        25 // same sa above - must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define GATEWAYID     1
#define NETWORKID     101
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
//#define LED           9 // led pin
#define PIN           6 // NeoPixel driver pin

// define objects
RFM69 radio;
Adafruit_BMP280 bme; // I2C
//Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_Si7021 sensor = Adafruit_Si7021();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);  // number of pixels, digital pin, pixel flags

/*
// define IAQ core global variables
#define iaqaddress 0x5A
uint16_t co2;
uint8_t statu;
int32_t resistance;
uint16_t tvoc;
*/

// define CCS811 global variables
#define ADDR      0x5B
#define WAKE_PIN  5
CCS811 ccs811_sensor;
uint16_t tvoc;

// define MH-z19 global variables
SoftwareSerial mySerial(8, 7); // RX, TX
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
unsigned char response[18]; // expect a 9 bytes response, give twice the size just in case things go crazy, it reduces likelyhood of crash/buffer overrun

// define PMS7003 global variables
SoftwareSerial mySerial2(3, 4); // RX, TX
char buf[31];
long CF1PM01Value,CF1PM25Value,CF1PM10Value,atmPM01Value,atmPM25Value,atmPM10Value,Partcount0_3,Partcount0_5,Partcount1_0,Partcount2_5,Partcount5_0,Partcount10;

// define other global variables
long lux;
float bar;
float sound;
int ppm, _ppm;
float co2;

char dataPacket[150];

ISR(WDT_vect)  // Interrupt service routine for WatchDog Timer
{
  wdt_disable();  // disable watchdog
}

void setup()
{
  pinMode(10, OUTPUT); // Radio SS pin set as output

  Serial.begin(115200);
  Serial.println("Setup");

  ccs811_sensor.begin(uint8_t(ADDR), uint8_t(WAKE_PIN));  // initialize CCS811 sensor
  
  mySerial.begin(9600);   // initialize MHz-19 in UART mode
  mySerial2.begin(9600);   // initialize PMS7003 in UART mode

  while(mySerial2.available() > 0)  // clear out buffer
    char x = mySerial2.read();
  delay(1);

  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  
  //pinMode(9, OUTPUT);  // pin 9 controls LED
  
  strip.begin(); // initialize neo pixels
  strip.show(); // Initialize all pixels to 'off'

  // get first CO2 reading
  while(mySerial.available() > 0)  // clear out buffer
    char x = mySerial.read();
  mySerial.write(cmd,9);
  delay(1);
  if(mySerial.available() > 0 && (unsigned char)mySerial.peek() != 0xFF)
    mySerial.read();
  mySerial.readBytes(response, 9);
  unsigned int responseHigh = (unsigned int) response[2];
  unsigned int responseLow = (unsigned int) response[3];
  ppm = (256 * responseHigh) + responseLow;
  _ppm = ppm;
  
  Serial.println("Ready");
  delay(10);
}


void sleep()
{
  Serial.flush(); // empty the send buffer, before continue with; going to sleep
  
  radio.sleep();
  
  cli();          // stop interrupts
  MCUSR = 0;
  WDTCSR  = (1<<WDCE | 1<<WDE);     // watchdog change enable
  WDTCSR  = 1<<WDIE | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0); // set  prescaler to 8 second
  sei();  // enable global interrupts

  byte _ADCSRA = ADCSRA;  // save ADC state
  ADCSRA &= ~(1 << ADEN);

  asm("wdr");
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();       

  sleep_enable();  
  sleep_bod_disable();
  sei();       
  sleep_cpu();   
    
  sleep_disable();   
  sei();  

  ADCSRA = _ADCSRA; // restore ADC state (enable ADC)
  delay(1);
}


void loop() 
{
  sleep();
  readSensors();
  
  Serial.println(dataPacket);
  delay(50);

  // send datapacket
  radio.sendWithRetry(GATEWAYID, dataPacket, strlen(dataPacket), 5, 100);  // send data, retry 5 times with delay of 100ms between each retry
  dataPacket[0] = (char)0; // clearing first byte of char array clears the array
 
  //colorWipe(strip.Color(0, 255, 0), 10); // Green
  //strip.show();
  
  for (int i = 0; i <= 255; i++)
  {
      colorWipe(strip.Color(0,i,0), 1);
  }
  for (int i = 255; i > 0; i--)
  {
      colorWipe(strip.Color(0,i,0), 2);
  }
  colorWipe(strip.Color(0, 0, 0), 1); // turn pixel off
  strip.show();
}


void readSensors()
{
  /*
  // T/RH - SHT31
  sht31.begin(0x44);
  float temp = sht31.readTemperature();
  float rh = sht31.readHumidity();
  */
  
  // T/RH - Si7021
  sensor.begin();
  float temp = sensor.readTemperature();
  float rh = sensor.readHumidity();
  
  // Light Intensity - TSL2591
  tsl.begin();
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  sensors_event_t event;
  tsl.getEvent(&event);
  if ((event.light == 0) | (event.light > 4294966000.0) | (event.light <-4294966000.0))
  {
    lux = 0;  // invalid value; replace with 'NAN' if needed
  }
  else
  {
    lux = event.light;
  }

  /*
  // IAQ core
  Wire.requestFrom(iaqaddress, 9);
  co2 = (Wire.read()<< 8 | Wire.read()); 
  statu = Wire.read();
  resistance = (Wire.read()& 0x00)| (Wire.read()<<16)| (Wire.read()<<8| Wire.read());
  tvoc = (Wire.read()<<8 | Wire.read());
  */

  // MH-z19 UART
  mySerial.listen();
  delay(1);
  while(mySerial.available() > 0)  // clear out buffer
    char x = mySerial.read();
  mySerial.write(cmd,9);
  delay(1);
  if(mySerial.available() > 0 && (unsigned char)mySerial.peek() != 0xFF)
    mySerial.read();
  mySerial.readBytes(response, 9);
  unsigned int responseHigh = (unsigned int) response[2];
  unsigned int responseLow = (unsigned int) response[3];
  ppm = (256 * responseHigh) + responseLow;
  co2 = (0.7*ppm) + (0.3*_ppm);  // real-time exponential smoothing of data with a damping factor of 0.3
  _ppm = (int)co2;  // save old reading;

  
  // ADMP401 mic for sound level
  float sumADC=0.0;
  for(int i=0;i<5;i++)
  {
     sumADC = sumADC + analogRead(A0); // take avg of 5 readings
  }
  float averageADC = sumADC/5.0;
  float volts = (averageADC/1023.0 * 3.3);
  sound = (20 * log10(volts/0.007943)) - 42 + 94 - 60;  // VRMS = 0.007943; -42dB is sensitivity of ADMP401 mic; 1 Pa = 94 dB SPL RMS; 60dB is gain of amplifier
  // the above is uncalibrated sound level - needs to be calibrated with reference to an accurate sound level meter in varying SPLs, frequencies and environments.
  //sound = averageADC;

  // BMP280 air pressure sensor
  bme.begin();
  float bar = bme.readPressure();
  bar = bar / 3386.39;  // convert pressure in Pa to inches of Mercury


  // CCS811 VOC readings
  ccs811_sensor.compensate(temp, rh);  // environmental compensation for current temperature and humidity
  ccs811_sensor.getData();
  tvoc = ccs811_sensor.readTVOC();


  // PMS7003
  char response[10];
  long pm;
  //while(mySerial2.available() > 0)  // clear out buffer
  //  char x = mySerial2.read();
  delay(1);
  mySerial2.listen();
  delay(1);
  //if(mySerial2.available()>0)
  //  mySerial2.read();
  //mySerial2.readBytes(buf, 31);
  if (mySerial2.find(0x42))
  {
      mySerial2.readBytes(buf, 31);
      get_data(buf);
  }
  delay(1);
  //get_data(buf);
  //Serial.println(buf);
  //Serial.println(atmPM01Value);
  pm = atmPM01Value;
  

  // define character arrays for all variables
  char _i[3];
  char _t[7];
  char _h[7];
  char _c[7];
  char _o[7];
  char _g[7];
  char _l[7];
  char _n[7];
  char _s[7];
  char _v[7];
  char _p[7];
  
  // convert all flaoting point and integer variables into character arrays
  dtostrf(NODEID, 1, 0, _i);
  dtostrf(temp, 3, 2, _t);  // this function converts float into char array. 3 is minimum width, 2 is decimal precision
  dtostrf(rh, 3, 2, _h);
  dtostrf(co2, 3, 0, _c); 
  dtostrf(bar, 3, 2, _g);
  dtostrf(lux, 1, 0, _l);
  dtostrf(sound, 1, 1, _s);
  dtostrf(tvoc, 1, 0, _v);
  dtostrf(pm, 1, 0, _p);
  delay(50);
  
  dataPacket[0] = 0;  // first value of dataPacket should be a 0
  
  // create datapacket by combining all character arrays into a large character array
  strcat(dataPacket, "i:");
  strcat(dataPacket, _i);
  strcat(dataPacket, ",t:");
  strcat(dataPacket, _t);
  strcat(dataPacket, ",h:");
  strcat(dataPacket, _h);
  strcat(dataPacket, ",c:");
  strcat(dataPacket, _c);
  strcat(dataPacket, ",g:");
  strcat(dataPacket, _g);
  strcat(dataPacket, ",l:");
  strcat(dataPacket, _l);
  strcat(dataPacket, ",s:");
  strcat(dataPacket, _s);
  strcat(dataPacket, ",p:");
  strcat(dataPacket, _p);
  strcat(dataPacket, ",v:");
  strcat(dataPacket, _v);
  delay(50);
}


void get_data(unsigned char b[])
{
  int receiveSum = 0;
  for (int i = 0; i < (31 - 2); i++)
  {
    receiveSum = receiveSum + b[i];
  }
  receiveSum = receiveSum + 0x42;
  receiveSum == ((b[31 - 2] << 8) + b[31 - 1]);
  
  if(b[0] == 0x4D && receiveSum)
  {
    CF1PM01Value = (b[9] << 8) + b[10]; //PM1.0 CF1 value of the air detector module
    CF1PM25Value = (b[5] << 8) + b[6]; //PM2.5 CF1 value of the air detector module
    CF1PM10Value = (b[7] << 8) + b[8]; //PM10 CF1 value of the air detector module

    atmPM01Value = (b[3] << 8) + b[4]; //PM1.0 atmospheric value of the air detector module
    atmPM25Value = (b[11] << 8) + b[12]; //PM2.5 atmospheric value of the air detector module
    atmPM10Value = (b[13] << 8) + b[14]; //PM10 atmospheric value of the air detector module

    Partcount0_3 = (b[15] << 8) + b[16]; //count particules > 0.3 µm
    Partcount0_5 = (b[17] << 8) + b[18]; //count particules > 0.5 µm
    Partcount1_0 = (b[19] << 8) + b[20]; //count particules > 1.0 µm
    Partcount2_5 = (b[21] << 8) + b[22]; //count particules > 2.5 µm
    Partcount5_0 = (b[23] << 8) + b[24]; //count particules > 5.0 µm
    Partcount10 = (b[25] << 8) + b[26]; //count particules > 10.0 µm
  }
}

/*
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
*/



// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// bruh
