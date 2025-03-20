#include <ArduinoJson.h>

const int led1 = 7;
const int led2 = 8;
const int baudRate = 9600;

void setup() {
    Serial.begin(baudRate);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
}

void loop() {
    // Send JSON command to start a quantum job with 2 qubits
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["action"] = "start_job";
    jsonDoc["num_qubits"] = 2;
    
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println(jsonString);
    
    // Wait for a response from the Python script
    unsigned long startTime = millis();
    while (!Serial.available()) {
        if (millis() - startTime > 5000) {  // Timeout after 5 seconds
            return;
        }
    }
    
    // Read response
    String response = Serial.readStringUntil('\n');
    StaticJsonDocument<200> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);
    if (error) {
        return; // Skip if JSON parsing fails
    }
    
    // Extract qubit results and set LEDs accordingly
    JsonArray resultArray = responseDoc["job_result"].as<JsonArray>();
    if (resultArray.size() >= 2) {
        digitalWrite(led1, resultArray[0] ? HIGH : LOW);
        digitalWrite(led2, resultArray[1] ? HIGH : LOW);
    }
    
    delay(10000);  // Wait 10 seconds before next request
}
