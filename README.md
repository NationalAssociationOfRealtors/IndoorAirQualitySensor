# Open Source Indoor Air Quality Sensor
An open hardware and software indoor air quality sensor provided by the [National Association of REALTORS](http://realtor.org) and [CRT Labs](https://crtlabs.org).

The board comes in two form factors. One dedicated for the Raspberry Pi - [PiAQ](http://piaq.io)

<p align="center">
  <a href="http://piaq.io"><img src="http://piaq.io/images/preview-full-IMG_0117.png" alt="CRT Sensor Board"/></a>
</p>


And a standalone model with a variety of connectivity options
<p align="center">
  <img src="https://github.com/NationalAssociationOfRealtors/IndoorAirQualitySensor/blob/master/docs/media/v0.3_prototype.jpg" alt="CRT Sensor Board"/>
</p>



<p align="center">
  <img src="https://github.com/NationalAssociationOfRealtors/IndoorAirQualitySensor/blob/master/docs/media/sensor_board_v0.5.png" alt="CRT Sensor Board OSH Park render"/>
</p>

Measurement Points:

* Temperature (SHT31)
* Relative Humidity (SHT31)
* VOC (Volatile Organic Compounds) (IAQ Core)
* Derived CO2 (IAQ Core)
* Barometric Pressure (MPL3115A2)
* Light Intensity (TSL2561)
* CO Concentration (MiCS-4154)
* NO2 Concentration (MiCS-4154)
* Sound Intensity (ADMP401)

Connectivity:

* FSK (RFM69HW 433/915MHz)
* WiFi (Particle Photon)
* Cellular (Particle Electron) (Coming soon)
* LoRa (Coming Soon)

License:
Apache License Version 2.0
