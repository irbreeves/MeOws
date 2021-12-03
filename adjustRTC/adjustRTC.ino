// Date and time functions using a RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "Sodaq_DS3231.h"

char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

//24 Hour clock!
//year, month, day, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also. Monday is 1, Tuesday 2, etc.
//Follow instructions here for setting RTC with this script: https://github.com/EnviroDIY/Sodaq_DS3231/tree/master/examples/adjust
//Must use double-0 not single 0
DateTime dt(2021, 11, 19, 15, 44, 00, 5);

//NOTE: Ian did this for MeOws1-6 for DUNEX on September 2 using his desktop computer.
//Instead of using the reset button, he just timed the time it takes to upload the 
//sketch from his computer, which was consistently ~8 seconds. MADE EDT.


void setup () 
{
    Serial.begin(57600);
    Wire.begin();
    rtc.begin();
    rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
}

void loop () 
{
    DateTime now = rtc.now(); //get the current date-time
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.date(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print(weekDay[now.dayOfWeek()]);
    Serial.println();
    delay(1000);
}
