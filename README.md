# Wireless RC Car 🚗

An Arduino-based RC car controlled by a custom wireless joystick transmitter. The car receives real-time joystick input over a 2.4GHz radio link and translates it into motor commands — supporting forward, backward, left/right turns, and smooth diagonal movement.

Built as a personal hardware project and demoed at high school.

---

## 📸 Demo

<!-- Upload your photos to the repo and replace the filenames below -->
![RC Car](imagesRC-Car/RC_Car.jpg)
![Transmitter](imagesRC-Car/transmitterSchematic.jpg)
![Reciever](imagesRC-Car/recieverSchematic.jpg)
---

## 🔧 How It Works

The project is split into two Arduinos:

- **Transmitter** (joystick controller): Reads X and Y axis values from the joystick module and sends them wirelessly as a data packet via the nRF24L01 radio module.
- **Receiver** (the car): Listens for incoming data packets and maps the joystick values to motor speed and direction commands sent to the L298N motor driver, which drives two DC motors.

The joystick's analog output (0–1023) is mapped to 8 movement zones: forward, backward, turn left, turn right, and four diagonal glide directions. A center deadzone prevents motor jitter when the joystick is at rest.

---

## 🛒 Components

| Component | Quantity | Purpose |
|---|---|---|
| Arduino Nano | 2 | Microcontroller for transmitter and receiver |
| nRF24L01 Wireless Module | 2 | 2.4GHz radio communication between the two Arduinos |
| Joystick Module | 1 | Reads X/Y position input from the user |
| L298N Motor Driver | 1 | Controls speed and direction of the DC motors |
| DC Motors | 2 | Drives the wheels of the car |
| RC Car Chassis | 1 | Frame, wheels, and battery holder |
| Jumper Wires | — | Connecting components |
| Power Supply | 1 | Battery pack to power the car |

---

## 🔌 Wiring Notes

### Receiver (Car) — Arduino Nano → L298N Motor Driver
| Arduino Pin | L298N Pin |
|---|---|
| D3 (PWM) | ENA — Motor A speed |
| D2 | IN1 — Motor A direction |
| D4 | IN2 — Motor A direction |
| D5 | IN3 — Motor B direction |
| D7 | IN4 — Motor B direction |
| D6 (PWM) | ENB — Motor B speed |

### Both Arduinos → nRF24L01
| Arduino Pin | nRF24L01 Pin |
|---|---|
| D8 | CE |
| D9 | CSN |
| D11 (MOSI) | MOSI |
| D12 (MISO) | MISO |
| D13 (SCK) | SCK |
| 3.3V | VCC ⚠️ Must use 3.3V, not 5V |
| GND | GND |

### Transmitter (Controller) — Joystick Module → Arduino Nano
| Arduino Pin | Joystick Pin |
|---|---|
| A0 | Y-axis |
| A1 | X-axis |
| 5V | VCC |
| GND | GND |

> ⚠️ The nRF24L01 module must be powered with **3.3V**, not 5V — connecting it to 5V will damage the module.

---

## 💻 Code

The project contains three sketches:

| Folder | Sketch | Description |
|---|---|---|
| `receiver/` | `rc_car_receiver.ino` | Upload to the Arduino on the car |
| `transmitter/` | `rc_car_transmitter.ino` | Upload to the Arduino in the joystick controller |
| `joystick_test/` | `joystick_test.ino` | Standalone test to verify joystick readings over Serial Monitor |

### Installing the RF24 Library
Before uploading either sketch, install the RF24 library in the Arduino IDE:
1. Open Arduino IDE → **Sketch** → **Include Library** → **Manage Libraries**
2. Search for `RF24` by TMRh20
3. Click **Install**

### Uploading
1. Open the sketch for whichever Arduino you are programming
2. Select the correct board: **Tools** → **Board** → **Arduino Nano**
3. Select the correct port: **Tools** → **Port**
4. Click **Upload**
5. Repeat for the second Arduino with the other sketch

### Calibration
The receiver sketch uses threshold values defined at the top of the file to determine the joystick's deadzone and movement boundaries. If your car behaves unexpectedly, run `joystick_test.ino` first to read your joystick's actual center and min/max values, then update the `#define` constants in `rc_car_receiver.ino` accordingly.

---

*Built by Arden Zeng — ECE @ Princeton University*
