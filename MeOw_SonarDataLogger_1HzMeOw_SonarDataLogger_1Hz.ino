
// MeOw Station Ultrasonic Distance Sensor Data Logging

// Version 2 - 1 Hz sampling option, no sleep

// Takes readings from two Maxbotix MB7389 sonars and prints it on the serial monitor
// Records data on microSD card; uses a SSD1306 OLED display connected to the Mayfly's I2C Grove connector

// Sensor A
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D6
// Sonar 5 (Serial) to Mayfly D10

// Sensor B
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D7
// Sonar 5 (Serial) to Mayfly D11

// Ian Reeves and Katherine Anarde
// July 2, 2021


/////////////////////////////////////////////  SET DATA HEADER  //////////////////////////////////////////////

#define LOGGERNAME "MeOw Station [5]"
#define PROJECT_LOCATION "Project & Location: DUNEX - Pea Island, NC"
#define INSTALL_DATE "Installation Date: September 13, 2021"
#define DATA_HEADER "DateTime,UnixTime,Battery_V,SonarRange_A_mm,SonarRange_B_mm"
#define FILE_NAME "DataLog5.txt"  

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

SoftwareSerial sonarSerial_A(10, -1);  
SoftwareSerial sonarSerial_B(11, -1);      

const int SonarExcite_A = 6;
const int SonarExcite_B = 7;

boolean stringComplete_A = false;
boolean stringComplete_B = false;

String dataRec = "";
int currentminute;
int currentsecond;
long currentunixtime = 0;
uint32_t old_ts;

int batteryPin = A6;    //Select the input pin for the potentiometer
int batterysenseValue = 0;  //Variable to store the value coming from the sensor
float batteryvoltage;
float batterypercent;

int range_mm_A;
int range_mm_B;

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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println();
  display.println(LOGGERNAME);
  display.display();
  
  //Initialise the sonar serial connection
  sonarSerial_A.begin(9600);
  sonarSerial_B.begin(9600);
  delay(100);
  
  //Set mode for LEDs
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  //Set mode for powering sonars
  pinMode(SonarExcite_A, OUTPUT);   
  pinMode(SonarExcite_B, OUTPUT);

  //Turn sonars on
  delay(50);    
  digitalWrite(SonarExcite_A, HIGH);  
  delay(50);  
  digitalWrite(SonarExcite_B, HIGH);    
  delay(50);  

  //Blink the LEDs to show the board is on
  greenred4flash();
  
  setupLogFile();
  
  Serial.println("Power On, running");
  Serial.println("Date,Time,UnixTime,Battery_V,SonarRange_A_mm,SonarRange_B_mm");


}

void loop() 
{   
    DateTime now = rtc.now(); //Get the current date-time
    uint32_t ts = now.getEpoch();

    if (old_ts == 0 || old_ts != ts) {
          old_ts = ts;
          dataRec = createDataRecord(ts);

          ////////////////   take Sensor A reading   ////////////////
          range_mm_A = SonarRead_A();
          stringComplete_A = false; 

          ////////////////  take Sensor B reading  ////////////////
          range_mm_B = SonarRead_B(); 
          stringComplete_B = false;        
          
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
          display.print("SensorA:    ");
          display.println(range_mm_A);
          display.print("SensorB:  ");
          display.println(range_mm_B);
          display.display();
   
          String dataRec = "";   
     }
}


String getDateTime(uint32_t ts)
{
  String dateTimeStr;
  
  //Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(ts));

  currentunixtime = (dt.get()) + 946684800;    // Unix time in seconds 

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

//void greenflash()
//{
//  digitalWrite(8, HIGH);   
//  delay(150);
//  digitalWrite(8, LOW);
//  delay(150);
//  digitalWrite(8, HIGH);   
//  delay(150);
//  digitalWrite(8, LOW);
//  delay(150);
//}

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
    //Date, Time, UnixTime, BatteryV, Battery%, Distance_A, Distance_B
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


int SonarRead_A() 
{
  bool stringComplete_A = false;
  int result_A = 0;

  sonarSerial_A.listen();

  result_A = sonarSerial_A.parseInt();
  sonarSerial_A.read();  //To throw away the carriage return
  
  dataRec += ",";    
  dataRec += result_A;

  return result_A;
}


int SonarRead_B() 
{
  bool stringComplete_B = false;
  int result_B = 0;

  sonarSerial_B.listen();

  result_B = sonarSerial_B.parseInt();
  sonarSerial_B.read();  // To throw away the carriage return
  
  dataRec += ",";    
  dataRec += result_B;

  return result_B;
}
