#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <espnow.h>

// ================= THÔNG SỐ MẠNG & MQTT =================
const char* ssid = "CyberCenter";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com"; 

// ================= CẤU HÌNH CHÂN (PINOUT) =================
#define PIR_PIN    D3  
#define DHTPIN     D5  
#define RELAY_PIN  D7  

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

int lastPirState = LOW; 
unsigned long lastDHTRead = 0;
unsigned long lastReconnectAttempt = 0;

// --- HÀM HỨNG SÓNG TỪ ESP32 ---
void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  char msg[len + 1];
  memcpy(msg, incomingData, len);
  msg[len] = '\0';
  
  Serial.println(">> NHAN TU ESP32: " + String(msg));
  
  // Đẩy lên Java qua đúng topic mà Java đang subscribe
  if (client.connected()) {
    client.publish("lab/node1/auth", msg); 
    Serial.println(">> Da day data len MQTT Topic: lab/node1/auth");
  } else {
    Serial.println(">> Loi: Chua co ket noi MQTT de day data!");
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConnecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK!");
  
  // BƯỚC QUAN TRỌNG: In kênh WiFi để cấu hình cho ESP32
  Serial.print("CRITICAL: WiFi Channel is: ");
  Serial.println(WiFi.channel()); 
}

void publishStatus(int motion, float t = -1, float h = -1) {
  if (!client.connected()) return;
  String payload = "{\"motion\":" + String(motion);
  if (t != -1) {
    payload += ", \"temp\":" + String(t) + ", \"hum\":" + String(h);
  }
  payload += "}";
  client.publish("lab/node2/status", payload.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) message += (char)payload[i];
  if (message == "FAN_ON") digitalWrite(RELAY_PIN, HIGH);
  else if (message == "FAN_OFF") digitalWrite(RELAY_PIN, LOW);
}

boolean reconnect() {
  String clientId = "Node2-Wemos-";
  clientId += String(random(0xffff), HEX);
  if (client.connect(clientId.c_str())) {
    client.subscribe("lab/node2/control");
    Serial.println("MQTT Connected!");
  }
  return client.connected();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Đợi 1 giây cho điện áp ổn định
  Serial.println("--- DANG CHAY ROI NE DUY OI ---");
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Duy - Group 1");

  dht.begin();
  setup_wifi();

  // Khởi tạo ESP-NOW (phải sau khi có WiFi)
  if (esp_now_init() == 0) {
    Serial.println("ESP-NOW Ready");
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataRecv);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      unsigned long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        if (reconnect()) lastReconnectAttempt = 0;
      }
    } else {
      client.loop();
    }
  }

  int currentPirState = digitalRead(PIR_PIN);
  if (currentPirState != lastPirState) {
    if (currentPirState == HIGH) {
      lcd.setCursor(0, 1); lcd.print("Xin chao!      ");
      publishStatus(1);
    } else {
      lcd.setCursor(0, 1); lcd.print("Phong trong    ");
      publishStatus(0);
    }
    lastPirState = currentPirState;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTRead > 5000) {
    lastDHTRead = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) publishStatus(currentPirState, t, h);
  }
}
