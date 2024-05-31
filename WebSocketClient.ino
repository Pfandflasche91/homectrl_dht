#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <DHT.h>
#include <Arduino_JSON.h>

const char* ssid = "maxworld";
const char* password = "01232605";
const char* webSocketServerAddress = "192.168.2.231"; // Replace with the IP address of the WebSocket server

#define BOARD_ID 2
// Digital pin connected to the DHT sensor
#define DHTPIN 2 

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message {
    float temp;
    float hum;
    int readingId;
    char* sensorlocation;
    char* sensornr;
} struct_message;

//Create a struct_message called myData
struct_message myData;
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

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

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected!");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
    case WStype_TEXT:
      Serial.printf("Received text: %s\n", payload);
      break;
  }
}

// Function to convert struct to JSON stringdel
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
void setup() {
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.begin(115200);
  dht.begin();
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW); 
    delay(2500);
    Serial.print(".");
  }
  delay(2500);
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.println("");

  Serial.println("WiFi connected");

  // Connect to WebSocket server
  webSocket.begin(webSocketServerAddress,8657);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    //Set values to send
    myData.temp = readDHTTemperature();
    myData.hum = readDHTHumidity();
    myData.readingId = readingId++;
    myData.sensorlocation = "Keller";
    myData.sensornr = "1";
    
    //Send 
    webSocket.sendTXT(structToJson(myData).c_str());
    Serial.print(structToJson(myData));
  }
}