// MeOw Station Ultrasonic Distance Sensor Data Logging v.8

// Takes readings from two Maxbotix MB7389 sonars and prints to a serial monitor; records data on microSD card.

// v.8 Takes samples at 2 Hz; Mayfly does not sleep

// Bed-Level Sensor
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D6
// Sonar 5 (Serial) to Mayfly D10

// Water-Level Sensor
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D7
// Sonar 5 (Serial) to Mayfly D11

// Ian Reeves 
// 2 July 2021


/////////////////////////////////////////////  SET DATA HEADER  //////////////////////////////////////////////

#define LOGGERNAME "MeOw Station [3]"
#define PROJECT_LOCATION "Project & Location: New Code v.8 Test"
#define INSTALL_DATE "Installation Date: 2 July 2021"
#define DATA_HEADER "DateTime,UnixTime,Battery_V,SonarRangeA_mm,SonarRangeB_mm"
#define FILE_NAME "DataLog3.txt"  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////  


#include <Arduino.h>
#include <Sodaq_PcInt_PCINT0.h>
#include <SoftwareSerial_PCINT12.h>
#include <Sodaq_DS3231.h>
#include <SDL_Arduino_SSD1306.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <AMAdafruit_GFX.h>

#define RTC_PIN A7
#define SD_SS_PIN 12

SoftwareSerial sonarSerialA(10, -1);  // Define serial port for recieving data.
SoftwareSerial sonarSerialB(11, -1);  // Define serial port for recieving data.

// I2C pin
SDL_Arduino_SSD1306 display(4);

const int SonarExciteA = 6;
const int SonarExciteB = 7;

int batteryPin = A6;    // Select the input pin for the potentiometer
int batterysenseValue = 0;  // Variable to store the value coming from the sensor
float batteryvoltage;

String dataRec = "";
int currentminute;
int currentsecond;
long currentunixtime = 0;
int rangeA_mm;
int rangeB_mm;
int halfsec;
static uint32_t nowsec;
static uint32_t lastsec;
static uint32_t nowmillis;
static uint32_t startmillis;

void setup()
{
    //Initialize serial display
    Serial.begin(57600);
    
    //Initialize OLED display
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  // initialize with the I2C addr 0x3C (for the 128x64)
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println();
    display.println(LOGGERNAME);
    display.display();
     
    // Initialize sonar serial connection
    sonarSerialA.begin(9600);  // Start serial port for maxSonar
    sonarSerialB.begin(9600);  // Start serial port for maxSonar
    delay(250);
    
    // Initial RTC
    Wire.begin();
    rtc.begin();
    delay(250);

    //Set mode for LEDs
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    
    // Power sonars
    pinMode(SonarExciteA, OUTPUT);
    digitalWrite(SonarExciteA, HIGH); 
    pinMode(SonarExciteB, OUTPUT);  
    digitalWrite(SonarExciteB, HIGH);

    //Blink the LEDs to show the board is on
    greenred4flash();
    
    setupLogFile();
    
    Serial.println("Power On, running");
    Serial.println("Date,Time,UnixTime,Battery_V,SonarRangeA_mm,SonarRangeB_mm");
}

void loop()
{
    DateTime now = rtc.now(); // Get the current date-time
    nowsec = now.getEpoch();  // Get current second
    
    if (lastsec == 0 || lastsec != nowsec) // Every minute
    {
      startmillis = millis(); // Get current milliseconds from start of this current minute
      lastsec = nowsec;
      
      //////////////////////////////////// TAKE FIRST READINGS ////////////////////////////////////
      halfsec = 0; // Determines whether this sample is on the half or whole second (0 = whole, 1 = half)
      dataRec = createDataRecord();
      
      // Take reading A
      rangeA_mm = parseSonarA();
      
      // Add result to dataRec
      dataRec += ",";    
      dataRec += rangeA_mm;
      
      // Take reading B
      rangeB_mm = parseSonarB();

      // Add result to dataRec
      dataRec += ",";    
      dataRec += rangeB_mm;
      
      // Save the data record to the log file   
      logData(dataRec);
          
      // Display data via the serial connection
      Serial.println();
      Serial.print("Data Record: ");
      Serial.println(dataRec);

      // Display data via the OLED display connection
      String displaytime = getDateTime();
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println();
      display.println(displaytime);
      display.println();
      display.print("Range A:    ");
      display.println(rangeA_mm);
      display.print("Range B:    ");
      display.println(rangeB_mm);
      display.display();
  
      String dataRec = ""; // Reset


      //////////////////////////////////// TAKE SECOND READINGS ////////////////////////////////////
      nowmillis = millis(); // Get new current milliseconds
      
      // Delay start of second reading by half a second, adjusting for duration of first reading
      delay(500 - (nowmillis - startmillis));

      halfsec = 1;
      dataRec = createDataRecord();
      
      // Take reading A
      rangeA_mm = parseSonarA();
      
      // Add result to dataRec
      dataRec += ",";    
      dataRec += rangeA_mm;
      
      // Take reading B
      rangeB_mm = parseSonarB();
      
      // Add result to dataRec
      dataRec += ",";    
      dataRec += rangeB_mm;

      // Save the data record to the log file   
      logData(dataRec);
          
      // Display data via the serial connection
      Serial.println();
      Serial.print("Data Record: ");
      Serial.println(dataRec);

      // Only display via the OLED for first reading
  
      dataRec = "";
      
    }

    // Flash green LED every 10 seconds
    if(nowsec % 10 == 0) {quickgreenflash();} 
    
}


int parseSonarA(void)
{
    int result = 0;
    sonarSerialA.listen();
    result = sonarSerialA.parseInt();
    sonarSerialA.read();
    
    return result;
}

int parseSonarB(void)
{
    int result = 0;
    sonarSerialB.listen();
    result = sonarSerialB.parseInt();
    sonarSerialB.read();
    
    return result;
}

String getDateTime()
{
  String dateTimeStr;
  
  // Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(rtc.now().getEpoch()));

  currentunixtime = (dt.get()) + 946684800; // Unix time in seconds 

  currentminute = (dt.minute());
  currentsecond = (dt.second());

  // Convert it to a String
  dt.addToString(dateTimeStr);
   
  return dateTimeStr;  
}


uint32_t getNow()
{
  currentunixtime = rtc.now().getEpoch() + 946684800; // Converted to UnixTime
  return currentunixtime;
}


void setupLogFile()
{
  // Initialise the SD card
  if (!SD.begin(SD_SS_PIN))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
    display.println("Error: SD card failed to initialise or is missing.");
    display.display();
  }
  
  // Check if the file already exists
  bool oldFile = SD.exists(FILE_NAME);  
  
  // Open the file in write mode
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  // Add header information if the file did not already exist
  if (!oldFile)
  {
    logFile.println(LOGGERNAME);
    logFile.println(PROJECT_LOCATION);
    logFile.println(INSTALL_DATE);
    logFile.println(DATA_HEADER);
  }
  
  // Close the file to save it
  logFile.close();  
}

void logData(String rec)
{
  // Re-open the file
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  // Write the CSV data
  logFile.println(rec);
  
  // Close the file to save it
  logFile.close();  
}

String createDataRecord()
{
    // Create a String type data record in csv format
    // Date, Time, UnixTime, BatteryV, Distance_A, Distance_B
    String data = getDateTime();

    // Add half second to time, if applicable
    if(halfsec == 1)
    {
      data += ".5";
    }
    else
    {
      data += ".0";
    }
    data += ",";  

    // Sample battery
    batterysenseValue = analogRead(batteryPin);
    batteryvoltage = (3.3/1023.) * 4.7 * batterysenseValue;

    // Add half second to unixtime, if applicable
    data += currentunixtime;
    if(halfsec == 1)
    {
      data += ".5";
    }
    else
    {
      data += ".0";
    }
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

void greenflash()
{
  digitalWrite(8, HIGH);   
  delay(150);
  digitalWrite(8, LOW);
  delay(150);
  digitalWrite(8, HIGH);   
  delay(150);
  digitalWrite(8, LOW);
  delay(150);
}

void quickgreenflash()
{
  digitalWrite(8, HIGH);   
  delay(25);
  digitalWrite(8, LOW);
}
