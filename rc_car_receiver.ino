// ─── Libraries ───────────────────────────────────────────────────────────────
// Core RF24 library for controlling the nRF24L01 wireless transceiver module
#include <RF24.h>
// RF24 configuration definitions (data rates, power levels, chip settings)
#include <RF24_config.h>
// nRF24L01 hardware register definitions and constants
#include <nRF24L01.h>
// Enables printf() support on Arduino (used by radio.printDetails())
#include <printf.h>

/*
ARDUINO JOYSTICK CONTROLLED CAR (RECEIVER)

YOU HAVE TO INSTALL THE RF24 LIBRARY BEFORE UPLOADING THE CODE
https://github.com/tmrh20/RF24/
*/

// SPI library — the nRF24L01 communicates with the Arduino via the SPI bus
#include <SPI.h>


// ─── Motor Driver Pin Definitions (L298N) ────────────────────────────────────
// The L298N has two motor channels (A and B), each controlled by:
//   enX  = Enable pin — PWM signal that controls motor speed (0–255)
//   inX  = Input pins — determine the direction of the motor (HIGH/LOW combos)

#define enA 3  // Enable pin for Motor A — controls speed via PWM
#define in1 2  // Motor A direction input 1
#define in2 4  // Motor A direction input 2
// in1=HIGH, in2=LOW  → Motor A spins forward
// in1=LOW,  in2=HIGH → Motor A spins backward
// in1=LOW,  in2=LOW  → Motor A stopped

#define in3 5  // Motor B direction input 1
#define in4 7  // Motor B direction input 2
// in3=HIGH, in4=LOW  → Motor B spins forward
// in3=LOW,  in4=HIGH → Motor B spins backward
// in3=LOW,  in4=LOW  → Motor B stopped

#define enB 6  // Enable pin for Motor B — controls speed via PWM


// ─── Radio Setup ─────────────────────────────────────────────────────────────
// Create the RF24 radio object
// Pin 8 = CE (Chip Enable): activates the module for TX/RX operations
// Pin 9 = CSN (Chip Select Not): selects the nRF24L01 on the SPI bus
RF24 radio(8, 9);

// Address that both transmitter and receiver must share to communicate
// Acts like a "channel" — only radios with matching addresses can talk
const byte address[6] = "00001";


// ─── Data Structure ───────────────────────────────────────────────────────────
// Defines the structure of the data packet sent by the transmitter
// Both the transmitter and receiver must use the same struct layout
// so the data is interpreted correctly on both ends
struct DataPacket {
  int xAxis;  // Joystick X-axis value (0–1023); left/right steering
  int yAxis;  // Joystick Y-axis value (0–1023); forward/backward throttle
};

// Global instance of DataPacket to store the most recently received values
DataPacket data;

// FIX 3: motorSpeed is declared once here as a global so the floor check
// at the bottom of loop() correctly reads and clamps the same variable
// that each movement block writes to — previously each block re-declared
// it as a local int, making the floor check always read the stale global 0
int motorSpeed = 0;


// ─── Joystick Calibration Thresholds ─────────────────────────────────────────
// These values define the deadzone and movement boundaries for the joystick.
// A centered joystick reads approximately 512 on both axes (midpoint of 0–1023).
// Adjust these if your joystick's center resting position differs.
//
//   Deadzone (center band):  480 – 540  (xAxis and yAxis both treated as stopped)
//   Turn threshold:          480 / 540  (X-axis boundary for left/right turns)
//   Straight threshold:      460 / 560  (X-axis range considered "straight ahead")
//   Diagonal boundary:       460 / 560  (Y-axis boundary separating diagonal zones)
//
// To recalibrate: run the joystick test sketch and note the min/max/center
// readings for your specific module, then update the values below accordingly.
#define DEAD_LOW_X   480   // INSERT1 — lower X deadzone boundary
#define DEAD_HIGH_X  540   // INSERT2 — upper X deadzone boundary
#define DEAD_LOW_Y   480   // INSERT3 — lower Y deadzone boundary
#define DEAD_HIGH_Y  540   // INSERT4 — upper Y deadzone boundary
#define STRAIGHT_LOW  460  // INSERT5 — lower X bound for straight forward/backward
#define STRAIGHT_HIGH 560  // INSERT6 — upper X bound for straight forward/backward
#define DIAG_LOW     460   // INSERT7 — lower Y boundary for diagonal zones
#define DIAG_HIGH    560   // INSERT8 — upper Y boundary for diagonal zones


// ─── Setup (runs once on power-on) ───────────────────────────────────────────
void setup() {
  // Set all motor control pins as outputs so the Arduino can drive the L298N
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Start serial communication at 9600 baud for debugging in the Serial Monitor
  Serial.begin(9600);

  // Initialize the nRF24L01 radio module
  radio.begin();

  // Open reading pipe 1 on the shared address to receive incoming data
  radio.openReadingPipe(1, address);

  // Set transmit power to LOW — reduces interference and power draw
  radio.setPALevel(RF24_PA_LOW);

  // Set data rate to 1 Mbps — balances range and speed
  radio.setDataRate(RF24_1MBPS);

  // Set the radio channel — must match the transmitter's channel exactly (0–125)
  radio.setChannel(100);

  // Put the radio into listening (receiver) mode
  radio.startListening();

  // Initialize printf support (required before calling radio.printDetails())
  printf_begin();

  // Print all radio config to Serial Monitor for debugging
  // Confirms address, channel, data rate, and power level are set correctly
  radio.printDetails();
}


// ─── Main Loop (runs repeatedly) ─────────────────────────────────────────────
void loop() {

  // Check if a new data packet has arrived from the transmitter
  if (radio.available()) {

    // Read the incoming DataPacket into the global 'data' variable
    // sizeof(DataPacket) ensures exactly the right number of bytes are read
    radio.read(&data, sizeof(DataPacket));

    // Extract X and Y joystick values from the received packet into local variables
    int xAxis = data.xAxis;
    int yAxis = data.yAxis;

    // ── Debug Print (uncomment to enable) ──────────────────────────────────
    // Useful during testing to verify the correct values are being received
    /*
    Serial.print("X: ");
    Serial.print(xAxis);
    Serial.print(" Y: ");
    Serial.print(yAxis);
    Serial.println();
    */

    // ── Deadzone: Stop motors when joystick is centered ────────────────────
    // If both axes are within the center deadzone, cut all motor power
    if (xAxis > DEAD_LOW_X && xAxis < DEAD_HIGH_X && yAxis > DEAD_LOW_Y && yAxis < DEAD_HIGH_Y) {
      analogWrite(enA, 0);        // Cut power to Motor A
      analogWrite(enB, 0);        // Cut power to Motor B
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);     // Motor A: stopped
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);     // Motor B: stopped
      Serial.println(" Stopped");

    } else {

      // ── Forward ────────────────────────────────────────────────────────────
      // Y-axis pushed up (yAxis high) and X-axis is centered (not turning)
      // Both motors spin in the same forward direction at equal speed
      if (yAxis >= DEAD_HIGH_Y && xAxis >= STRAIGHT_LOW && xAxis <= STRAIGHT_HIGH) {
        motorSpeed = map(yAxis, DEAD_HIGH_Y, 1023, 0, 255); // Scale Y to PWM range
        analogWrite(enA, motorSpeed);
        analogWrite(enB, motorSpeed);
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);   // Motor A: forward
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);   // Motor B: forward
        Serial.println(" Forwards"); // FIX 1: was incorrectly printing "Backwards"

      // ── Backward ───────────────────────────────────────────────────────────
      // Y-axis pushed down (yAxis low) and X-axis is centered (not turning)
      // Both motors spin in the same backward direction at equal speed
      } else if (yAxis <= DEAD_LOW_Y && xAxis >= STRAIGHT_LOW && xAxis <= STRAIGHT_HIGH) {
        motorSpeed = map(yAxis, DEAD_LOW_Y, 0, 0, 255); // Scale inverted Y to PWM range
        analogWrite(enA, motorSpeed);
        analogWrite(enB, motorSpeed);
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);  // Motor A: backward
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);  // Motor B: backward
        Serial.println(" Backwards"); // FIX 1: was incorrectly printing "Forwards"

      // ── Turn Right ─────────────────────────────────────────────────────────
      // X-axis pushed right (xAxis high) and Y-axis is centered
      // Motor A (left side) drives forward, Motor B (right side) drives backward
      // The speed offset (+30) on enB makes the turn tighter
      } else if (xAxis >= DEAD_HIGH_X && yAxis >= DIAG_LOW && yAxis <= DIAG_HIGH) {
        motorSpeed = map(xAxis, DEAD_HIGH_X, 1023, 50, 150); // Scale X to turning speed
        analogWrite(enA, motorSpeed);
        analogWrite(enB, 30 + motorSpeed); // Slight boost on B for sharper turn
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);   // Motor A: forward
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);  // Motor B: backward
        Serial.println(" Turn Right");

      // ── Turn Left ──────────────────────────────────────────────────────────
      // X-axis pushed left (xAxis low) and Y-axis is centered
      // Motor A (left side) drives backward, Motor B (right side) drives forward
      } else if (xAxis <= DEAD_LOW_X && yAxis >= DIAG_LOW && yAxis <= DIAG_HIGH) {
        motorSpeed = map(xAxis, DEAD_LOW_X, 0, 50, 150); // Scale inverted X to turning speed
        analogWrite(enA, motorSpeed);
        analogWrite(enB, 30 + motorSpeed); // Slight boost on B for sharper turn
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);  // Motor A: backward
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);   // Motor B: forward
        Serial.println(" Turn Left");

      // ── Diagonal Q1 (Bottom-Right) ─────────────────────────────────────────
      // Y-axis low + X-axis right: car glides backward-right
      // Motor A runs slower than Motor B to curve the path rightward
      } else if (yAxis <= DIAG_LOW && xAxis >= STRAIGHT_HIGH) {
        motorSpeed = map(yAxis, DIAG_LOW, 0, 0, 255);
        analogWrite(enA, (motorSpeed / 3));               // Motor A: slower
        analogWrite(enB, 100 + (motorSpeed / 1.64));      // Motor B: faster
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);  // Motor A: backward
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);  // Motor B: backward
        Serial.println(" Q1 Glide");

      // ── Diagonal Q2 (Bottom-Left) ──────────────────────────────────────────
      // Y-axis low + X-axis left: car glides backward-left
      // Motor B runs slower than Motor A to curve the path leftward
      } else if (yAxis <= DIAG_LOW && xAxis <= STRAIGHT_LOW) {
        motorSpeed = map(yAxis, DIAG_LOW, 0, 0, 255);
        analogWrite(enA, 100 + (motorSpeed / 1.64));      // Motor A: faster
        analogWrite(enB, (motorSpeed / 3));               // Motor B: slower
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);  // Motor A: backward
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);  // Motor B: backward
        Serial.println(" Q2 Glide");

      // ── Diagonal Q3 (Top-Left) ─────────────────────────────────────────────
      // Y-axis high + X-axis left: car glides forward-left
      // Motor A runs faster than Motor B to curve the path leftward
      } else if (yAxis >= DIAG_HIGH && xAxis <= STRAIGHT_LOW) {
        motorSpeed = map(yAxis, DIAG_HIGH, 1023, 0, 255);
        analogWrite(enA, 100 + (motorSpeed / 1.64));      // Motor A: faster
        analogWrite(enB, (motorSpeed / 3));               // Motor B: slower
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);   // Motor A: forward
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);   // Motor B: forward
        Serial.println(" Q3 Glide");

      // ── Diagonal Q4 (Top-Right) ────────────────────────────────────────────
      // Y-axis high + X-axis right: car glides forward-right
      // Motor B runs faster than Motor A to curve the path rightward
      } else if (yAxis >= DIAG_HIGH && xAxis >= STRAIGHT_HIGH) {
        motorSpeed = map(yAxis, DIAG_HIGH, 1023, 0, 255);
        analogWrite(enA, (motorSpeed / 3));               // Motor A: slower
        analogWrite(enB, 100 + (motorSpeed / 1.64));      // Motor B: faster
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);   // Motor A: forward
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);   // Motor B: forward
        Serial.println(" Q4 Glide");
      }
    }

    // ── Motor Speed Floor ───────────────────────────────────────────────────
    // If motorSpeed drops below 10 (too low for motors to physically move),
    // clamp it to 0 to prevent motor hum/stalling at near-zero power
    // FIX 3: now correctly reads the global motorSpeed written by each movement
    // block above, instead of always checking the stale initial value of 0
    if (motorSpeed < 10) {
      motorSpeed = 0;
    }
  }
}
