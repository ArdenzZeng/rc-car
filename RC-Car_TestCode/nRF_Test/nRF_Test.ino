// ─── Libraries ───────────────────────────────────────────────────────────────
// Core RF24 library for controlling the nRF24L01 wireless transceiver module
#include <RF24.h>
// RF24 configuration definitions (data rates, power levels, chip settings)
#include <RF24_config.h>
// nRF24L01 hardware register definitions and constants
#include <nRF24L01.h>
// Enables printf() support on Arduino (used by radio.printDetails())
#include <printf.h>
// SPI library — the nRF24L01 communicates with Arduino via the SPI bus
#include <SPI.h>


// ─── Radio Setup ─────────────────────────────────────────────────────────────
// Create the RF24 radio object
// Pin 8 = CE (Chip Enable): activates the module for TX/RX
// Pin 9 = CSN (Chip Select Not): selects the module on the SPI bus
RF24 radio(8, 9);

// Address that both the transmitter and receiver must share to communicate
// Acts like a "channel" — only matching addresses can talk to each other
const byte address[6] = "00001";

// Buffer to store incoming data from the transmitter (max 32 bytes for nRF24L01)
char receivedData[32] = "";

// Variables to hold the joystick X and Y axis values sent by the transmitter
// X axis controls left/right, Y axis controls forward/backward
int xAxis, yAxis;


// ─── Setup (runs once on power-on) ───────────────────────────────────────────
void setup() {
  // Start serial communication at 9600 baud for debugging in the Serial Monitor
  Serial.begin(9600);

  // Initialize the nRF24L01 radio module
  radio.begin();

  // Open reading pipe 1 on the shared address
  // Pipe 1 is one of 6 available data pipes — pipe 0 is reserved for TX acknowledgments
  radio.openReadingPipe(1, address);

  // Set data transmission rate to 1 Mbps
  // Lower rates (250 Kbps) give longer range; higher rates (2 Mbps) give faster throughput
  radio.setDataRate(RF24_1MBPS);

  // Set transmit power to LOW to reduce interference and power consumption
  // Options: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setPALevel(RF24_PA_LOW);

  // Put the radio into listening (receiver) mode
  // The transmitter uses radio.stopListening() to switch to sending mode instead
  radio.startListening();

  // Initialize printf support (required before calling radio.printDetails())
  printf_begin();

  // Print all radio configuration details to Serial Monitor for debugging
  // Shows address, data rate, power level, channel, and more
  radio.printDetails();
}


// ─── Main Loop (runs repeatedly) ─────────────────────────────────────────────
void loop() {
  // TODO: Add radio.available() check here to see if data has been received,
  // then use radio.read() to read joystick values into xAxis and yAxis,
  // and map those values to motor speed/direction commands
}