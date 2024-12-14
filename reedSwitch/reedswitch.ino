#include <Wire.h>

#define REED_PIN 0x04  // PD2 (INT0) for reed switch
#define LED_PIN  0x20  // PD5 for LED

volatile uint8_t *ddr = &DDRD;     // Data Direction Register for PORTD
volatile uint8_t *port = &PORTD;   // Output Register for PORTD
volatile uint8_t *pin = &PIND;     // Input Register for PORTD
volatile uint8_t *eicra = &EICRA;  // External Interrupt Control Register A
volatile uint8_t *eimsk = &EIMSK;  // External Interrupt Mask Register

volatile bool stateChanged = false;
bool doorOpen = false;
const unsigned long debounceDelay = 50; // Shorter debounce time for sensitivity

unsigned long lastInterruptTime = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("Setup Started");

    // Configure REED_PIN as input with pull-up resistor
    *ddr &= ~REED_PIN;  // Set REED_PIN as input
    *port |= REED_PIN;  // Enable pull-up resistor

    // Configure LED_PIN as output
    *ddr |= LED_PIN;    // Set LED_PIN as output
    *port &= ~LED_PIN;  // Turn OFF LED initially

    // Initialize I2C as slave with address 0x08
    Wire.begin(0x08);
    Wire.onRequest(sendDoorStatus);

    // Set up external interrupt on REED_PIN for any logical change (rising or falling edge)
    *eicra |= (1 << ISC00); // Trigger interrupt on any change
    *eimsk |= (1 << INT0);  // Enable INT0 interrupt

    Serial.println("Setup Complete");
}

void loop() {
    // Main loop is very minimal to allow immediate reaction to reed switch changes
    if (stateChanged && millis() - lastInterruptTime >= debounceDelay) {
        stateChanged = false;
        bool currentDoorState = !(*pin & REED_PIN); // Read the current state of reed switch (LOW = open, HIGH = closed)

        // Update door state if it has changed
        if (currentDoorState != doorOpen) {
            doorOpen = currentDoorState;
            Serial.print("Door State: ");
            Serial.println(doorOpen ? "Open" : "Closed");

            // Control the LED based on door state
            if (doorOpen) {
                *port |= LED_PIN; // Turn ON LED
            } else {
                *port &= ~LED_PIN; // Turn OFF LED
            }
        }
    }
}

// Interrupt Service Routine for reed switch state change
ISR(INT0_vect) {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime >= debounceDelay) {
        stateChanged = true;        // Set the flag to indicate a state change
        lastInterruptTime = interruptTime; // Record the time of this interrupt
    }
}

// Function to send door status when requested via I2C
void sendDoorStatus() {
    Wire.write(doorOpen ? 1 : 0); // Send 1 if door is open, 0 if closed
    Serial.print("I2C Request Sent: ");
    Serial.println(doorOpen ? "Open" : "Closed");
}
