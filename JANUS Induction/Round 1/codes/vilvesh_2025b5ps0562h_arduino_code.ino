//pin defns
const int forceSensorPin = A0;
const int ledAscending = 9;
const int ledApogee = 10;
const int ledDescending = 11;
const int buzzerPin = 12;

//vars for filtering sensor data
const int numReadings = 10;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int averageForce = 0;

//thresholds for state detection (to be calibrated)
const int thresholdAscend = 100;
const int thresholdDescend = 80;

//current and previous states
enum State {ASCENDING, APOGEE, DESCENDING};
State currentState = ASCENDING;
State previousState = ASCENDING;

void setup() {
  Serial.begin(9600);
  pinMode(ledAscending, OUTPUT);
  pinMode(ledApogee, OUTPUT);
  pinMode(ledDescending, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Initialize readings array to zero
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  //reading fsr and updating readings for moving average
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(forceSensorPin);
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;

  averageForce = total / numReadings;

  //printing force value for debugging
  Serial.println(averageForce);

  //state detection based on smoothed force value
  if (averageForce > thresholdAscend) {
    currentState = ASCENDING;
  }
  else if (averageForce < thresholdDescend) {
    currentState = DESCENDING;
  }
  else {
    currentState = APOGEE;
  }

  //if apoge state detected, activate buzzer briefly
  if (currentState == APOGEE && previousState != APOGEE) {
    tone(buzzerPin, 1000); // 1000Hz tone
    delay(500);
    noTone(buzzerPin);
  }

  //LEDs indication
  digitalWrite(ledAscending, currentState == ASCENDING);
  digitalWrite(ledApogee, currentState == APOGEE);
  digitalWrite(ledDescending, currentState == DESCENDING);

  previousState = currentState;

  //loop delay for sensor read stability	
  delay(100); 
}
