#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// I2C Addresses
#define BOARD1_ADDRESS 8  // Address of Board 1 (Magnetic reed switch)
#define BOARD2_ADDRESS 7  // Address of Board 2 (PIR motion sensor)
#define BOARD3_ADDRESS 9  // Address of Board 3 (LED and Buzzer control)

// GPIO Definitions
#define LED_PIN 4    // PD4 for the LED
#define BUZZER_PIN 3 // PD3 for the Buzzer

// Register Pointers for Direct Access
volatile uint8_t *ddr = &DDRD;  // Data Direction Register for PORTD
volatile uint8_t *port = &PORTD; // Data Register for PORTD
volatile uint8_t *pin = &PIND;   // Input Pin Register for PORTD

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust I2C address if needed

// Previous states for optimization
byte lastDoorStatus = 255;
byte lastMotionStatus = 255;

// Polling interval
unsigned long lastPollTime = 0;
const unsigned long pollInterval = 1000; // 1 second

void setup() {
    // Configure LED and Buzzer pins as outputs using bitmasking
    *ddr |= _BV(LED_PIN) | _BV(BUZZER_PIN);  // Set LED_PIN and BUZZER_PIN as outputs
    *port &= ~(_BV(LED_PIN) | _BV(BUZZER_PIN)); // Ensure both are OFF initially

    // Initialize I2C
    Wire.begin();

    // Initialize the LCD
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("System Ready");
    delay(2000);
    lcd.clear();

    // Debugging
    Serial.begin(9600);
    Serial.println("Board 4 Initialized");
}

void loop() {
    if (millis() - lastPollTime >= pollInterval) {
        lastPollTime = millis();
        handleDoorStatus();
        handleMotionStatus();
    }
}

// Handle door status from Board 1
void handleDoorStatus() {
    if (Wire.requestFrom(BOARD1_ADDRESS, 1) == 1) {
        byte doorStatus = Wire.read();
        if (doorStatus != lastDoorStatus) {
            lastDoorStatus = doorStatus;

            updateLCD(0, "Door: ", doorStatus == 1 ? "Open   " : "Closed ");
            Serial.print("Door Status: ");
            Serial.println(doorStatus == 1 ? "Open" : "Closed");

            // Send door status to Board 3 (control buzzer)
            Wire.beginTransmission(BOARD3_ADDRESS);
            Wire.write(doorStatus); // Send door signal (0 or 1)
            Wire.endTransmission();
        }
    }
}

// Handle motion status from Board 2
void handleMotionStatus() {
    if (Wire.requestFrom(BOARD2_ADDRESS, 1) == 1) {
        byte motionStatus = Wire.read();
        if (motionStatus != lastMotionStatus) {
            lastMotionStatus = motionStatus;

            updateLCD(1, "Motion: ", motionStatus == 1 ? "Detected" : "None    ");
            Serial.print("Motion Status: ");
            Serial.println(motionStatus == 1 ? "Detected" : "None");

            // Send motion status to Board 3 (control LED)
            Wire.beginTransmission(BOARD3_ADDRESS);
            Wire.write(motionStatus == 1 ? 3 : 2); // Send motion signal (3 or 2)
            Wire.endTransmission();
        }
    }
}

// Function to update the LCD efficiently
void updateLCD(byte row, const char *label, const char *value) {
    lcd.setCursor(0, row);
    lcd.print(label);
    lcd.print(value);
    lcd.print("          "); // Clear extra characters
}
