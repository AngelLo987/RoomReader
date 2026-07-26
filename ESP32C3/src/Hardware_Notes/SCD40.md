# Payload structure of the SCD40 
## -The sensor gives a 9-byte frame, only when requested via I2C and samples every 5 seconds 

1. CO2 Concentration (Bytes 0-2)
0&1: CO2 in ppm (high Byte and Low Byte)
2: CRC checksum for the CO2 reading 

2. Temperature (Bytes 3-5)
3&4: Raw Temperature (High and Low Byte). Will be in ticks, so can convert to C by doing (175*Tick)/65536
5: CRC checksum for the Temperature reading

3. Relative Humidity (Bytes 6-8)
6&7: Raw Humidty (High and Low Byte). Will be in ticks, so can convert to % RH by doing (100*Tick)/65535
8: 

**FYI: ticks range from 0 to 56,535 for both temp and humidity**

# SCD40 Air Quality Sensor Reference

## 1. Hardware Interface 
* **Communication:** I2C 
* **I2C Address:** 0x62
* **Power (VCC):** 3.3V (2.4V to 5.5V supported)
* **Logic Level:** 3.3V (SDA/SCL - Safe for direct connection to ESP32)

# Example Frame:

uint8_t frame[9] = {
  0x03, 0x20, 0x54, // [0-2] CO2: ~800 ppm + CRC
  0x66, 0x66, 0x93, // [3-5] Temp: ~25.0°C + CRC
  0x5E, 0x96, 0x48  // [6-8] Humidity: ~37.0% + CRC
};

# Important