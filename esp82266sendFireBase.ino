#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Firebase configuration
FirebaseConfig config;
FirebaseAuth auth;

// WiFi credentials
#define WIFI_SSID "Electric Fertilizer"
#define WIFI_PASSWORD "12345678a"

FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");

  // Firebase configuration
  config.host = "el-fert-default-rtdb.firebaseio.com";
  config.signer.tokens.legacy_token = "f1pYivLq2lbxJtACOIqPqMrirfYbnHki8hs8x364";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Check if there is any incoming data on Serial
  if (Serial.available()) {
    String incomingMessage = "";
    while (Serial.available()) {
      incomingMessage += char(Serial.read());
      delay(50); // Wait a bit to ensure all data has been received
    }

    // Parse the received message
    int moistureLevel;
    float voltage;
    float current;
    int height;
    int dutyCycle; // Variable to store PWM value
    sscanf(incomingMessage.c_str(), "%d;%f;%f;%d;%d", &moistureLevel, &voltage, &current, &height, &dutyCycle);

    // Print the received values
    Serial.print("Soil Moisture: ");
    Serial.println(moistureLevel);
    Serial.print("DC Voltage: ");
    Serial.println(voltage);
    Serial.print("Current: ");
    Serial.println(current);
    Serial.print("Height: ");
    Serial.println(height);
    Serial.print("PWM: ");
    Serial.println(dutyCycle);

    // Send data to Firebase
    if (Firebase.ready()) {
      if (Firebase.setInt(firebaseData, "/sensorData/moistureLevel", moistureLevel)) {
        Serial.println("Moisture Level updated successfully");
      } else {
        Serial.println("Failed to update Moisture Level");
        Serial.println("REASON: " + firebaseData.errorReason());
      }

      if (Firebase.setFloat(firebaseData, "/sensorData/voltage", voltage)) {
        Serial.println("Voltage updated successfully");
      } else {
        Serial.println("Failed to update Voltage");
        Serial.println("REASON: " + firebaseData.errorReason());
      }

      if (Firebase.setFloat(firebaseData, "/sensorData/current", current)) {
        Serial.println("Current updated successfully");
      } else {
        Serial.println("Failed to update Current");
        Serial.println("REASON: " + firebaseData.errorReason());
      }

      if (Firebase.setInt(firebaseData, "/sensorData/height", height)) {
        Serial.println("Height updated successfully");
      } else {
        Serial.println("Failed to update Height");
        Serial.println("REASON: " + firebaseData.errorReason());
      }

      // Send PWM value to Firebase at /sensorData/dutyCycle
      if (Firebase.setInt(firebaseData, "/sensorData/dutyCycle", dutyCycle)) {
        Serial.println("Duty Cycle updated successfully at /sensorData/dutyCycle");
      } else {
        Serial.println("Failed to update Duty Cycle at /sensorData/dutyCycle");
        Serial.println("REASON: " + firebaseData.errorReason());
      }

      // Also send PWM value to Firebase at /controlData/pwm
      if (Firebase.setInt(firebaseData, "/controlData/pwm", dutyCycle)) {
        Serial.println("Duty Cycle updated successfully at /controlData/pwm");
      } else {
        Serial.println("Failed to update Duty Cycle at /controlData/pwm");
        Serial.println("REASON: " + firebaseData.errorReason());
      }
    } else {
      Serial.println("Firebase not ready");
    }
  }

  // Check for updates from Firebase to control PWM
  if (Firebase.ready()) {
    if (Firebase.getInt(firebaseData, "/controlData/pwm")) {
      int pwmValue = firebaseData.intData();
      String message = "PWM:" + String(pwmValue);
      Serial.println(message); // Send the PWM value to Arduino
    } else {
      Serial.println("Failed to get PWM value from Firebase");
      Serial.println("REASON: " + firebaseData.errorReason());
    }
  }

  delay(1000); // Adjust the delay as needed
}
