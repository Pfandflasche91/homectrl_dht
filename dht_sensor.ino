#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <Arduino_JSON.h>


//network config
const char* ssid = "maxworld";
const char* password = "01232605";
//API URL
const char* entpoint_temperatur = "http://192.168.2.231:3000/temperature";
const char* entpoint_humidity = "http://192.168.2.231:3000/humidity";
//Device ID
#define BOARD_ID 2
// Digital pin connected to the DHT sensor
#define DHTPIN 2 
// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

// struct for data
typedef struct struct_message {
    float temp;
    float hum;
    int readingId;
    char* sensorlocation;
    int sensornr;
} struct_message;
// Function to read Temperature
float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(t);
    return t;
  }
}
// Function to read Humidity
float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(h);
    return h;
  }
}
// Function to convert struct to JSON string
String structToJson(struct_message data) {
  JSONVar jsonDoc(200);
  jsonDoc["temp"] = data.temp;
  jsonDoc["hum"] = data.hum;
  jsonDoc["readingId"] = data.readingId;
  jsonDoc["sensornr"] = data.sensornr;
  jsonDoc["sensorlocation"] = data.sensorlocation;
  String jsonString = JSON.stringify(jsonDoc);
  return jsonString;
}


//Create a struct_message called myData and other start values
  struct_message myData;
  unsigned long previousMillis = 0;   // Stores last time temperature was published
  const long interval = 10000;        // Interval at which to publish sensor readings
  unsigned int readingId = 0;
  
  WiFiClient wifiClient;
  HTTPClient http;
 
  
void setup() {
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.begin(115200);
  dht.begin();
  delay(2000);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  delay(500);
  Serial.println("");
  Serial.print("Connecting to WIFI..");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); 
    delay(2500);
    Serial.print(".");
  }
  delay(2500);
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.println("\nWiFi connected");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    //Set values to send
    myData.temp = readDHTTemperature();
    myData.hum = readDHTHumidity();
    myData.readingId = readingId++;
    myData.sensorlocation = "Keller"; //todo create define
    myData.sensornr = BOARD_ID;  
    //Send 
    if (WiFi.status() == WL_CONNECTED) {
      http.begin(wifiClient,entpoint_temperatur);
        http.addHeader("Accept", "*/*"); // Header setzen
      http.addHeader("Content-Type", "application/json"); // Header setzen

      // JSON-Daten erstellen
      JSONVar jsonDoc;
      jsonDoc["value"] = readDHTTemperature(); // Den Wert setzen
      String jsonPayload = JSON.stringify(jsonDoc);

      int httpResponseCode = http.POST(jsonPayload);  
      if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("Temperature");
        Serial.println(httpResponseCode);
        Serial.println(payload);
        } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
      }

      delay(2500);

      http.begin(wifiClient,entpoint_humidity);
        http.addHeader("Accept", "*/*"); // Header setzen
      http.addHeader("Content-Type", "application/json"); // Header setzen

      // JSON-Daten erstellen
      JSONVar jsonDoc_hum;
      jsonDoc_hum["value"] = readDHTHumidity(); // Den Wert setzen
      String jsonPayload_hum = JSON.stringify(jsonDoc_hum);

      httpResponseCode = http.POST(jsonPayload_hum);  
      if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("Humidity");
        Serial.println(httpResponseCode);
        Serial.println(payload);
        } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
      }
    }
    //Serial.print(structToJson(myData));
    //sendTemperature(http);
  }
}