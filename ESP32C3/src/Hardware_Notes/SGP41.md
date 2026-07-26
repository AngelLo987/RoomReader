# Payload Structre of the SGP41 
## The sensor returns a 6-byte frame when requested via I2C (0x2612) and samples every one seconf

1. Raw VOC signal (Bytes 0-2)
0&1: SRAW_VOC ticks (High and Low Byte)
2: CRC-8 Checksum for the reading
2. Raw NOx Signal (Bytes 3-5)
3&4: SRAW_NOX ticks (high & low byte)
5: checksum for reading 

# SGP41 Air Quality Sensor Reference

## 1. Hardware Interface 
* **Communication:** I2C 
* **I2C Address:** 0x59
* **Power (VCC):** 3.3V (1.7V to 3.6V supported)
* **Logic Level:** 3.3V (SDA/SCL - Safe for direct connection to ESP32)


# Example Frame:

uint8_t frame[6] = {
  0x7F, 0xFF, 0x89, // [0-2] Raw VOC Ticks: ~32767 + CRC
  0x7F, 0xFF, 0x89  // [3-5] Raw NOx Ticks: ~32767 + CRC
};


# Key Differences & Implementation Notes

## 1. It Requires an Algorithm
Unlike the SCD40 (which outputs direct CO2 ppm) or the PMS5003 (which outputs direct mass concentrations), the SGP41 *only* outputs raw digital ticks. These ticks represent the physical resistance across its internal heated membrane. To convert these ticks into a usable 0-500 "VOC Index" and "NOx Index," you have to feed the raw ticks into Sensirion's official C++ Gas Index Algorithm (GIA) library on your ESP32.

## 2. It Loves Humidity Data
For the SGP41 to give the most accurate readings, it actually wants you to send it the current Temperature and Humidity before you ask it for VOC and NOx. Fortunately, since you already have the SCD40 on your I2C bus, you can eventually pass the SCD40's temperature and humidity data right into the SGP41!