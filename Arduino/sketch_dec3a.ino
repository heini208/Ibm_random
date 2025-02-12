int result = 0;
int lastResult = 3;

void setup() {
  Serial.begin(9600);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Waiting for serial input...");
}

void loop() {
  if (Serial.available() > 0) {
    result = Serial.read();
    Serial.print("Received: ");
    Serial.println(result);
    
    if (result == 1 && lastResult != 1) {
      Serial.println("received 1");
      lastResult = 1;
      digitalWrite(8, HIGH);
      digitalWrite(9, LOW);
      digitalWrite(LED_BUILTIN, HIGH);
    } else if (result == 0 && lastResult !=0){
      digitalWrite(9, HIGH);
      digitalWrite(8, LOW);
      digitalWrite(LED_BUILTIN, LOW);
      lastResult = 0;

    }
  }
}
