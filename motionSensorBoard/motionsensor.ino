#include <Wire.h>

// Define GPIO pin for PIR sensor
#define PIR_SENSOR_PIN 2 // Using PD2 (pin 2) for PIR sensor

// I2C Slave Address for Board 2
#define I2C_SLAVE_ADDRESS 7

// Register Pointers for Direct Access
volatile uint8_t *ddr = &DDRD;  // Data Direction Register for PORTD
volatile uint8_t *port = &PORTD; // Output Register for PORTD
volatile uint8_t *pin = &PIND;   // Input Register for PORTD
volatile uint8_t *eicra = &EICRA; // External Interrupt Control Register A
volatile uint8_t *eimsk = &EIMSK; // External Interrupt Mask Register

// Debouncing Variables
volatile bool motionDetected = false; // Current motion detection state
volatile bool stableMotionState = false; // Stable state after debouncing
unsigned long lastMotionTime = 0; // Last time motion was detected
const unsigned long debounceDelay = 50; // Debounce delay in milliseconds

void setup() {
    // Configure PIR_SENSOR_PIN as input with pull-up resistor
    *ddr &= ~_BV(PIR_SENSOR_PIN);  // Clear bit for PIR_SENSOR_PIN (input)
    *port |= _BV(PIR_SENSOR_PIN);  // Enable pull-up resistor

    // Set up interrupt on PIR_SENSOR_PIN (triggered on CHANGE)
    *eicra |= _BV(ISC00); // Trigger interrupt on any logical change
    *eimsk |= _BV(INT0);  // Enable external interrupt INT0

    // Initialize I2C as a slave
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onRequest(sendMotionStatus);

    // Initialize Serial for debugging
    Serial.begin(9600);
    Serial.println("Board 2 Initialized: PIR Motion Sensor Ready");
    Serial.println("Waiting for motion...");
}

void loop() {
    unsigned long currentTime = millis();

    // Check if the motion state has stabilized
    if ((currentTime - lastMotionTime) > debounceDelay) {
        if (motionDetected != stableMotionState) {
            stableMotionState = motionDetected;

            // Log the stable state
            Serial.print("Stable Motion State: ");
            Serial.println(stableMotionState ? "Motion Detected" : "No Motion");
        }
    }

    // Periodic logging for stable state
    static unsigned long lastLogTime = 0;
    if (stableMotionState && (currentTime - lastLogTime > 500)) {
        Serial.print("Motion Detected at: ");
        Serial.println(currentTime);
        lastLogTime = currentTime;
    }
}

// Interrupt Service Routine for PIR sensor
ISR(INT0_vect) {
    // Update motionDetected state
    motionDetected = !(*pin & _BV(PIR_SENSOR_PIN)); // LOW = Motion Detected
    lastMotionTime = millis(); // Record the time of state change
}

// Function to send motion detection status to the main board (Board 4)
void sendMotionStatus() {
    // Send 1 if motion detected, 0 otherwise
    Wire.write(stableMotionState ? 1 : 0);

    // Log the I2C communication status
    if (stableMotionState) {
        Serial.println("Sending 'Motion Detected' over I2C");
    } else {
        Serial.println("Sending 'No Motion' over I2C");
    }
}
