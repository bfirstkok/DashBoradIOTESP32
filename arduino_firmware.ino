#include <WiFi.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include <time.h>          // เพิ่ม

#define API_KEY      "AIzaSyDON7jYsScQaKeFagtP1iedfkE2ORGcL7A"
#define DATABASE_URL "https://iot-dashboard-ea658-default-rtdb.asia-southeast1.firebasedatabase.app"

#define RELAY1_PIN     18
#define RELAY2_PIN     19
#define DHTPIN         27
#define DHTTYPE        DHT22
#define LDR_PIN        34
#define LED_STATUS     2
#define RESET_WIFI_PIN 4

DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseData streamFbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

#define PATH_SENSORS  "/iot_system/sensors"
#define PATH_CONTROLS "/iot_system/controls"
#define PATH_SETTINGS "/iot_system/settings"

unsigned long previousMillis = 0;
const long interval = 5000;

float temp          = 0.0;
float hum           = 0.0;
int   light_percent = 0;
bool  relay1_state  = false;
bool  relay2_state  = false;
bool  auto_mode     = false;
float auto_fan_temp = 30.0;
// เพิ่มตัวแปรจับเวลา history
unsigned long lastHistoryMs = 0;
const long historyInterval  = 60000;  // push ทุก 1 นาที

// ---- ดึง epoch ms จาก NTP ----
uint64_t getEpochMs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
}

bool isTimeReady() {
  return time(nullptr) > 1700000000UL;
}

void pushHistory() {
  if (!Firebase.ready() || !signupOK) return;
  if (!isTimeReady()) return;

  unsigned long now = millis();
  if (now - lastHistoryMs < historyInterval) return;
  lastHistoryMs = now;

  FirebaseJson json;
  json.set("temperature",   temp);
  json.set("humidity",      hum);
  json.set("light_percent", light_percent);
  json.set("timestamp",     (double)getEpochMs());

  if (Firebase.RTDB.pushJSON(&fbdo, "/iot_system/history", &json)) {
    Serial.println("History pushed OK");
  } else {
    Serial.printf("History push failed: %s\n", fbdo.errorReason().c_str());
  }
}

void streamCallback(FirebaseStream data) {
  String path = data.dataPath();
  String type = data.dataType();

  if (type == "boolean") {
    if (path == "/relay_1") {
      relay1_state = data.boolData();
      digitalWrite(RELAY1_PIN, relay1_state ? LOW : HIGH);
      Serial.printf("[STREAM] relay_1 = %s\n", relay1_state ? "ON" : "OFF");
    } else if (path == "/relay_2") {
      relay2_state = data.boolData();
      if (!auto_mode) digitalWrite(RELAY2_PIN, relay2_state ? LOW : HIGH);
      Serial.printf("[STREAM] relay_2 = %s\n", relay2_state ? "ON" : "OFF");
    }
  } else if (type == "json") {
    FirebaseJson* json = data.to<FirebaseJson*>();
    if (!json) return;
    FirebaseJsonData jd;
    if (json->get(jd, "relay_1") && jd.success) {
      relay1_state = jd.boolValue;
      digitalWrite(RELAY1_PIN, relay1_state ? LOW : HIGH);
    }
    if (json->get(jd, "relay_2") && jd.success) {
      relay2_state = jd.boolValue;
      if (!auto_mode) digitalWrite(RELAY2_PIN, relay2_state ? LOW : HIGH);
    }
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) Serial.println("[STREAM] timeout");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN,     OUTPUT);
  pinMode(RELAY2_PIN,     OUTPUT);
  pinMode(LED_STATUS,     OUTPUT);
  pinMode(RESET_WIFI_PIN, INPUT_PULLUP);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);

  dht.begin();

  WiFiManager wm;
  if (digitalRead(RESET_WIFI_PIN) == LOW) {
    Serial.println("Resetting WiFi...");
    wm.resetSettings();
  }
  if (!wm.autoConnect("(´。＿。｀)")) {
    delay(3000);
    ESP.restart();
  }

  Serial.println("WiFi Connected!");
  digitalWrite(LED_STATUS, HIGH);

  // ---- sync NTP ทันทีหลัง WiFi เชื่อมสำเร็จ ----
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Waiting for NTP");
  while (!isTimeReady()) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nNTP ready: %llu ms\n", getEpochMs());

  delay(2000);  // รอ DHT22

  config.api_key      = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase: login OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase login failed: %s\n",
                  config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.RTDB.beginStream(&streamFbdo, PATH_CONTROLS)) {
    Firebase.RTDB.setStreamCallback(&streamFbdo, streamCallback, streamTimeoutCallback);
    Serial.println("Stream controls ready");
  }
}

void loop() {
  Firebase.RTDB.readStream(&streamFbdo);

  unsigned long now = millis();
  if (now - previousMillis < interval) return;
  previousMillis = now;

  float newTemp = dht.readTemperature();
  float newHum  = dht.readHumidity();

  if (isnan(newTemp) || isnan(newHum)) {
    Serial.println("DHT read failed");
  } else {
    temp = newTemp;
    hum  = newHum;

    int ldr_raw = analogRead(LDR_PIN);
    light_percent = constrain(map(ldr_raw, 4095, 0, 0, 100), 0, 100);

    Serial.printf("T=%.1f H=%.1f L=%d ts=%llu\n",
                  temp, hum, light_percent, getEpochMs());

    uploadSensors();
    pushHistory();   // ← เพิ่มบรรทัดนี้
    pollSettings();
  }

  checkSmartAuto();
}

void uploadSensors() {
  if (!Firebase.ready() || !signupOK) return;
  if (!isTimeReady()) {
    Serial.println("NTP not ready, skip upload");
    return;
  }

  FirebaseJson json;
  json.set("temperature",   temp);
  json.set("humidity",      hum);
  json.set("light_percent", light_percent);
  json.set("timestamp",     (double)getEpochMs());  // ← ms จริงจาก NTP

  if (Firebase.RTDB.updateNode(&fbdo, PATH_SENSORS, &json)) {
    Serial.println("Upload OK");
  } else {
    Serial.printf("Upload failed: %s\n", fbdo.errorReason().c_str());
  }
}

void pollSettings() {
  if (!Firebase.ready() || !signupOK) return;

  if (Firebase.RTDB.getBool(&fbdo, "/iot_system/settings/auto_mode"))
    auto_mode = fbdo.boolData();

  if (Firebase.RTDB.getFloat(&fbdo, "/iot_system/settings/auto_fan_temp"))
    auto_fan_temp = fbdo.floatData();
}

void checkSmartAuto() {
  if (!auto_mode) return;

  if (temp >= auto_fan_temp && !relay2_state) {
    relay2_state = true;
    digitalWrite(RELAY2_PIN, LOW);
    if (Firebase.ready() && signupOK)
      Firebase.RTDB.setBool(&fbdo, "/iot_system/controls/relay_2", true);
    Serial.println("Auto: พัดลมเปิด");
  } else if (temp < (auto_fan_temp - 2.0) && relay2_state) {
    relay2_state = false;
    digitalWrite(RELAY2_PIN, HIGH);
    if (Firebase.ready() && signupOK)
      Firebase.RTDB.setBool(&fbdo, "/iot_system/controls/relay_2", false);
    Serial.println("Auto: พัดลมปิด");
  }
}
