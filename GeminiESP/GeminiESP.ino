#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- Update these ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* apiKey = "YOUR_GEMINI_API_KEY";
const char* endpoint = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Ready! Input a task in the Serial Monitor:");
}

void loop() {
  if (Serial.available() > 0) {
    String task = Serial.readStringUntil('\n');
    task.trim();
    if (task.length() > 0) {
      Serial.println("Processing: " + task + "...");
      analyzeTask(task);
      Serial.println("\nInput a new task:");
    }
  }
}

void analyzeTask(String task) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(endpoint) + apiKey);
  http.addHeader("Content-Type", "application/json");

  // Sanitize user input for JSON
  task.replace("\"", "\\\"");

  String payload = "{"
    "\"contents\": [{\"parts\":[{\"text\": \"Analyze: " + task + "\"}]}], "
    "\"systemInstruction\": {\"parts\":[{\"text\": \"Return ONLY a JSON object with keys: name, description, value (1-100), effort (1-100).\"}]},"
    "\"generationConfig\": {\"response_mime_type\": \"application/json\"}"
  "}";

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    String response = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      const char* jsonStr = doc["candidates"][0]["content"]["parts"][0]["text"];
      JsonDocument taskData;
      deserializeJson(taskData, jsonStr);
      serializeJsonPretty(taskData, Serial);
      Serial.println();
    }
  } else {
    Serial.printf("Error: %d\n", httpCode);
  }
  http.end();
}