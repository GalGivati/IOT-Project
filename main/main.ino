#include "neopixel_utils.h"
#include "time_utils.h"
#include "weather_utils.h"

// ----------------------------------------------------------

/*
    defines and globals declaration
*/

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

/* Bluetooth includes and globals */
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
String device_name = "WordClock-YRG";
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

/* Wifi includes and globals */
#include <WiFi.h>
String ssid       = "";
String password  = "";


/* Preferences */
#include <Preferences.h>
Preferences preferences;

/*
    end of defines and globals declaration
*/

// ----------------------------------------------------------

/*
    helper functions
*/

/* wifi credentials parser */
void getNameandPass(String output)
{
  int index = output.indexOf(",");
  ssid = output.substring(5, index);
  password = output.substring(index+1);
}

/* location settings parser */
void getCity(String output){
  int index = output.indexOf("+");
  city = output.substring(5, index);
  countryCode = output.substring(index+1);
  Serial.println(city);
  Serial.println(countryCode);
}

/*
    End of helper functions
*/

// ----------------------------------------------------------

/*
    Alarm handling functions
*/

void printAlarmCount() {
  Serial.print(F("AlarmCount = "));
  Serial.println(alarmCount);
}

void putAlarm(int index, String alarm)
{
  preferences.putString(("alarm" + String(index)).c_str(), alarm);
}

String getAlarm(int index) {
  return preferences.getString(("alarm" + String(index)).c_str());
}

/* check if alarm exist and return its index if exist */
int findAlarm(String alarm) {
  for (int i = 0; i < alarmCount; i++) {
    if (alarm == getAlarm(i)) {
      return i;
    }
  }
  return -1;
}

/* add an alarm */
void addAlarm(String alarm) {
  int index = findAlarm(alarm);
  if (index != -1) {
    return;
  }
  putAlarm(alarmCount, alarm);
  alarmCount++;
  preferences.putInt("alarmCount", alarmCount);
  printAlarmCount();
  return;
}

/* remove an alarm */
void removeAlarm(String alarm)
{
  int index = findAlarm(alarm);
  if (index == -1) {
    return;
  }
  if (index != alarmCount -1) {
    String lastAlarm = getAlarm(alarmCount-1);
    putAlarm(index, lastAlarm);
  }
  alarmCount--;
  preferences.putInt("alarmCount", alarmCount);
  printAlarmCount();
}

/* stop an alarm */
void stopAlarm(String alarm)
{
  if (indexAlarm == -1) {
    return;
  }
  String currentAlarm = getAlarm(indexAlarm);
  if (currentAlarm != alarm) {
    return;
  }
  alarmToStop = alarm;
}

/*
    End of alarm handling functions
*/

// ----------------------------------------------------------

/*
    Setup functions
*/

/* Set theme for the words that descrive clock and lights the describe minutes- ascending and descending */
void setTheme(String theme)
{
  Serial.println(theme);
  JSONVar my_json = JSON.parse(theme);
  
  JSONVar words_json = my_json["Words"];
  day_words_color = pixels.Color(words_json["Green"], words_json["Red"], words_json["Blue"]);
  
  JSONVar asc_minutes_json = my_json["Asc"];
  day_asc_color = pixels.Color(asc_minutes_json["Green"], asc_minutes_json["Red"], asc_minutes_json["Blue"]);
  
  JSONVar dsc_minutes_json = my_json["Dsc"];
  day_dsc_color = pixels.Color(dsc_minutes_json["Green"], dsc_minutes_json["Red"], dsc_minutes_json["Blue"]);

  light_time(last_hour,last_minute);
}

void setup_preferences() {
    preferences.clear();
    preferences.begin("credentials", false);
    gmtOffset_sec = preferences.getInt("gmtOffset", 0);
    alarmCount = preferences.getInt("alarmCount", 0);
}

/* setup the word clock */
void setup() {
  Serial.begin(9600);
  setup_neopixel();
  setup_preferences();
  SerialBT.begin(device_name); // begin the bluetooth
  setup_time();
  pixels.clear(); // Set all pixel colors to 'off'
}

/*
    End of setup functions
*/

// ----------------------------------------------------------

/*
    Loop functions
*/

/* bluetooth loop accepts user inputs and acts accordingly */
void bluetooth_loop()
{
  if (SerialBT.available()) {
    String output = SerialBT.readString();
    output.trim();
    Serial.println(output);

    // Update WIFI credentials
    if (output.indexOf(F("SSID")) == 0) {
      getNameandPass(output);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      WiFi.disconnect();
      wifi_not_connected();
      delay(1000);
    }

    // Change location
    else if (output.indexOf(F("City")) == 0) {
      getCity(output);
      preferences.putString("city", city);
      preferences.putString("countryCode", countryCode);
      weather_timer = 0;
    }

    // Add alarm
    else if (output.indexOf(F("Alarm")) == 0) {
      addAlarm(output.substring(6));
    }

    // Remove alarm
    else if (output.indexOf(F("RemoveAlarm")) == 0) {
      removeAlarm(output.substring(12));
    }

    // Stop alarm
    else if (output.indexOf(F("StopAlarm")) == 0) {
      stopAlarm(output.substring(10));
    }

    // Set Theme
    else if (output.indexOf(F("Theme")) == 0) {
      setTheme(output.substring(6));
    } 
  }
}

/* time loop that displays the time on the word clock */
void time_loop()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  String day = dayNames[timeinfo.tm_wday];

  // update the time if it's changed
  if (last_hour != hour || last_minute != minute) {
    last_hour = hour;
    last_minute = minute;
    printLocalTime();
    light_time(hour, minute);  
  }
  alarm_loop(day, hour, minute);
}

/* loop that controls all the alarms,  */
void alarm_loop(String day, int hour, int minute)
{
  if (alarmCount > 0) {

    // Iterate over all the alarms.
    for (int i = 0; i < alarmCount; i++) {
      String alarmKey = "alarm" + String(i);
      String alarm = preferences.getString(alarmKey.c_str());
      int sub_index = alarm.indexOf("-");
      String alarmDay = alarm.substring(0, sub_index);

      // Not the right day
      if (day != alarmDay) {
        continue;
      }
      int index = alarm.indexOf(":");
      int alarmHour = alarm.substring(sub_index+2,index).toInt();

      //Not the right hour
      if (hour!= alarmHour) {
        continue;
      }
      int alarmMinute = alarm.substring(index+1).toInt();

      // Not the right minute
      if (minute!= alarmMinute) {
        continue;
      }

      // If we arrive here, it means that alarm i is the scheduled for now.
      // indexAlarm is the index of the current alarm (which is scheduled for now).
      indexAlarm = i;
      if (alarm == alarmToStop) {
        if (isAlarmOn) {
          clear_alarm();
        }
        return;
      }
      else {

        // Bliping
        if (isAlarmOn) {
          clear_alarm();
        }
        else {
          light_alarm();
        }
        return;
      }
    }
    indexAlarm = -1;
    alarmToStop = "";
    clear_alarm();
  }
}

/* weather loop that weather conditions the time on the word clock */
void weather_loop()
{
  
  if (city.isEmpty() || countryCode.isEmpty()) {
    return;
  }
  
  if ((millis() - weather_timer) > 60000) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";
      jsonBuffer = httpGETRequest(serverPath.c_str());
      JSONVar myObject = JSON.parse(jsonBuffer);
  
       if (JSON.typeof(myObject) == "undefined") {
        return;
      }
      int temp = myObject["main"]["temp"];
      Serial.print(F("Country: "));
      Serial.println(countryCode);
      Serial.print(F("City: "));
      Serial.println(city);
      Serial.print(F("Temperature: "));
      Serial.println(temp);
      int tz = myObject["timezone"];
      if (gmtOffset_sec != tz) {
        gmtOffset_sec = tz;
        preferences.putInt("gmtOffset", gmtOffset_sec);
        configTime(gmtOffset_sec, 0, "pool.ntp.org", "time.nist.gov");
      }
      int wc = myObject["weather"][0]["id"];
      Serial.print(F("wc = "));
      Serial.println(wc);
      if (currentTemperature != temp || weatherCondition != wc) {
        currentTemperature = temp;
        weatherCondition = wc;
        setWeatherLights(temp, wc);
      }
      weather_timer = millis();
  }
}

/* main loop of the word clock */
void loop() {
  bluetooth_loop(); // wait for some inputs from bluetooth
  if (WiFi.status() == WL_CONNECTED) // only if wifi exist display time and weather
  {
    time_loop();
    weather_loop();
    delay(1000);
  }
  else{ // not connected to wifi so keep trying to connect
    Serial.println(F("Not Connected to WIFI"));
    wifi_not_connected();
    if (!ssid.isEmpty() && !password.isEmpty()) {
      Serial.println(F("ssid is : "));
      Serial.println(ssid);
      Serial.println(F("password is : "));
      Serial.println(password);
      WiFi.begin(ssid, password);
    }
    delay(5000);
    if (WiFi.status() == WL_CONNECTED) {
      wifi_connected();
      Serial.println(F("Connected!"));
    }
  }
}

/*
    End of loop functions
*/
// ----------------------------------------------------------

