#include <RFM69.h>  //  https://github.com/LowPowerLab/RFM69
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h> 
#include <Adafruit_SHT31.h> //https://github.com/adafruit/Adafruit_SHT31
#include <Adafruit_BMP280.h> // https://github.com/adafruit/Adafruit_BMP280_Library
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_TSL2591.h> // https://github.com/adafruit/Adafruit_TSL2591_Library
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <avr/sleep.h>
#include <avr/wdt.h>

// define node parameters
char node[] = "25";
#define NODEID        25 // same sa above - must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define GATEWAYID     1
#define NETWORKID     101
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // led pin
#define PIN           6 // NeoPixel driver pin

// define objects
RFM69 radio;
Adafruit_BMP280 bme; // I2C
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);  // number of pixels, digital pin, pixel flags

// define IAQ core global variables
#define iaqaddress 0x5A
uint16_t co2;
uint8_t statu;
int32_t resistance;
uint16_t tvoc;

// define global variables
long lux;
float bar;
float sound;

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

  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  
  pinMode(9, OUTPUT);  // pin 9 controls LED
  
  strip.begin(); // initialize neo pixels
  strip.show(); // Initialize all pixels to 'off'

  Serial.println("Ready");
}


void sleep()
{
  Serial.flush(); // empty the send buffer, before continue with; going to sleep
  
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
 
  //fadeLED();
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
   //T/RH
  sht31.begin(0x44);
  float temp = sht31.readTemperature();
  float rh = sht31.readHumidity();

  
  //light
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

  
  // IAQ core
  Wire.requestFrom(iaqaddress, 9);
  co2 = (Wire.read()<< 8 | Wire.read()); 
  statu = Wire.read();
  resistance = (Wire.read()& 0x00)| (Wire.read()<<16)| (Wire.read()<<8| Wire.read());
  tvoc = (Wire.read()<<8 | Wire.read());

  
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
    

  // BMP280 air pressure sensor
  bme.begin();
  float bar = bme.readPressure();
  bar = bar / 3386.39;  // convert pressure in Pa to inches of Mercury
  


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
  dtostrf(bar, 3, 2, _g);
  dtostrf(lux, 1, 0, _l);
  dtostrf(sound, 1, 1, _s);
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
  strcat(dataPacket, ",g:");
  strcat(dataPacket, _g);
  strcat(dataPacket, ",l:");
  strcat(dataPacket, _l);
  strcat(dataPacket, ",s:");
  strcat(dataPacket, _s);
  strcat(dataPacket, ",v:");
  strcat(dataPacket, _v);
  delay(50);
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


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

