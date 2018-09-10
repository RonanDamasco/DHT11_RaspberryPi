 /* --------------------------------------------------------------------------------------------------------------------------------------
 * 
 *  dht11_sensor.c - V1.0 
 * 
 *  Ronan Damasco - Jul/17
 * 
 *  Reads temperature and humidity from a DHT11 sensor 
 * 
 *  Allows to configure: 
 *     - Time interval to perform readings
 *     - Number of readings 
 *     - Interval between readings
 *     - Possible outputs:
 *          - Display
 *          - Output file
 *
 *  Based on example available at: http://kookye.com/2017/06/01/%E6%A0%91%E8%8E%93%E6%B4%BE%E8%AF%BB%E5%8F%96dht11/#more-4539
 * 
 *  Compile with: sudo gcc -o dht11_sensor dht11_sensor.c -lwiringPi 
 *
 * --------------------------------------------------------------------------------------------------------------------------------------
 */

/*   -----------
 *    Libraries 
 *   -----------
 */

/* C libraries */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Raspberry Pi libraries */

#include <wiringPi.h> 

/*   -----------
 *    Constants 
 *   -----------
 */

#define MAXTIMINGS 85
#define DHTPIN 15  /* GPIO pin 14 */

/*   ------------------
 *    Global variables 
 *   ------------------
 */

/* Line argument variables */

uint8_t bVerbose = 0;     /* Verbose mode flag */
uint8_t bOutputFile = 0;  /* Output file flag */
char strOutputFile[80];  /* Output file name */
FILE *f_outputFile; /* Output file handle */

/* Temperature & humidity array */

int dht11_data[5] = {0, 0, 0, 0, 0}; 

/* Current time */

time_t t_currentTime;
struct tm t_Str;

/* Readings counter */

int contReadings = 0;

/* ----------------------------------------------------------------
 *
 *  Function that reads temperature and humiduty from DHT11 sensor
 *
 * ----------------------------------------------------------------
 */

int read_DTH11_data() {

  /* Local counters */

  uint8_t i, j = 0;
 
  /* DHT11 sensor variables */

  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  
  /* Clear input data array */

  dht11_data[0] = dht11_data[1] = dht11_data[2] = dht11_data[3] = dht11_data[4] = 0;

  /* Configures pin for output */
  
  pinMode(DHTPIN, OUTPUT);

  /* Start communication with sensor by sending start-signal (pulling pin down for 20 milliseconds and up for 40 microseconds) */

  digitalWrite(DHTPIN,LOW);
  delay(20);
  digitalWrite(DHTPIN,HIGH);
  delayMicroseconds(40);

  /* Configures pin for input */
  
  pinMode(DHTPIN,INPUT);

  /* Wait for response-signal (0 for MAXTIMINGS microseconds followed by 1 for MAXTIMINGS microseconds) */
  
  for (i = 0; i < MAXTIMINGS; i++) {
    counter = 0;
    while (digitalRead(DHTPIN) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) break;
    }
    laststate = digitalRead(DHTPIN);
    if (counter == 255) break;

    /* Ignore first 3 transitions */

    if ((i >= 4) && (i % 2 == 0)) {   
      dht11_data[j / 8] <<= 1; 
      if (counter > 16) dht11_data[j / 8] |= 1;
      j++;
    }
  }

  /* Check if 40 bits were read and verify checksum (last byte) */

  if ((j >= 40) && (dht11_data[4] == ((dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xFF))) {
    /* Get the current date & time */ 
    t_currentTime = time(NULL);
    t_Str = *localtime(&t_currentTime);
    return (1);
  }
  else return (0); 
}

/* --------------------------------------------------------------------------------------
 *
 * Function that mounts the data-vector and displays it on screen or output it to a file 
 *
 * --------------------------------------------------------------------------------------
 */

int WriteReading(void) {
  
  int i; /* local counter */

  char data_vector[25]; /* data vector */

  /* Time-stamp */

  char strTimeStampDay[3];
  char strTimeStampMonth[3];
  char strTimeStampYear[5];
  char strTimeStampHour[3];
  char strTimeStampMinute[3];
  char strTimeStampSecond[3];
  char strTimeStamp[15];

  /* Humidity and temperature */

  char strHumiTemp[9];

  /* Mount time-stamp string */

  sprintf(strTimeStampDay, "%02d", t_Str.tm_mday);
  sprintf(strTimeStampMonth, "%02d", t_Str.tm_mon + 1);
  sprintf(strTimeStampYear, "%04d", t_Str.tm_year + 1900);
  sprintf(strTimeStampHour, "%02d", t_Str.tm_hour);
  sprintf(strTimeStampMinute, "%02d", t_Str.tm_min);
  sprintf(strTimeStampSecond, "%02d", t_Str.tm_sec);
  
  sprintf(strTimeStamp, "%s%s%s%s%s%s", strTimeStampYear, strTimeStampMonth, strTimeStampDay, strTimeStampHour, strTimeStampMinute, strTimeStampSecond); 

  /* Mount humididty and temperature string */
 
  sprintf(strHumiTemp,"%02d%02d%02d%02d", dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3]); 
  
  /* Mount data-vector */ 

  sprintf(data_vector, "%s%s", strTimeStamp, strHumiTemp);

  /* Display readings */

  if (bVerbose) {
    printf("Reading #%d:\n", contReadings + 1);
    printf("   Date        -> %s-%s-%s\n", strTimeStampYear, strTimeStampMonth, strTimeStampDay);
    printf("   Time        -> %s:%s:%s\n", strTimeStampHour, strTimeStampMinute, strTimeStampSecond);
    printf("   Humidity    -> %02d.%02d %%\n", dht11_data[0], dht11_data[1]);
    printf("   Temperature -> %02d.%02d *C\n", dht11_data[2], dht11_data[3]);
    printf("\n   => data-vector: %s\n\n", data_vector);
  } 
  
  if (bOutputFile) fprintf(f_outputFile,"%s\n", data_vector);
  return (0);
}

/*----------------------------------------------------------
 *
 *                      Main function 
 *
 *----------------------------------------------------------
 */

int main(int argc, char *argv[]) {

  /* Counters */

  int i;
  int contMinutes = 0;

  /* Command line arguments */
  
  int nMinutes = 0;  /* Number of minutes */
  int nReadings = 0; /* Number of readings */
  int nInterval = 1; /* Interval (in seconds) between readings */

  char strTemp[10];  /* Temporary string */

  /* Display program instructions */

  if (((argc > 1 && (!strcmp(argv[1], "-?") || !strcmp(argv[1], "?") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "-H"))) || argc < 2) || argc > 6) {
    printf("\nDHT11_Sensor - V1.0\n\n");
    printf("Ronan Damasco - Jul/17\n\n");
    printf("Reads temperature and humidity from a DTH11 sensor and mount readings in a data-vector that can be displayed or written to a file\n\n");
    printf("Usage: dht11_sensor [?/-?/-h] [-v] [-m:xxxx] [-r:xxxx] [-t:xxxx] [-o:<output file>]\n\n");
    printf("Where: -?/?/-h: help\n");
    printf("       -v: verbose mode\n");
    printf("       -m: number of minutes (maximum 9,999, only the first 4 digits will be considered)\n");
    printf("       -r: number of readings (maximum 9,999, only the first 4 digits will be considered)\n");
    printf("       -t: interval, number of seconds between each read (maximum 9,999, only the first 4 digits will be considered)\n");
    printf("       -o: output file\n\n");
    printf("Obs: if both -m and -r are set, minutes have precedence. At least one must be specified.\n\n");
    printf("Return values:\n");
    printf("    0 - OK\n");
    printf("    1 - Error\n\n");
    exit (0);
  }
  
  /* Read line arguments */

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-V")) bVerbose = 1;
    
    if (!strncmp(argv[i], "-m:", 3) || !strncmp(argv[i], "-M:", 3)) {
      strncpy(strTemp, argv[i] + 3, 4); 
      nMinutes = atoi(strTemp);
    }
    
    if (!strncmp(argv[i], "-r:", 3) || !strncmp(argv[i], "-R:", 3)) {
      strncpy(strTemp, argv[i] + 3, 4); 
      nReadings = atoi(strTemp);
    }
    
    if (!strncmp(argv[i], "-t:", 3) || !strncmp(argv[i], "-T:", 3)) {
      strncpy(strTemp, argv[i] + 3, 4); 
      nInterval = atoi(strTemp);
    }

    if (!strncmp(argv[i], "-o:", 3) || !strncmp(argv[i], "-O:", 3)) {
      bOutputFile = 1;
      strncpy(strOutputFile, argv[i] + 3, 75); 
    }
  }

  /* Comment out to print line arguments for debugging purposes 

  printf("\nLine arguments:\n");   
  printf("   Verbose mode - %d\n", bVerbose);
  printf("   Number of minutes - %d\n", nMinutes);
  printf("   Number of readings - %d\n", nReadings);
  printf("   Interval in minutes - %d\n", nInterval);
  if (bOutputFile) printf("   Output file - %s\n", strOutputFile);
  printf("\n");

  */

  /* Initializes the wiringPI library */
  
  if (wiringPiSetup() == -1) {
    printf("Error initializing the Raspberry Pi wiringPi library\n");
    exit(1);
  }

  /* Open output file */

  if (bOutputFile) {
    f_outputFile = fopen(strOutputFile, "w");
    if (f_outputFile == NULL) {
      printf("Error creating output file <%s>\n", strOutputFile);
      exit(1);
    }
  }

  /* Program execution loop */
 
  if (bVerbose) printf("\nBeginning execution\n\n");

  /* Reading loop when number of minutes specified */
  
  if (nMinutes > 0) {
    struct timespec t_start, t_end;
    double t_diff;

    clock_gettime(CLOCK_MONOTONIC, &t_start);

    while (contMinutes < nMinutes) {
      if (read_DTH11_data()) {
        if (!WriteReading()) {
          delay(nInterval * 1000); /* Wait nInterval secs to read again */
          contReadings++;
        }
      }
      else delay(1000);

      clock_gettime(CLOCK_MONOTONIC, &t_end);
      t_diff = (t_end.tv_sec - t_start.tv_sec); 
      if (t_diff >= 60) {
        contMinutes++;
        clock_gettime(CLOCK_MONOTONIC, &t_start);
      }
    }
      
    if (bOutputFile) fclose(f_outputFile);
    if (bVerbose) printf("Finished execution, %d readings perfomed\n\n", contReadings);
    exit (0);
  }

  /* Reading loop when number of readings specified */

  if (nReadings > 0) {
    while (contReadings < nReadings) {
      if (read_DTH11_data()) { 
        if (!WriteReading()) {
          delay(nInterval * 1000); /* Wait nInterval secs to read again */
          contReadings++;
        }
      }
      else delay(1000);
    }
    
    if (bOutputFile) fclose(f_outputFile);  
    if (bVerbose) printf("Finished execution, %d readings perfomed\n\n", contReadings);
    exit (0);
  }

  printf("No temperature and humidity was read from sensor, specify either number minutes or readings\n\n");
  exit(1);
}