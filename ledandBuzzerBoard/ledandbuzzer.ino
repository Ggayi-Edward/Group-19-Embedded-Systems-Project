#include <Wire.h>

// Define GPIO pins for LED and Buzzer
#define LED_PIN 4    // Using PD4 (pin 4) for the LED
#define BUZZER_PIN 3 // Using PD3 (pin 3) for the Buzzer

// I2C Slave Address for Board 3
#define I2C_SLAVE_ADDRESS 9

// Direct memory access pointers for GPIO registers
volatile uint8_t *ddr = &DDRD;  // Data Direction Register for PORTD
volatile uint8_t *port = &PORTD; // Output Register for PORTD

// Variables for LED blinking
bool isBlinking = false;          // Tracks whether the LED should blink
unsigned long blinkStartTime = 0; // Timestamp when blinking started
const unsigned long blinkDuration = 4000; // Total blink time in milliseconds
const unsigned long blinkInterval = 500;  // Interval for LED ON/OFF in milliseconds
unsigned long lastBlinkTime = 0;  // Timestamp for the last toggle

void setup() {
    // Configure LED_PIN and BUZZER_PIN as outputs
    *ddr |= _BV(LED_PIN) | _BV(BUZZER_PIN); // Set LED_PIN and BUZZER_PIN as outputs
    *port &= ~(_BV(LED_PIN) | _BV(BUZZER_PIN)); // Ensure both are OFF initially

    // Initialize I2C as slave
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onReceive(receiveSignal);

    // Initialize Serial for debugging
    Serial.begin(9600);
    Serial.println("Board 3 Initialized: LED and Buzzer Ready");
}

void loop() {
    // Handle LED blinking
    handleLEDBlinking();
}

// Function to process received signals
void processSignal(byte signal) {
    switch (signal) {
        // Handle door status (control buzzer only)
        case 1: // Door open: Turn ON the buzzer
            Serial.println("Signal: Door Open. Turning Buzzer ON.");
            *port |= _BV(BUZZER_PIN);  // Turn ON Buzzer
            break;

        case 0: // Door closed: Turn OFF the buzzer
            Serial.println("Signal: Door Closed. Turning Buzzer OFF.");
            *port &= ~_BV(BUZZER_PIN); // Turn OFF Buzzer
            break;

        // Handle motion status (control LED only)
        case 3: // Motion detected: Start LED blinking
            if (!isBlinking) {  // Only start if not already blinking
                isBlinking = true;
                blinkStartTime = millis(); // Record the start time
                Serial.println("Signal: Motion Detected. LED Blinking Started.");
            }
            break;

        case 2: // No motion: Stop LED only if blinking is not active
            if (!isBlinking) {
                *port &= ~_BV(LED_PIN); // Ensure LED is OFF
                Serial.println("Signal: No Motion. LED Turned OFF.");
            } else {
                Serial.println("No Motion signal ignored; LED still blinking.");
            }
            break;

        default: // Unknown signal
            Serial.print("Unknown signal received: ");
            Serial.println(signal);
            break;
    }
}

// Function to handle received signal from Main Board
void receiveSignal(int numBytes) {
    while (Wire.available()) {
        byte signal = Wire.read(); // Read the signal
        Serial.print("Signal Received from Main Board: ");
        Serial.println(signal);
        processSignal(signal);  // Delegate to processSignal()
    }
}

// Function to handle LED blinking logic
void handleLEDBlinking() {
    if (isBlinking) {
        unsigned long currentTime = millis();

        // Stop blinking after the specified duration
        if (currentTime - blinkStartTime >= blinkDuration) {
            isBlinking = false; // Stop blinking
            *port &= ~_BV(LED_PIN); // Ensure LED is OFF
            Serial.println("LED Blinking Stopped: Time Elapsed");
            return;
        }

        // Toggle the LED state at the specified interval
        if (currentTime - lastBlinkTime >= blinkInterval) {
            *port ^= _BV(LED_PIN); // Toggle LED state
            lastBlinkTime = currentTime; // Update the last toggle time
        }
    }
}
