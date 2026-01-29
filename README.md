# Gemini API on ESP32 (Minimalist Guide)

This guide shows you how to send a prompt to Google's Gemini API and receive a structured JSON response using an ESP32.

## 1. Prerequisites

**Hardware:** ESP32 Dev Board.

**Libraries:**
- ArduinoJson (Install via Library Manager).
- WiFi and HTTPClient (Built-in to ESP32 board package).

**API Key:** Get one for free at [Google AI Studio](https://aistudio.google.com/).

## 2. Code Breakdown

### Step 1: Configuration & Headers

We start by importing the necessary libraries for WiFi connection, making HTTP POST requests, and parsing the JSON data returned by the AI. We also define the endpoint for the Gemini 2.5 Flash model.

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* apiKey = "YOUR_GEMINI_API_KEY";
const char* endpoint = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=";
```

### Step 2: WiFi Setup & Serial Input

The `setup()` function initializes the hardware and connects to the internet. In the `loop()`, we listen for user input from the Serial Monitor to trigger the AI analysis.

```cpp
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Ready! Input a task:");
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
```

### Step 3: Request & Structured Output

The `analyzeTask()` function builds a JSON payload. We use `systemInstruction` to define the schema and `generationConfig` to force the model to respond with a valid JSON object.

```cpp
void analyzeTask(String task) {
  HTTPClient http;
  http.begin(String(endpoint) + apiKey);
  http.addHeader("Content-Type", "application/json");

  // Define the JSON structure requirements in the payload
  String payload = "{"
    "\"contents\": [{\"parts\":[{\"text\": \"Analyze this: " + task + "\"}]}], "
    "\"systemInstruction\": {\"parts\":[{\"text\": \"Return ONLY a JSON object with keys: name, description, value (1-100), effort (1-100).\"}]},"
    "\"generationConfig\": {\"response_mime_type\": \"application/json\"}"
  "}";

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    
    // Gemini returns the structured JSON as a string within its own response
    const char* jsonStr = doc["candidates"][0]["content"]["parts"][0]["text"];
    JsonDocument taskData;
    deserializeJson(taskData, jsonStr);
    
    serializeJsonPretty(taskData, Serial);
    Serial.println();
  }
  http.end();
}
```

## 3. Tips

- **Sanitization:** The `task.replace("\"", "\\\"");` line is crucial to prevent user input from breaking your JSON payload.
- **Timeouts:** For a better experience, call `Serial.setTimeout(50);` in `setup()` to reduce the delay when reading strings from the monitor.
- **Memory:** If the AI response is very long, use `JsonDocument doc` without a size limit (it will allocate on the heap) but monitor `ESP.getFreeHeap()`.
