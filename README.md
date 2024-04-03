# Word Clock
This project Made by Yan Tomsinsky, Gal Givati and Rafi Green.
We want to introduce you the word clock that we built. This clock is word and signs clock that displayes the time in words within a minute resolution. 
Also, the clock shows current weather conditions and have alarams functionallity.
The clock is controlled by bluetooth serial app that allows the user to change the location and set/delete alarms.

## More details in depth :
Time is displayed by words. The words describe the time in resolution of 5 minutes and 4 lights describe the minutes, one color is for ascending minutes and one for descending.
First, the user should download bluetooth serial app and connect to the device named "WordClock-YRG" throught bluetooth. Then, the user should enter the wifi name and password that he wants the clock to connect to.
Afterwards, following functions can be controled by user:

1. **Location settings**: user can provide city and country code of the location that he wants the time and weather to be displayed at.  <br>

2. **Network Settings**: user can provide the wifi credentials that he wants the clock to be connected. <br>

3. **Themes**: user can provide color theme for the clock words, the minutes ascending and the minutes descending. <br>

4. **Alarms**: <br>
4.1. **Add Alarm**- the user can add alarms and the clock will blink its alarm icon the desired time. <br>
4.2. **Stop Alarm**- the user can stop an alarm that ringed. <br>
4.3. **Remove Alarm**- the user can remove alarms. <br>

## Folder Description:
main- the source code for the esp side. <br>
test- test for all different clock and chip functionallity. <br>
input formats- the expected inputs format for the different functions requested by user. <br>

## Arduino/ESP32 libraries used in this project:
- Wifi <br>
- BluetoothSerial <br>
- Preferences <br>
- Adafruit_NeoPixel <br>
- Arduino_JSON <br>
- HTTPClient <br>

This project is part of ICST - The Interdisciplinary Center for Smart Technologies, Taub Faculty of Computer Science, Technion https://icst.cs.technion.ac.il/
