// ─── Libraries ───────────────────────────────────────────────────────────────
// Core RF24 library for controlling the nRF24L01 wireless transceiver module
#include <RF24.h>
// RF24 configuration definitions (data rates, power levels, chip settings)
#include <RF24_config.h>
// nRF24L01 hardware register definitions and constants
#include <nRF24L01.h>
// Enables printf() support on Arduino — useful for debugging radio config
#include <printf.h>

/*
ARDUINO JOYSTICK CONTROLLED CAR (TRANSMITTER)

YOU HAVE TO INSTALL THE RF24 LIBRARY BEFORE UPLOADING THE CODE
https://github.com/tmrh20/RF24/
*/

// SPI library — the nRF24L01 communicates with the Arduino via the SPI bus
#include <SPI.h>


// ─── Radio Setup ─────────────────────────────────────────────────────────────
// Create the RF24 radio object
// Pin 8 = CE (Chip Enable): activates the module for TX/RX operations
// Pin 9 = CSN (Chip Select Not): selects the nRF24L01 on the SPI bus
RF24 radio(8, 9);

// Address that both transmitter and receiver must share to communicate
// Acts like a "channel" — only radios with matching addresses can talk to each other
// This value must be identical in both the transmitter and receiver code
const byte address[6] = "00001";


// ─── Data Structure ───────────────────────────────────────────────────────────
// Packages both joystick axis readings into a single struct so they can be
// sent together in one radio transmission instead of two separate sends.
// The receiver must define this exact same struct to unpack the data correctly.
struct DataPacket {
  int xAxis;  // Joystick X-axis reading (0–1023); controls left/right steering
  int yAxis;  // Joystick Y-axis reading (0–1023); controls forward/backward throttle
};

// Global instance of DataPacket — reused every loop to hold the latest readings
DataPacket data;


// ─── Setup (runs once on power-on) ───────────────────────────────────────────
void setup() {
  // Start serial communication at 9600 baud for debugging in the Serial Monitor
  // Allows you to see what joystick values are being read and whether
  // transmissions are succeeding or failing in real time
  Serial.begin(9600);

  // Initialize the nRF24L01 radio module
  radio.begin();

  // Open the writing pipe on the shared address
  // The writing pipe is the outbound channel — this module will SEND on this address
  // The receiver opens a reading pipe on the same address to receive the data
  radio.openWritingPipe(address);

  // Set transmit power to LOW — reduces interference and power consumption
  // Use RF24_PA_HIGH if range is insufficient
  radio.setPALevel(RF24_PA_LOW);

  // Set data transfer rate to 1 Mbps — balances speed and range
  // Lower rates (RF24_250KBPS) give longer range at the cost of speed
  radio.setDataRate(RF24_1MBPS);

  // Set the radio channel — must match the receiver's channel exactly
  // Valid range is 0–125; channel 100 is used here to avoid common WiFi interference
  radio.setChannel(100);

  // Switch the radio to transmit (TX) mode
  // Unlike the receiver which calls startListening(), the transmitter calls
  // stopListening() to disable RX mode and enable active data sending
  radio.stopListening();
}


// ─── Main Loop (runs repeatedly) ─────────────────────────────────────────────
void loop() {
  // Read the current joystick position from both analog axes
  // analogRead() returns a value from 0–1023 based on the voltage on the pin:
  //   ~0    = joystick pushed fully in one direction
  //   ~512  = joystick centered (resting position)
  //   ~1023 = joystick pushed fully in the opposite direction
  data.xAxis = analogRead(A1);  // X-axis: controls left/right steering on the car
  data.yAxis = analogRead(A0);  // Y-axis: controls forward/backward throttle on the car

  // Print the current joystick readings to the Serial Monitor for debugging
  // Useful for verifying the joystick is being read correctly before transmitting
  Serial.print("Transmitting: ");
  Serial.print("X: ");
  Serial.print(data.xAxis);
  Serial.print(" Y: ");
  Serial.println(data.yAxis);

  // Transmit the DataPacket to the receiver
  // radio.write() sends the data and returns true if the receiver acknowledged it,
  // or false if no acknowledgment was received (out of range, interference, etc.)
  // sizeof(DataPacket) ensures exactly the right number of bytes are sent
  bool success = radio.write(&data, sizeof(DataPacket));

  // Print transmission result to Serial Monitor
  // "Data sent successfully" means the receiver acknowledged the packet
  // "Data send failed" means no acknowledgment — check range, channel, and address
  if (success) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Data send failed");
  }

  // Wait 200ms before the next transmission
  // This gives a ~5 transmissions/second rate — enough for smooth RC control
  // Reducing the delay makes control more responsive but increases radio traffic
  delay(200);
}
