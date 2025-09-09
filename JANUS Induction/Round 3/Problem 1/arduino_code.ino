// Sketch: Quectel L89HA -> TinyGPS++ parsing + PSTM setup + simple altitude state machine
// For Arduino Uno (SoftwareSerial). If you have a Mega, replace SoftwareSerial with Serial1.

#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define GPS_RX_PIN 4  // Arduino pin that receives from module TX
#define GPS_TX_PIN 3  // Arduino pin that transmits to module RX
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN); // RX, TX
TinyGPSPlus gps;

enum FlightState { IDLE, ASCENT, APOGEE, DESCENT, PAYLOAD_DEPLOYED, LANDED };
FlightState state = IDLE;

// State machine parameters (tweak these for your flight)
const float ASCEND_THRESHOLD_M = 3.0;       // altitude rise over ground to treat as ascent
const int  CONSEC_READS_TO_CONFIRM = 3;     // how many consistent readings to change state
const float LANDED_MARGIN_M = 2.0;          // within this of ground => landed
const float SMOOTH_ALPHA = 0.3;             // EMA smoothing factor for altitude

float groundAlt = NAN;
float smoothedAlt = NAN;
float lastSmoothedAlt = NAN;
float peakAlt = -1e9;
bool payloadDeployedFlag = false;

int upCount = 0;
int downCount = 0;

// ============ Helper: compute NMEA checksum (two hex chars) ============
String nmeaChecksum(const String &body) {
  uint8_t cs = 0;
  for (size_t i = 0; i < body.length(); ++i) cs ^= (uint8_t)body[i];
  char buf[3];
  sprintf(buf, "%02X", cs);
  return String(buf);
}

void sendPSTMCommand(const String &body) {
  // body example: PSTMCFGMSGL,0,1,00000042,00000000
  String cs = nmeaChecksum(body);
  String full = "$" + body + "*" + cs + "\r\n";
  Serial.print("-> Sending to module: ");
  Serial.print(full); // echo to debug
  gpsSerial.print(full);
  delay(200);
  // optionally read response for a short while:
  unsigned long start = millis();
  while (millis() - start < 300) {
    while (gpsSerial.available()) {
      char c = gpsSerial.read();
      Serial.write(c); // show module responses (OK messages)
    }
  }
}

// Convert TinyGPS's time to hh:mm:ss string
String timeString() {
  if (!gps.time.isValid()) return String("----");
  char buf[16];
  sprintf(buf, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
  return String(buf);
}

String latLonString() {
  if (!gps.location.isValid()) return String("no-fix");
  char buf[64];
  dtostrf(gps.location.lat(), 8, 6, buf);
  String sLat(buf);
  dtostrf(gps.location.lng(), 9, 6, buf);
  String sLon(buf);
  return sLat + ", " + sLon;
}

void updateStateMachine(float alt) {
  if (isnan(groundAlt)) {
    // initialize ground altitude after a few stable readings
    static int stableCount = 0;
    static float lastAlt = 0;
    if (stableCount == 0) lastAlt = alt;
    if (fabs(alt - lastAlt) < 1.0) {
      stableCount++;
    } else {
      stableCount = 0;
      lastAlt = alt;
    }
    if (stableCount >= 3) {
      groundAlt = (lastAlt + alt) / 2.0;
      Serial.print("Ground altitude set to: ");
      Serial.print(groundAlt);
      Serial.println(" m");
      state = IDLE;
    }
    return;
  }

  // maintain peak
  if (alt > peakAlt) peakAlt = alt;

  // detect ascent/descent trend using smoothed altitude
  if (smoothedAlt > lastSmoothedAlt + 0.2) { // rising
    upCount++; downCount = 0;
  } else if (smoothedAlt < lastSmoothedAlt - 0.2) { // falling
    downCount++; upCount = 0;
  } else {
    upCount = downCount = 0;
  }

  switch (state) {
    case IDLE:
      if ((alt > groundAlt + ASCEND_THRESHOLD_M) && upCount >= CONSEC_READS_TO_CONFIRM) {
        state = ASCENT;
        Serial.println("STATE -> ASCENT");
      }
      break;
    case ASCENT:
      // update peak as we go
      if (downCount >= CONSEC_READS_TO_CONFIRM) {
        state = APOGEE;
        Serial.println("STATE -> APOGEE (peak reached)");
      }
      break;
    case APOGEE:
      // if continues to descend, switch to DESCENT
      if (downCount >= CONSEC_READS_TO_CONFIRM) {
        state = DESCENT;
        Serial.println("STATE -> DESCENT");
      }
      break;
    case DESCENT:
      // when altitude crosses 75% of peak -> PAYLOAD deployed
      if (!payloadDeployedFlag && (alt <= 0.75 * peakAlt)) {
        payloadDeployedFlag = true;
        state = PAYLOAD_DEPLOYED;
        Serial.println("STATE -> PAYLOAD_DEPLOYED (75% of peak reached)");
      }
      // landed?
      if (alt <= (groundAlt + LANDED_MARGIN_M) && fabs(smoothedAlt - lastSmoothedAlt) < 0.2) {
        state = LANDED;
        Serial.println("STATE -> LANDED");
      }
      break;
    case PAYLOAD_DEPLOYED:
      if (alt <= (groundAlt + LANDED_MARGIN_M) && fabs(smoothedAlt - lastSmoothedAlt) < 0.2) {
        state = LANDED;
        Serial.println("STATE -> LANDED");
      }
      break;
    case LANDED:
      // stay here
      break;
  }
}

void printStatus() {
  Serial.print("Time: "); Serial.print(timeString());
  Serial.print(" | Lat/Lon: "); Serial.print(latLonString());
  Serial.print(" | Alt(MSL): ");
  if (gps.altitude.isValid()) {
    Serial.print(gps.altitude.meters(), 2);
  } else {
    Serial.print("no-fix");
  }
  Serial.print(" | State: ");
  switch(state) {
    case IDLE: Serial.print("IDLE"); break;
    case ASCENT: Serial.print("ASCENT"); break;
    case APOGEE: Serial.print("APOGEE"); break;
    case DESCENT: Serial.print("DESCENT"); break;
    case PAYLOAD_DEPLOYED: Serial.print("PAYLOAD_DEPLOYED"); break;
    case LANDED: Serial.print("LANDED"); break;
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting L89 + TinyGPS++ example");
  gpsSerial.begin(9600); // L89 default NMEA rate (9600)
  delay(200);

  // 1) Configure module to output only GPRMC & GPGGA (mask 0x00000042)
  // PSTMCFGMSGL,0,1,00000042,00000000 -> checksum 4A (precomputed)
  sendPSTMCommand("PSTMCFGMSGL,0,1,00000042,00000000");
  delay(200);
  sendPSTMCommand("PSTMSAVEPAR"); // saves: checksum 58
  delay(200);
  sendPSTMCommand("PSTMSRR"); // reset: checksum 49
  delay(4000); // wait for module to reboot

  Serial.println("Configuration sent; waiting for GNSS sentences...");
}

unsigned long lastPrint = 0;

void loop() {
  // feed gps parser
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gps.encode(c);
  }

  // if we have a new altitude reading, update smoothed altitude and state machine
  if (gps.altitude.isUpdated()) {
    float alt = gps.altitude.meters(); // in meters (MSL)
    if (isnan(smoothedAlt)) {
      smoothedAlt = alt;
    } else {
      smoothedAlt = SMOOTH_ALPHA * alt + (1.0 - SMOOTH_ALPHA) * smoothedAlt;
    }

    // update state machine using smoothed value
    updateStateMachine(smoothedAlt);

    // store lastSmoothedAlt for trend detection
    lastSmoothedAlt = smoothedAlt;
  }

  // Print status once per second
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    printStatus();
  }
}
