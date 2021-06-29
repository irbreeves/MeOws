// This Mayfly sketch parses data from MaxSonar serial data and prints it on the serial monitor

// To use this sketch, connect the MaxBotix serial data output pin (pin 5) to Mayfly pin D5.
// Connect the MaxBotix power pin (pin 6) to the Vcc pin next to Mayfly pin D5.
// Connect the MaxBotix power pin (pin 7) to the ground pin near Mayfly pin D5.
// Set the jumper controlling power to pins D4-5 to be continuously powered.
// Leave all other pins on the MaxBotix unconnected

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RTCTimer.h>
#include <Sodaq_DS3231.h>

SoftwareSerial sonarSerial(5, -1);  // Define serial port for recieving data.

int currentminute;
int currentsecond;
long milliTime = 0;
long t = 0;
long currentunixtime = 0;
static uint32_t oldtime=millis();


//int parseSonar(void)
//{
//    bool stringComplete = false;
//    int rangeAttempts = 0;
//    int result = 0;
//
//    // Serial.println(F("Beginning detection for Sonar"));  // For debugging
//    while (stringComplete == false && rangeAttempts < 1)
//    {
//        result = sonarSerial.parseInt();
//        sonarSerial.read();  // To throw away the carriage return
//
//        Serial.print("Range: ");
//        Serial.print(result);
//        Serial.println(" mm");
//
//        rangeAttempts++;
//
//        // If it cannot obtain a result , the sonar is supposed to send a value
//        // just above it's max range.  For 10m models, this is 9999, for 5m models
//        // it's 4999.  The sonar might also send readings of 300 or 500 (the
//        //  blanking distance) if there are too many acoustic echos.
//        // If the result becomes garbled or the sonar is disconnected, the parseInt function returns 0.
//        if (result == 0 || result == 300 || result == 500 || result == 4999 || result == 9999)
//        {
//            Serial.print(F("Bad or Suspicious Result, Retry Attempt #"));  // For debugging
//            Serial.println(rangeAttempts);  // For debugging
//        }
//        else
//        {
//            //Serial.println(F("Good result found"));  // For debugging
//            stringComplete = true;  // Set completion of read to true
//        }
//    }
//    return result;
//}


int parseSonar(void)
{
    int result = 0;

    result = sonarSerial.parseInt();
    sonarSerial.read();  // To throw away the carriage return

    Serial.print("Range: ");
    Serial.print(result);
    Serial.println(" mm");

    return result;
}


void setup()
{
    Serial.begin(57600);  // Start serial port for display
    rtc.begin();
    pinMode(5, INPUT);  // Set the pin mode for the software serial port
    sonarSerial.begin(9600);  // Start serial port for maxSonar
    sonarSerial.setTimeout(200);  // Set a timeout for the serial instance
    // Even the slowest sensors should respond at a rate of 6Hz (166ms).
    
    Serial.println("Mayfly MaxBotix sonar sensor rangefinder example");

    Serial.println("Date/Time: ");
    Serial.print(getDateTime());

}

void loop()
{
    if (millis() - oldtime >= 1000)
    {
    oldtime = millis();
    Serial.print("Time");
    Serial.println(oldtime);
    parseSonar();
    }
}


//void loop()
//{
//    if (currentsecond % 1 == 0)
//    {
//    Serial.print("Time");
//    Serial.println(currentsecond);
//    parseSonar();
//    }
//}


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
