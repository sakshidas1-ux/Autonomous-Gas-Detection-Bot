// ── Pin definitions ──────────────────────────
// Motor A (Left)
const int IN1 = 2;
const int IN2 = 3;
const int ENA = 6;

// Motor B (Right)
const int IN3 = 4;
const int IN4 = 5;
const int ENB = 7;

// Ultrasonic
const int TRIG = 8;
const int ECHO = 9;

// Gas sensors (MQ-9)
const int GAS_L = A0; 
const int GAS_R = A1; 

// IR Sensors (Edge/Line Detection)


// Buzzer
const int BUZZER = 13;

// ── Thresholds ───────────────────────────────
const int GAS_DANGER     = 200; // Stop and alarm
const int GAS_DETECT     = 40; // Start "seeking" behavior
const int OBSTACLE_CM    = 20;  // Distance to reverse
const int MOTOR_SPEED    = 170;// Base speed (0-255)

// ─────────────────────────────────────────────
void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  
  
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600); // Communication with ESP32
  stopMotors();
}

// ─────────────────────────────────────────────
void loop() {
  int gLeft  = analogRead(GAS_L);
  int gRight = analogRead(GAS_R);
  long dist  = getDistance();
  // Change this:
// serial.print("gleft",gLeft,"gRight",gRight,"distance",dist);

// To this:
  Serial.print("gLeft: "); Serial.print(gLeft);
  Serial.print(" gRight: "); Serial.print(gRight);
  Serial.print(" distance: "); Serial.println(dist);
  delay(100);
  // IR Sensors: HIGH usually means "No Surface/Edge Detected" 
  // depending on your specific IR module type.
  // bool edgeLeft  = digitalRead(IR_L); 
  // bool edgeRight = digitalRead(IR_R);

  // 1. CRITICAL GAS LEAK CHECK
  if (gLeft > GAS_DANGER || gRight > GAS_DANGER) {
    stopMotors();
    digitalWrite(BUZZER, HIGH);

    sendDataToESP(gLeft, gRight, dist, "EMERGENCY");
    delay(100); // Lock in state
    return; 
  } else {
    digitalWrite(BUZZER, LOW);
  }

  // 2. EDGE DETECTION (Safety First)
  // if (edgeLeft || edgeRight) {
  //   stopMotors();
  //   moveBackward();
  //   delay(500);
  //   turnRight(); // Turn away from edge
  //   delay(500);
  //   return;
  // }

  // 3. OBSTACLE AVOIDANCE
  if (dist < OBSTACLE_CM) {
    stopMotors();
    delay(200);
    moveBackward();
    delay(400);
    gLeft > gRight ? turnLeft() : turnRight(); // Turn toward gas if seen
    delay(500);
    return;
  }

  // 4. GAS SEEKING & NAVIGATION
  // If gas is detected but not yet dangerous, steer toward it
  if (gLeft > GAS_DETECT || gRight > GAS_DETECT) {
    if (gLeft > gRight + 50) {
      turnLeft(); // Veer toward higher concentration
    } else if (gRight > gLeft + 50) {
      turnRight();
    } else {
      moveForward();
    }
    sendDataToESP(gLeft, gRight, dist, "SEEKING");
  } else {
    moveForward();
    sendDataToESP(gLeft, gRight, dist, "SCANNING");
  }

  delay(50); 
}

// ── Helper Functions ─────────────────────────

void sendDataToESP(int l, int r, long d, String stat) {
  // Format for ESP32 Parsing: GAS_L,GAS_R,DIST,STATUS
  Serial.print(l); Serial.print(",");
  Serial.print(r); Serial.print(",");
  Serial.print(d); Serial.print(",");
  Serial.println(stat);
}

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 25000); 
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
}

void turnLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}