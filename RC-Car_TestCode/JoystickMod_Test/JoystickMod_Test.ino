// ─── Pin Definitions ─────────────────────────────────────────────────────────
// Analog pin A1 connected to the X-axis output of the joystick module
// Moving the joystick left/right changes the voltage on this pin (0–1023)
const int X_AX = A1;

// Analog pin A0 connected to the Y-axis output of the joystick module
// Moving the joystick up/down changes the voltage on this pin (0–1023)
const int Y_AX = A0;


// ─── Global Variables ─────────────────────────────────────────────────────────
// Tracks how many loop iterations have passed since the last heading was printed
// Used to reprint the column header every 20 rows so the table stays readable
int rows = 0;


// ─── Setup (runs once on power-on) ───────────────────────────────────────────
void setup() {
  // Start serial communication at 9600 baud so values appear in the Serial Monitor
  Serial.begin(9600);

  // Print the column heading ("X axis  |  Y axis") before any readings appear
  showHeading();
}


// ─── Main Loop (runs repeatedly) ─────────────────────────────────────────────
void loop() {
  // Increment the row counter each iteration to track how many readings have printed
  rows++;

  // Every 20 rows, reprint the column heading so the table stays easy to read
  // as it scrolls — approximately every 6 seconds (20 rows × 300ms delay)
  if (rows > 20) {
    rows = 0;      // Reset counter back to 0
    showHeading(); // Reprint the "X axis | Y axis" header
  }

  // Read and print the current X-axis value from the joystick
  // analogRead() returns a value from 0–1023:
  //   ~0    = joystick pushed fully left
  //   ~512  = joystick centered (resting position)
  //   ~1023 = joystick pushed fully right
  Serial.print("          ");       // Indent for alignment
  Serial.print(analogRead(X_AX));   // Print raw X-axis ADC value

  Serial.print("          ");       // Spacing between columns
  Serial.println(analogRead(Y_AX)); // Print raw Y-axis ADC value, then newline
  // Note: Y-axis reads similarly — ~0 = fully down, ~512 = center, ~1023 = fully up

  // Wait 300ms before the next reading to debounce and keep output readable
  // Reducing this value gives faster updates; increasing it slows the output down
  delay(300);
}


// ─── Helper Function ─────────────────────────────────────────────────────────
// Prints the column headers to the Serial Monitor
// Called once on startup and again every 20 rows to keep the table labeled
void showHeading() {
  Serial.println("          X axis       Y axis");
}