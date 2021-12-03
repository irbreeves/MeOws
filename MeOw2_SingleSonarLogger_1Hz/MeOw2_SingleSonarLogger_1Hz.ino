// MeOw Station Ultrasonic Distance Sensor Data Logging

// Version 7.2 - 1 Hz sampling option, no sleep, single sonar

// Takes readings from a single Maxbotix MB7389 sonar and prints it on the serial monitor
// Records data on microSD card; uses a SSD1306 OLED display connected to the Mayfly's I2C Grove connector

// Sensor Pins
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly 11
// Sonar 5 (Serial) to Mayfly D10

// Ian Reeves and Katherine Anarde
// September 3, 2021


/////////////////////////////////////////////  SET DATA HEADER  //////////////////////////////////////////////

#define LOGGERNAME "MeOw Station [2]"
#define PROJECT_LOCATION "Project & Location: DUNEX - Pea Island, NC"
#define INSTALL_DATE "Installation Date: October 9, 2021"
#define DATA_HEADER "DateTime,UnixTime,Battery_V,SonarRange_mm"
#define FILE_NAME "DataLog2.txt"  //NOTE: this cannot have underscores

/////////////////////////////////////////////////////////////////////////////////////////////////////////////  

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include <SDL_Arduino_SSD1306.h>
#include <AMAdafruit_GFX.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_PcInt_PCINT0.h>
#include <SoftwareSerial_PCINT12.h>

SoftwareSerial sonarSerial(11, -1);

const int SonarExcite = 10;

String dataRec = "";
int currentminute;
int currentsecond;
long currentunixtime = 0;
uint32_t old_ts;

int batteryPin = A6;    //Select the input pin for the potentiometer
int batterysenseValue = 0;  //Variable to store the value coming from the sensor
float batteryvoltage;
float batterypercent;

int range_mm;

//SD pin
#define SD_SS_PIN 12

//I2C pin
SDL_Arduino_SSD1306 display(4);
 

void setup() 
{
  //Initialize serial display
  Serial.begin(57600);
  Wire.begin();
  rtc.begin();
  
  //Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  //Initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println();
  display.println(LOGGERNAME);
  display.display();

  //Set mode for LEDs
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  //Set mode for powering sonars
  pinMode(SonarExcite, OUTPUT);   

  //Turn sonars on
  delay(50);    
  digitalWrite(SonarExcite, HIGH);  
  delay(50);  

  //Blink the LEDs to show the board is on
  greenred4flash();
  
  setupLogFile();
  
  Serial.println("Power On, running");
  Serial.println("Date,Time,UnixTime,Battery_V,SonarRange_mm");


}

void loop() 
{   
    DateTime now = rtc.now(); //Get the current date-time
    uint32_t ts = now.getEpoch();

    if (old_ts == 0 || old_ts != ts) {
          old_ts = ts;
          dataRec = createDataRecord(ts);

          //Take sensor reading
          range_mm = SonarRead();
          
          //Save the data record to the log file   
          logData(dataRec);
          
          //Display data via the serial connection
          Serial.println();
          Serial.print("Data Record: ");
          Serial.println(dataRec);

          //Display data via the OLED display connection
          String displaytime = getDateTime(ts);
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println();
          display.println(displaytime);
          display.println();
          display.print("Distance:    ");
          display.println(range_mm);
          display.display();
   
          String dataRec = "";   
     }
}


String getDateTime(uint32_t ts)
{
  String dateTimeStr;
  
  //Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(ts));

  currentunixtime = (dt.get()) + 946684800;    //Unix time in seconds 

  currentminute = (dt.minute());
  currentsecond = (dt.second());
  
  //Convert it to a String
  dt.addToString(dateTimeStr);
   
  return dateTimeStr;  
}


void greenred4flash()
{
  for (int i=1; i <= 4; i++){
  digitalWrite(8, HIGH);   
  digitalWrite(9, LOW);
  delay(50);
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
  delay(50);
  }
  digitalWrite(9, LOW);
}


void setupLogFile()
{
  //Initialise the SD card
  if (!SD.begin(SD_SS_PIN))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
    display.println("Error: SD card failed to initialise or is missing.");
    display.display();
  }
  
  //Check if the file already exists
  bool oldFile = SD.exists(FILE_NAME);  
  
  //Open the file in write mode
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  //Add header information if the file did not already exist
  if (!oldFile)
  {
    logFile.println(LOGGERNAME);
    logFile.println(PROJECT_LOCATION);
    logFile.println(INSTALL_DATE);
    logFile.println(DATA_HEADER);
  }
  
  //Close the file to save it
  logFile.close();  
}

void logData(String rec)
{
  //Re-open the file
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  //Write the CSV data
  logFile.println(rec);
  
  //Close the file to save it
  logFile.close();  
}

String createDataRecord(uint32_t ts)
{
    //Create a String type data record in csv format
    //Date, Time, UnixTime, BatteryV, Distance
    String data = getDateTime(ts);

    data += ",";  
    
    batterysenseValue = analogRead(batteryPin);
    batteryvoltage = (3.3/1023.) * 4.7 * batterysenseValue;

    data += currentunixtime;
    data += ",";

    addFloatToString(data, batteryvoltage, 4, 2);
  
    return data;
}


static void addFloatToString(String & str, float val, char width, unsigned char precision)
{
  char buffer[10];
  dtostrf(val, width, precision, buffer);
  str += buffer;
}


int SonarRead() 
{
  bool stringComplete = false;
  int rangeAttempts = 0;
  int result = 0;

  sonarSerial.begin(9600);  //Initialize the sonar serial connection
  //delay(100);

  sonarSerial.listen();
  while (stringComplete == false && rangeAttempts < 5)
  {
      result = sonarSerial.parseInt();
      sonarSerial.read();  //To throw away the carriage return

      rangeAttempts++;

      //In case of bad return, continue while loop and re-take reading up to 4 more times
      if (result >= 300 && result <= 4999)
      {
          stringComplete = true;  //Set completion of read to true and exit while loop
      }
      
  }
  dataRec += ",";    
  dataRec += result;

  sonarSerial.end();

  return result;
}
