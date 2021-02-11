
// MeOw Station Ultrasonic Distance Sensor Data Logging v.6

// Takes readings from two Maxbotix MB7389 sonars, one for water elevation and one for bed elevation, and 
// prints it on the serial monitor; records data on microSD card; puts Mayfly to sleep in between samples.

// v.6 Uses a SSD1306 OLED display connected to the Mayfly's I2C Grove connector

// Bed-Level Sensor
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D6
// Sonar 5 (Serial) to Mayfly D10

// Water-Level Sensor
// Sonar 7 (Ground) to Mayfly Grnd
// Sonar 6 (Excite) to Mayfly D7
// Sonar 5 (Serial) to Mayfly D11

// Ian Reeves 
// 30 August 2018


/////////////////////////////////////////////  SET DATA HEADER  //////////////////////////////////////////////

#define LOGGERNAME "MeOw Station [5]"
#define PROJECT_LOCATION "Project & Location: Spartina, Smith Island NC"
#define INSTALL_DATE "Installation Date: 20-22 August 2019"
#define DATA_HEADER "DateTime,UnixTime,BoardTemp_C,Battery_V,SonarRange_Bed_mm,SonarRange_Water_mm"
#define FILE_NAME "DataLog5.txt"  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////  

#include <Wire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>

#include <Arduino.h>
#include <SDL_Arduino_SSD1306.h>
#include <AMAdafruit_GFX.h>

#include <RTCTimer.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_PcInt_PCINT0.h>

#include <SoftwareSerial_PCINT12.h>
const int SonarExcite_Bed = 6;
const int SonarExcite_Water = 7;
SoftwareSerial sonarSerial_Bed(10, -1);
SoftwareSerial sonarSerial_Water(11, -1);      
 
boolean stringComplete_Bed = false;
boolean stringComplete_Water = false;
 
#define READ_DELAY 1
 
//RTC Timer
RTCTimer timer;

String dataRec = "";
int currentminute;
int currentsecond;
long currentunixtime = 0;
float boardtemp = 0.0;

int batteryPin = A6;    //Select the input pin for the potentiometer
int batterysenseValue = 0;  //Variable to store the value coming from the sensor
float batteryvoltage;
float batterypercent;

int range_mm_Bed;
int range_mm_Water;

//RTC Interrupt pin
#define RTC_PIN A7
#define RTC_INT_PERIOD EveryMinute

#define SD_SS_PIN 12

//I2C pin
SDL_Arduino_SSD1306 display(4);
 

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
  
  //Initialise the sonar serial connection
  sonarSerial_Bed.begin(9600);
  sonarSerial_Water.begin(9600);
  rtc.begin();
  delay(100);
  
  //Set mode for LEDs
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  //Bed
  pinMode(SonarExcite_Bed, OUTPUT);
  digitalWrite(SonarExcite_Bed, LOW);  //Power pin for the ultrasonic sensor

  //Water
  pinMode(SonarExcite_Water, OUTPUT);
  digitalWrite(SonarExcite_Water, LOW);  //Power pin for the ultrasonic sensor

  //Blink the LEDs to show the board is on
  greenred4flash();
  
  setupLogFile();

  //Setup timer events
  setupTimer();
  
  //Setup sleep mode
  setupSleep();
  
  Serial.println("Power On, running");
  Serial.println("Date,Time,UnixTime,BoardTemp_C,Battery_V,SonarRange_Bed_mm,SonarRange_Water_mm");

}

void loop() 
{   
  //Update the timer 
  timer.update();
  
  if(currentminute % 1 == 0)    //Sample Frequency
     {    
          digitalWrite(9, HIGH);  //Red LED on while taking samples
          dataRec = createDataRecord();


          ////////////////   Bed   ////////////////
          delay(200);    
          digitalWrite(SonarExcite_Bed, HIGH);
          delay(400); 
          range_mm_Bed = SonarRead_Bed();
          digitalWrite(SonarExcite_Bed, LOW);
          stringComplete_Bed = false; 

          ////////////////  Water  ////////////////
          delay(200);  
          digitalWrite(SonarExcite_Water, HIGH);
          delay(400);  
          range_mm_Water = SonarRead_Water(); 
          digitalWrite(SonarExcite_Water, LOW); 
          stringComplete_Water = false; 
          
          /////////////////////////////////////////          
          
 
          //Save the data record to the log file   
          logData(dataRec);
          
          //Display data via the serial connection
          Serial.println();
          Serial.print("Data Record: ");
          Serial.println(dataRec);

          //Display data via the OLED display connection
          String displaytime = getDateTime();
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println();
          display.println(displaytime);
          display.println();
          display.print("Bed:    ");
          display.println(range_mm_Bed);
          display.print("Water:  ");
          display.println(range_mm_Water);
          display.display();
   
          String dataRec = "";   
     
          digitalWrite(9, LOW);  //Turn off Red LED when done taking samples
          greenflash();  //Green LED to notify successful samples taken
          delay(200);
     }
    
  systemSleep();
}

void showTime(uint32_t ts)
{
  //Retrieve and display the current date/time
  String dateTime = getDateTime();
  //Serial.println(dateTime);
}

void setupTimer()
{
  
  //Schedule the wakeup every minute
  timer.every(READ_DELAY, showTime);
  
  //Instruct the RTCTimer how to get the current time reading
  timer.setNowCallback(getNow);


}

void wakeISR()
{
  //Leave this blank
}

void setupSleep()
{
  pinMode(RTC_PIN, INPUT_PULLUP);
  PcInt::attachInterrupt(RTC_PIN, wakeISR);

  //Setup the RTC in interrupt mode
  rtc.enableInterrupts(RTC_INT_PERIOD);
  
  //Set the sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void systemSleep()
{  
  //Wait until the serial ports have finished transmitting
  Serial.flush();
  Serial1.flush();
  display.flush(); //Needed?
  
  //The next timed interrupt will not be sent until this is cleared
  rtc.clearINTStatus();
    
  //Disable ADC
  ADCSRA &= ~_BV(ADEN);
  
  //Sleep time
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
 
  //Enbale ADC
  ADCSRA |= _BV(ADEN);
  
}

String getDateTime()
{
  String dateTimeStr;
  
  //Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(rtc.now().getEpoch()));

  currentunixtime = (dt.get()) + 946684800;    //Unix time in seconds 

  currentminute = (dt.minute());
  currentsecond = (dt.second());
  //Convert it to a String
  dt.addToString(dateTimeStr);
   
  return dateTimeStr;  
}

uint32_t getNow()
{
  currentunixtime = rtc.now().getEpoch() + 946684800; // Converted to UnixTime
  return currentunixtime;
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

void setupLogFile()
{
  //Initialise the SD card
  if (!SD.begin(SD_SS_PIN))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
    display.println("Error: SD card failed to initialise or is missing.");
    display.display();
    //Hang
    //while (true); 
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

String createDataRecord()
{
    //Create a String type data record in csv format
    //Date, Time, UnixTime, Boardtemp, BatteryV, Battery%, Distance_Bed, Distance_Water
    String data = getDateTime();

    data += ",";  
  
    rtc.convertTemperature();          //Convert current temperature into registers
    boardtemp = rtc.getTemperature();  //Read temperature sensor value
    
    batterysenseValue = analogRead(batteryPin);
    batteryvoltage = (3.3/1023.) * 4.7 * batterysenseValue;

    data += currentunixtime;
    data += ",";

    addFloatToString(data, boardtemp, 3, 1); 
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


int SonarRead_Bed() 
{
  bool stringComplete_Bed = false;
  int rangeAttempts_Bed = 0;
  int result_Bed = 0;

  sonarSerial_Bed.listen();

  while (stringComplete_Bed == false && rangeAttempts_Bed < 11)
  {
      result_Bed = sonarSerial_Bed.parseInt();
      sonarSerial_Bed.read();  //To throw away the carriage return

      rangeAttempts_Bed++;

      //In case of bad return, continue while loop and re-take reading up to 9 more times
      if (result_Bed != 0 || result_Bed != 300 || result_Bed != 4999)
      {
          stringComplete_Bed = true;  //Set completion of read to true and exit while loop
      }
  }
  
  dataRec += ",";    
  dataRec += result_Bed;

  return result_Bed;
}


int SonarRead_Water() 
{
  bool stringComplete_Water = false;
  int rangeAttempts_Water = 0;
  int result_Water = 0;

  sonarSerial_Water.listen();

  while (stringComplete_Water == false && rangeAttempts_Water < 11)
  {
      result_Water = sonarSerial_Water.parseInt();
      sonarSerial_Water.read();  // To throw away the carriage return

      rangeAttempts_Water++;

      //In case of bad return, continue while loop and re-take reading up to 9 more times
      if (result_Water != 0 || result_Water != 300 || result_Water != 4999)
      {
          stringComplete_Water = true;  //Set completion of read to true and exit while loop
      }
  }
  
  dataRec += ",";    
  dataRec += result_Water;

  return result_Water;
}
