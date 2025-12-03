#include <Wifi.h>
#include <Firebase_ESP_Client.h>

const char* ssid = "namawifianda";
const char* password = "passwordwifianda"

#define API_KEY "AIzaSyBrBcNvI03cwwkSBktBvBMVYuFd3SJNNy4"
#define DATABASE_URL "https://tugas1-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "usernameyangsudah didaftarkan"
#define USER_PASSWORD "your-auth-password"

#define dht 23
#define ldr 19
#define soil 18

void setup() {
  void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== SMART PLANT GREENHOUSE ===");
  Serial.println("Inisialisasi sistem...\n");

  pinMode(LDR_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(OBJECT_PIN, INPUT);

  connectWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu dengan NTP...");
  delay(2000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Serial.println("Menghubungkan ke Firebase...");
  Firebase.reconnectWiFi(true);

  unsigned long fbstart = millis();
  while (!Firebase.ready() && millis() - fbstart < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println("\n✔ Firebase terhubung!");
    Serial.println("✓ Sistem siap monitoring!\n");
  } else {
    Serial.println("\n✘ Firebase gagal terhubung, sistem tetap berjalan...\n");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus! Mencoba reconnect...");
    connectWiFi();
  }

  unsigned long now = millis();
  if (now - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = now;
    bacaDanKirimData();
  }
}

void connectWiFi() {
  WiFi.begin(WiFi_SSID, WiFi_PASSWORD);
  Serial.println("Menghubungkan ke WiFi");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    if (millis() - start > 20000) {
      Serial.println("\n✘ Gagal terhubung WiFi - restart...");
      ESP.restart();
    }
  }

  Serial.println("\n✔ WiFi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

unsigned long getTimestamp() {
  time_t now;
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠ Gagal mendapat waktu NTP, gunakan millis()");
    return millis();
  }

  time(&now);
  return (unsigned long)now * 1000; 
}

void bacaDanKirimData() {
  Serial.println("\n==============================");
  Serial.println("|   PEMBACAAN SENSOR GREENHOUSE   |");
  Serial.println("==============================");

  int rawLdr = analogRead(LDR_PIN);
  int lightLevel = map(rawLdr, 4095, 0, 0, 100);
  lightLevel = constrain(lightLevel, 0, 100);

  Serial.printf("🌞 Cahaya: %d %% (ADC=%d)\n", lightLevel, rawLdr);

  int rawSoil = analogRead(SOIL_PIN);
  int soilPercent = map(rawSoil, 4095, 0, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  Serial.printf("💧 Kelembaban Tanah: %d %% (ADC=%d)\n", soilPercent, rawSoil);

  if (soilPercent < 40) {
    Serial.println("⚠ STATUS: KERING - Perlu penyiraman!");
  } else {
    Serial.println("✓ STATUS: Kelembaban cukup");
  }
}

motionDetected = digitalRead(PIR_PIN) == HIGH;
flameDetected  = digitalRead(FLAME_PIN) == HIGH;
objectDetected = digitalRead(OBJECT_PIN) == HIGH;

Serial.printf("🕺 Gerakan (PIR): %s\n", motionDetected ? "TERDETEKSI ⚠️" : "Tidak ada");
Serial.printf("🔥 Api: %s\n", flameDetected ? "TERDETEKSI ⚠️" : "Aman");
Serial.printf("📦 Objek: %s\n", objectDetected ? "TERDETEKSI" : "Tidak ada");

if (Firebase.ready()) {
    Serial.println("\n📡 Mengirim data ke Firebase...");

    String basePath = "/greenhouse/sensors";
    bool allSuccess = true;

    if (Firebase.RTDB.setInt(&fbdo, basePath + "/lightLevel", lightLevel)) {
        Serial.println("   ✓ lightLevel terkirim");
    } else {
        Serial.printf("   ✗ lightLevel gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    if (Firebase.RTDB.setInt(&fbdo, basePath + "/soilMoisture", soilPercent)) {
        Serial.println("   ✓ soilMoisture terkirim");
    } else {
        Serial.printf("   ✗ soilMoisture gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    if (Firebase.RTDB.setBool(&fbdo, basePath + "/motion", motionDetected)) {
        Serial.println("   ✓ motion terkirim");
    } else {
        Serial.printf("   ✗ motion gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    if (Firebase.RTDB.setBool(&fbdo, basePath + "/flame", flameDetected)) {
        Serial.println("   ✓ flame terkirim");
    } else {
        Serial.printf("   ✗ flame gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    if (Firebase.RTDB.setBool(&fbdo, basePath + "/object", objectDetected)) {
        Serial.println("   ✓ object terkirim");
    } else {
        Serial.printf("   ✗ object gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    unsigned long timestamp = getTimestamp();
    if (Firebase.RTDB.setDouble(&fbdo, basePath + "/timestamp", timestamp)) {
        Serial.printf("   ✓ timestamp terkirim (%lu)\n", timestamp);
    } else {
        Serial.printf("   ✗ timestamp gagal: %s\n", fbdo.errorReason().c_str());
        allSuccess = false;
    }

    if (allSuccess) {
        Serial.println("\n✅ Semua data berhasil dikirim!");
    } else {
        Serial.println("\n⚠️ Beberapa data gagal dikirim");
    }

} else {
    Serial.println("⛔ Firebase belum siap, skip pengiriman");
}

Serial.println("──────────────────────────────────────────");

delay(100);
}