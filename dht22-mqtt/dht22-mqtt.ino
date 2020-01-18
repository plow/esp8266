#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "settings-sensor.h"
#include "settings-wifi.h"
#include "settings-mqtt.h"

// DHT Sensor
uint8_t DHTPin = D1;
unsigned long interval = 60*1000*5;
               
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE  (5)
char msg_buf[MSG_BUFFER_SIZE];

struct measurement {
  float temperature;
  float humidity;
  bool error;
};

void setup_wifi() {

  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_dht() {
  pinMode(DHTPin, INPUT);
  dht.begin();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    Serial.print("Attempting MQTT connection with client ID ");
    Serial.println(clientId);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected.");
      // Once connected, publish an announcement...
      //client.publish(topic_temp, "reconnected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(". Waiting for one interval to pass and then trying again.");
      // Wait an interval before retrying
      delay(interval);
    }
  }
}

struct measurement measure() {

  // Switch on the LED if an 1 was received as first character
  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because it is active low on the ESP-01)

  struct measurement m;
  m.temperature = dht.readTemperature(); // Gets the values of the temperature
  m.humidity = dht.readHumidity(); // Gets the values of the humidity 

  // Check if any reads failed and exit early (to try again).
  if (isnan(m.temperature) || isnan(m.humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    m.error = true;
    return m;
  }
  m.error = false;

  String message = "Temperature: ";
  message += m.temperature;
  message += "°C \nHumidity: ";
  message += m.humidity;
  message += "%";

  Serial.print("Humidity: ");
  Serial.print(m.humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(m.temperature);
  Serial.println(" °C ");

  digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH

  return m;
}

void mqttPublish(const char* topic, float msg) {
  snprintf (msg_buf, MSG_BUFFER_SIZE, "%.2f", msg);
  Serial.print("Publish message to "); Serial.print(topic); Serial.print(": "); Serial.println(msg_buf);
  boolean succeeded = client.publish(topic, msg_buf);
  if(!succeeded) {
    Serial.println("Publishing of MQTT message failed");
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH

  setup_wifi();
  setup_dht();

  Serial.print("Configuring MQTT client for ");
  Serial.print(mqtt_server); Serial.print(":"); Serial.println(mqtt_server_port);
  client.setServer(mqtt_server, mqtt_server_port);
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  struct measurement m = measure();
  if(!m.error) {
    mqttPublish(topic_temp, m.temperature);
    mqttPublish(topic_humi, m.humidity);
  }
  
  client.disconnect(); 
  espClient.flush();
  // wait until connection is closed completely
  while( client.state() != -1){
    delay(10);
  }
  
  WiFi.disconnect();
  //Serial.println("Disconnecting WiFi...");
  //while ( WiFi.status() != WL_DISCONNECTED) {
  //  delay(100);
  //}

  Serial.println("Going to sleep now...");
  ESP.deepSleep(interval * 1e3); // micros
  //delay(interval);
  //Serial.print(".");
}
