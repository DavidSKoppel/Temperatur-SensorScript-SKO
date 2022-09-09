/*
This Temperature Sensor script was designed to be used with a 
ESP8266-D1-mini for IT&Data Odense SKO
*/

#include "DHT.h";
#include <ESP8266WiFi.h>
#include <time.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN 5 //(D1) what pin we're connected to, modify for different boards
#define DHTTYPE DHT22 // DHT 22 Sensor
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

//Variables
float humi;  // Stores humidity value
float temp = -273; // Stores temperature value, is sat at this beginning temp so it fires of immediately upon startup
String hostName = "mu8-37";
String ipAddress;
const char* apiService = "http://infotavle.itd-skp.sde.dk/TH_API/ClimateSensor_Api/api/climateSensor/create.php";
struct tm timeinfo; // Our constructor for localtime

// Our wifi and api, add password if wifi is secured
const char* ssid = "Sde-Guest";
//const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Define NTP Client to get time
const char* ntpServer = "dk.pool.ntp.org";
const long  gmtOffset_sec = 3628;
const int   daylightOffset_sec = 3600; // Will be used if we are in summertime

// Splits hostName at the "-" so it can be sent in proper format in JSON
int splitPoint = hostName.indexOf('-');
String deviceName = hostName.substring(0, splitPoint);
String zone = hostName.substring(splitPoint + 1, hostName.length()); 

bool getLocalTime(struct tm * info)
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= 5000) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}

void InitWifi() {
  // Connects to wifi
  WiFi.begin(ssid);
  //WiFi.begin(ssid, PASSWORD_IF_NEEDED);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    digitalWrite(LED_BUILTIN, LOW); 
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(250);
  }
  // Enables autoreconnect to wifi, in case of wifi shutdown or weak connection
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  Serial.println(WiFi.localIP());
  ipAddress = IpAddress2String(WiFi.localIP());
}

// Needed for converting ipAddres to string for JSON
String IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void SetTimezone()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("There was an in getting time, this could be that the NTPServer is not available, or a weak connection to the NTPServer");
    Serial.println("System will now restart");
    delay(10000);
    ESP.restart();
  }
  byte dd = timeinfo.tm_mday;
  byte mm = timeinfo.tm_mon + 1;
  byte yy = timeinfo.tm_year + 1900;
  byte x1 = 31 - (yy + yy / 4 - 2) % 7; //last Sunday March
  byte x2 = 31 - (yy + yy / 4 + 2) % 7; // last Sunday October
  if((mm > 3 && mm < 10) || (mm == 3 && dd >= x1) || (mm == 10 && dd < x2)){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
      Serial.println("There was an error in getting time, this could be that the NTPServer is not available, or a weak connection to the NTPServer");
      Serial.println("System will now restart");
      delay(10000);
      ESP.restart();
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  InitWifi();
  
  // Sets time based on denmark
  configTime(gmtOffset_sec, 0, ntpServer);
  SetTimezone();
  
  dht.begin();
}

void SendPOSTData() {  
  //Converts time to string format so it can later be used in JSON
  getLocalTime(&timeinfo);
  char timeAsString[24];
  strftime(timeAsString, sizeof(timeAsString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  String stringified(timeAsString);
  
  humi = dht.readHumidity();// Read humidity
  temp = dht.readTemperature();// Read temperature
    
  // Check if wifi is still connected
  if(WiFi.status() == WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
  
    http.begin(client, apiService);

    // The JSON that will be send to our api service
    http.addHeader("Content-Type", "application/json");            
    String httpRequestData = "{\"ipaddress\": \"" + ipAddress;
    httpRequestData += "\", \"zone\": " + zone;
    httpRequestData += ", \"name\": \"" + deviceName; 
    httpRequestData += "\", \"updated\": \"" + String(timeAsString);
    httpRequestData += "\", \"temperature\": " + String(temp);
    httpRequestData += ", \"humidity\": " + String(humi);    
    httpRequestData += "}"; 

    // Sends the POST request
    Serial.print(httpRequestData);
    int httpResponseCode = http.POST(httpRequestData);

    // Show the response code so we can determined if it was received by the api
    // 400 = Error, 200 = Received
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("");

    http.end();
    client.stop();
    digitalWrite(LED_BUILTIN, LOW); 
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(50);
    digitalWrite(LED_BUILTIN, LOW); 
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(50);
  }
  else {
    // Cancels and waits for wifi connection
    Serial.print("Unable to connect to wifi, please wait for autoreconnect or restart the system");
  }
  // Waits for 1 minute before scanning again
  delay(60000);
}

void loop() {
  // Used for comparison to old readings
  float currentTemp = dht.readTemperature();
  
  if ( isnan(currentTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(LED_BUILTIN, LOW); 
  } else {
    digitalWrite(LED_BUILTIN, HIGH); 
    // Gets current hour and minutes (E.g. 08:00) for scheduled checks
    getLocalTime(&timeinfo);
    char timeAsString[5];
    strftime(timeAsString, sizeof(timeAsString), "%H%M", &timeinfo);
    String plannedCheck(timeAsString);

    if(plannedCheck == "0158" || plannedCheck == "0159"){
      // Daily reboot
      delay(120000);
      ESP.restart();
    } else if(temp > currentTemp + 0.5 || temp < currentTemp - 0.5 
    || plannedCheck == "0800" || plannedCheck == "1200" || plannedCheck == "1500"){
      SendPOSTData();
    }
    Serial.print('.');
  }
  delay(1000);
}
