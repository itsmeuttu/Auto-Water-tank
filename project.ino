#include <WiFi.h>
#include <HTTPClient.h>

// Pin Definitions
const int TRIG_PIN = 5;  // Ultrasonic Sensor Trigger Pin
const int ECHO_PIN = 18; // Ultrasonic Sensor Echo Pin
const int RELAY_PIN = 15; // Relay module to control motor

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Discord Webhook URL
const String discordWebhookUrl = "YOUR_DISCORD_WEBHOOK_URL";

// Tank Dimensions
const float tankHeight = 100.0; // in cm (Set to your tank height)

void setup() {
  Serial.begin(115200);
  
  // Initialize Ultrasonic sensor and relay pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

void loop() {
  float waterLevel = getWaterLevel();
  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.println(" %");

  // Motor control logic
  if (waterLevel < 50) {
    digitalWrite(RELAY_PIN, LOW); // Start motor
  } else if (waterLevel >= 100) {
    digitalWrite(RELAY_PIN, HIGH); // Stop motor
  }

  // Send notification when below 25% or full
  if (waterLevel < 25) {
    sendDiscordNotification("Water level is below 25%! Consider refilling the tank.");
  } else if (waterLevel >= 100) {
    sendDiscordNotification("The water tank is full.");
  }

  delay(30000); // Wait for 30 seconds before re-checking
}

float getWaterLevel() {
  // Send a 10us pulse to TRIG_PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the duration of the ECHO_PIN pulse in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in cm
  float distance = duration * 0.034 / 2;

  // Convert the distance to water level percentage
  float waterHeight = tankHeight - distance;
  float waterLevel = (waterHeight / tankHeight) * 100;

  return waterLevel;
}

void sendDiscordNotification(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(discordWebhookUrl);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    String payload = "{\"content\":\"" + message + "\"}";

    // Send the request
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Discord Webhook Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending message: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
