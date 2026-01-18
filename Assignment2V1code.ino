#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===== PIN DEFINITIONS =====
#define DHTPIN 21
#define DHTTYPE DHT11

#define IR_PIN 27
#define AIR_PIN 34
#define LED_PIN 22
#define FAN_PIN 26

// ===== WIFI & MQTT =====

const char* ssid = "PUT_YOUR_OWN_SSID";
const char* password = "PUT_YOUR_OWN_PASSWORD";
const char* mqtt_server = "PUT_YOUR_OWN_MQTT_SERVER"; // 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// ===== FUNCTIONS =====
void setup_wifi() {
  Serial.println("[WiFi] Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("[WiFi] Connected successfully");
  Serial.print("[WiFi] IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] Attempting connection...");

    // Create unique client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println(" connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}


// ===== ARDUINO REQUIRED FUNCTIONS =====
void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("=== ESP32 Smart Home Starting ===");

  pinMode(IR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  dht.begin();
  Serial.println("[DHT] Sensor initialized");

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  Serial.println("[MQTT] Broker configured");
}

void loop() {


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ===== SENSOR READINGS =====
int presence = digitalRead(IR_PIN);
float temperature = dht.readTemperature();
int airQuality = analogRead(AIR_PIN);
bool fanOn = false;

if (isnan(temperature)) {
  Serial.println("[ERROR] DHT read failed");
  delay(2000);
  return;
}

// ===== ACTUATOR LOGIC =====
digitalWrite(LED_PIN, presence ? LOW : HIGH);

if (presence && temperature > 23) {
  digitalWrite(FAN_PIN, LOW);   // ACTIVE-LOW RELAY
  fanOn = true;
} else {
  digitalWrite(FAN_PIN, HIGH);
  fanOn = false;
}

// ===== DEBUG OUTPUT =====
Serial.println("----- Sensor Readings -----");
Serial.print("Presence: ");
Serial.println(presence ? "DETECTED" : "NOT DETECTED");

Serial.print("Temperature: ");
Serial.print(temperature);
Serial.println(" °C");

Serial.print("Fan Status: ");
Serial.println(fanOn ? "ON" : "OFF");



  // ===== ACTUATOR LOGIC =====
  digitalWrite(LED_PIN, presence ? HIGH : LOW);

if (presence && temperature > 25) {
  digitalWrite(FAN_PIN, HIGH);
  fanOn = true;
} else {
  digitalWrite(FAN_PIN, LOW);
  fanOn = false;
}

  // ===== JSON PAYLOAD =====
  String payload = "{";
payload += "\"presence\":" + String(presence) + ",";
payload += "\"air_quality\":" + String(airQuality) + ",";
payload += "\"temperature\":" + String(temperature)+ ",";
payload += "\"fan_status\":\"" + String(fanOn ? "ON" : "OFF") + "\"";
payload += "}";


  Serial.print("[MQTT] Publishing data: ");
  Serial.println(payload);

  boolean status = client.publish("smarthome/data", payload.c_str());

  if (status) {
    Serial.println("[MQTT] Publish SUCCESS");
  } else {
    Serial.println("[MQTT] Publish FAILED");
  }

  Serial.println("---------------------------\n");
  delay(1000);
}
