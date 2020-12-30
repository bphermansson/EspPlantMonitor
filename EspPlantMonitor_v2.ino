/*
 * A device that detects humidity in soil. The probe is the sensor from a moist sensor  
 * that is connected to an ESP12. 
 * Connections:
 * Vcc -> 2N3904 Collector. Probe leg 1 -> Base 2N3904. 
 * 2N3904 emittor to ESP Gpio04 and via 10k resistor to ground.
 * 100ohm resistor from Vcc to Probe leg 2. 
 * 
 * 33k from battery in to ADC. 10k from ADC to Gnd.
 */

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define DEBUG 1
#define SEN_IN 5
#define powerPin 4
#define LED_BUILTIN 2

String strDevice = "EspPlantMonitorV2";

String topicPreFix = "EspPlantMonitor/";
String topicPostFixV = "/values";
String topicPostFixD = "/debug";
String topicPostFixE = "/error";
String topicPostFixS = "/status";

// Settings for the Mqtt broker:
#define MQTT_USERNAME "emonpi"     
#define MQTT_PASSWORD "emonpimqtt2016"  
const char* mqtt_server = "192.168.1.190";

// Wifi settings
const char* ssid = "NETGEAR83";
const char* password = "";

int sensorValue = 0;
int adc;
long starttime;
 
// Mqtt & Wifi
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  starttime = millis();
  // Build Mqtt topics
    String myString = topicPreFix + strDevice + topicPostFixV;
    const char * mqtt_value_topic = myString.c_str();
    myString = topicPreFix + strDevice + topicPostFixD;
    const char * mqtt_debug_topic = myString.c_str();
    myString = topicPreFix + strDevice + topicPostFixE;
    const char * mqtt_error_topic = myString.c_str();
    myString = topicPreFix + strDevice + topicPostFixS;
    const char * mqtt_status_topic = myString.c_str();

  // Start serial port
    #ifdef DEBUG
      Serial.begin(115200);
      //Serial.setTimeout(2000);
      // Wait for serial to initialize.
      while(!Serial) { }
      Serial.println("Booting");  
    #endif
 
  pinMode(powerPin, OUTPUT);  

  #ifdef DEBUG
    Serial.println("Read sensor");
  #endif

// Read sensor
  digitalWrite(powerPin, HIGH); 
  sensorValue = digitalRead(SEN_IN);
  digitalWrite(powerPin, LOW);
// Read Vcc
  adc = analogRead(0); 
  float adc_volt = (0.000945 * adc * 4.3);
  #ifdef DEBUG
    Serial.print("adc raw: ");
    Serial.println(adc);
    Serial.print("vcc: ");
    Serial.println(adc_volt);
    Serial.print("Moist: ");
    Serial.println(sensorValue);
  #endif

// Create message
  StaticJsonBuffer<150> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  char msg[150];
  root["moist"] = String(sensorValue);
  root["device"] = strDevice;
  root["adc"] = String(adc);
  root["vcc"] = String(adc_volt);
  root.printTo((char*)msg, root.measureLength() + 1);

// Static IP
  WiFi.begin(ssid, password);
  IPAddress ip(192,168,1,249);   
  IPAddress gateway(192,168,1,1);   
  IPAddress subnet(255,255,255,0);   
  WiFi.config(ip, gateway, subnet);
  WiFi.hostname(strDevice);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  #ifdef DEBUG
    Serial.println("Connected to Wifi");
    Serial.println(WiFi.localIP());
  #endif

  // Setup Mqtt connection
  client.setServer(mqtt_server, 1883);
  if (!client.connected()) {
      reconnect();
  }
  client.publish(mqtt_value_topic, msg);  // Send with retain=true
  client.loop();

  #ifdef DEBUG
      Serial.println("Published to Mqtt broker");
      Serial.println(msg);
  #endif

  // We dont use over the air programming for now
  // But test https://www.bakke.online/index.php/2017/06/02/self-updating-ota-firmware-for-esp8266/ some time...
  // ota();
  
  #ifdef DEBUG
    Serial.println("Done");
    Serial.print("Time: ");
    int runtime = millis()-starttime;
    Serial.println(runtime);
  #endif

  // Deep Sleep
  #ifdef DEBUG
    Serial.println("Going to sleep");
  #endif
  
  

  //const int sleepTimeS = 5000;  // 83.m
  const int sleepTimeS = 1000; // = 16m
  //const int sleepTimeS = 10;  // = 11s
  //const int sleepTimeS = 100;  // = 
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
  // Everything is done in setup as the Esp sleeps and runs setup code before going to sleep again.
}
/*
void ota() {
  ArduinoOTA.setHostname("EspPlantMonitor");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
 
}
*/
