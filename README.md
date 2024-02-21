# Arduino Temperature SensorScript
**Preparations**

To start you will need either an ESP32S or an ESP8266 as these scripts are made with them in mind.<br/>
The temperature sensor used for these scripts is the DHT22

**Setup**

To upload the script to the ESP you will first need to follow these steps:

 1. Download Arduino IDE from Arduino's website https://www.arduino.cc/en/software (I used 1.8.19 so if you're having problems try that one, be sure to checkmark Install USB driver and Associate .ino files in setup installation), then plug in your board
 2. Download the script fitting your board from this Git, open it in the IDE, and then replace the apiService text that says "insert api service name" with the apiService given by your instructor (Even if the URL is https, I'd recommend using http instead)
 3. In the upper navigation bar click on `File`, then click on `Preferences`
 4. In Preferences there should be a field called `Additional Boards Manager URLs:`,
    copy and paste below links to that field, and then click `OK`
    > https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json
 5. Once more in the navigation bar, click on `Tools`, then `Board:` and then `Boards Manager...`
 6. In Boards Manager search either __esp32__ or __esp8266__ depending on your board, and then click Install
 7. Back in `Board:` select the appropriate board (for ESP32 I use Node32s, and for ESP8266 Boards I use LOLIN(WeMos) D1 R1)
 6. Still in the `Tools` click on `Port:` and select the port your Arduino is in
 7. Now in the navigation bar click on `Sketch`, then `Include Library`, and then `Manage Libraries...`
 8. In the upper search in Library Manager search __DHT sensor library__ and choose the library "DHT sensor library" by Adafruit, install all other libraries needed for the download to complete
 9. Finally upload the sketch by clicking the arrow pointing right (for the ESP32S you will need to hold down the Boot button and let go when it says Connecting... in the console)
 10. You should now be able to see it connecting to the internet through the serial monitor, or by observing the boards built in led

**Connecting Sensor**

These scripts use pin 21 for ESP32 and pin D2 for ESP8266, feel free to change them if needed.<br/>
The DHT22 needs to be connected to either 3V3 or 5V, ground (G), and the serial pin stated above.

**Notes**

Once it connects to the wifi, the board should send a POST request upon startup.<br/>
When connecting to wifi the built in LED will blink, when succesfully sending POST request it will flicker, in the case it gets an http error code it will blink with a periodic tempo, and when it can't read the sensor it will be lit up
