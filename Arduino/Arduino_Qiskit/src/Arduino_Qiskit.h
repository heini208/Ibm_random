#ifndef ARDUINO_QISKIT_H
#define ARDUINO_QISKIT_H

#include <Arduino.h>
#include <list>


#define SERIAL_TIMEOUT 5000 // Timeout in milliseconds

bool waitResponse() {
    unsigned long startTime = millis();
    while (!Serial.available()) {
        if (millis() - startTime > SERIAL_TIMEOUT) {
            Serial.println("Error: No response received from the quantum backend.");
            return false;
        }
        delay(10);
    }
    return true;
}

// Converts the response from the backend into an int array representing qubit measurement
std::list<double> convertResponseToDouble(String response, int numQubits) {
    int startIdx = response.indexOf("[");
    int endIdx = response.indexOf("]");

    if (startIdx == -1 || endIdx == -1 || endIdx <= startIdx) {
        Serial.println("Error: Invalid JSON format.");
        return std::list<double>();  // Return an empty list
    }

    // Extract the array content and split it into doubles
    String resultStr = response.substring(startIdx + 1, endIdx);
    std::list<double> result;

    for (size_t i = 0; i < resultStr.length(); i++) { // Change to size_t
        char c = resultStr[i];
        if (c == '0' || c == '1') {
            result.push_back(c - '0');  // Convert char to double and add to the list
            if (result.size() >= numQubits) break;
        }
    }
    return result;
}

// Requests qubit measurement and returns the measured qubit states as an array
std::list<double> requestQubitMeasurement(int numQubits) {
    String jsonString = "{\"action\": \"start_job\", \"num_qubits\": " + String(numQubits) + "}";
    Serial.println(jsonString);

    if (!waitResponse()) {
        return std::list<double>();  // Return an empty list if no response
    }

    String response = Serial.readStringUntil('\n');
    return convertResponseToDouble(response, numQubits); // Pass numQubits to convert the response
}

String startRealIBMJob(int numQubits) {
    String jsonString = "{\"action\": \"start_real_ibm_job\", \"num_qubits\": " + String(numQubits) + "}";
    Serial.println(jsonString);

    if (!waitResponse()) {
        return "ERROR";
    }

    String response = Serial.readStringUntil('\n');

    int startIdx = response.indexOf("\"job_id\": \"") + 10;
    int endIdx = response.indexOf("\"", startIdx + 1);
    if (startIdx == -1 || endIdx == -1) {
        Serial.println("Error: Invalid job ID format.");
        return "ERROR";
    }

    return response.substring(startIdx, endIdx);  // Extract job ID
}

String getJobStatus(String jobId) {
    String jsonString = "{\"action\": \"get_job_status\", \"job_id\": \"" + jobId + "\"}";
    Serial.println(jsonString);

    if (!waitResponse()) {
        return "ERROR";
    }

    String response = Serial.readStringUntil('\n');

    int startIdx = response.indexOf("\"job_status\": \"") + 14;
    int endIdx = response.indexOf("\"", startIdx + 1);
    if (startIdx == -1 || endIdx == -1) {
        Serial.println("Error: Invalid job status format.");
        return "ERROR";
    }

    return response.substring(startIdx, endIdx);  // Extract job status
}

std::list<double> getJobResult(String jobId, int numQubits) {
    String jsonString = "{\"action\": \"get_job_result\", \"job_id\": \"" + jobId + "\"}";
    Serial.println(jsonString);

    if (!waitResponse()) {
        return std::list<double>();  // Return empty if no response
    }

    String response = Serial.readStringUntil('\n');
    return convertResponseToDouble(response, numQubits);
}

bool configureIBMToken(String token) {
    String jsonString = "{\"action\": \"configure_ibm\", \"token\": \"" + token + "\"}";
    Serial.println(jsonString);

    if (!waitResponse()) {
        return false;
    }

    String response = Serial.readStringUntil('\n');
    return response.indexOf("\"status\": \"IBM token configured\"") != -1;
}


#endif
