# DHT11_RaspberryPi
C language code example to read humidity and temperature from a DHT11 sensor connected to a Raspberry Pi

dht11_sensor.c - V1.0 
 
Ronan Damasco - Jul/17
 
Reads temperature and humidity from a DHT11 sensor 
 
Allows to configure: 
   - Time interval to perform readings
   - Number of readings 
   - Interval between readings
   - Possible outputs:
      - Display
      - Output file

Based on example available at: http://kookye.com/2017/06/01/%E6%A0%91%E8%8E%93%E6%B4%BE%E8%AF%BB%E5%8F%96dht11/#more-4539
 
Compile with: sudo gcc -o dht11_sensor dht11_sensor.c -lwiringPi

Usage: dht11_sensor [?/-?/-h] [-v] [-m:xxxx] [-r:xxxx] [-t:xxxx] [-o:<output file>]

Where: -?/?/-h: help
       -v: verbose mode
       -m: number of minutes (maximum 9,999, only the first 4 digits will be considered)
       -r: number of readings (maximum 9,999, only the first 4 digits will be considered)
       -t: interval, number of seconds between each read (maximum 9,999, only the first 4 digits will be considered)
       -o: output file

Obs: if both -m and -r are set, minutes have precedence. At least one must be specified.

Return values:
    0 - OK
    1 - Error
