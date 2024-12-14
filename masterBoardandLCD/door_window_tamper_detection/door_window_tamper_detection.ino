#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BUTTON_PIN 2 // INT0 (Digital Pin D2)
#define LED_PIN 4    // Digital Pin D4

#define F_CPU 16000000UL // Define CPU frequency
#define BAUD 9600        // Define baud rate for serial communication
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

// UART Initialization
void UART_init() {
    UBRR0H = (UBRR_VALUE >> 8);   // Set baud rate high byte
    UBRR0L = UBRR_VALUE;          // Set baud rate low byte
    UCSR0B = (1 << TXEN0);        // Enable UART transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set 8 data bits, 1 stop bit
}

// UART Transmit Function
void UART_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait until the buffer is empty
    UDR0 = data;                      // Send data
}

// UART Print Function (for strings)
void UART_print(const char *str) {
    while (*str) {
        UART_transmit(*str++);
    }
}

// GPIO Initialization
void GPIO_init() {
    DDRD &= ~(1 << BUTTON_PIN); // Set D2 as input
    PORTD |= (1 << BUTTON_PIN); // Enable pull-up resistor on D2
    DDRD |= (1 << LED_PIN);     // Set D4 as output
    PORTD &= ~(1 << LED_PIN);   // Turn off LED initially
}

// Interrupt Initialization
void Interrupt_init() {
    EICRA = (1 << ISC01); // Trigger on falling edge
    EIMSK = (1 << INT0);  // Enable external interrupt INT0
    sei();                // Enable global interrupts
}

// Interrupt Service Routine for INT0
ISR(INT0_vect) {
    UART_print("Button pressed! Turning on LED.\n"); // Debug message
    PORTD |= (1 << LED_PIN);  // Turn on LED
    _delay_ms(1000);          // Keep LED on for 1 second
    PORTD &= ~(1 << LED_PIN); // Turn off LED
    UART_print("LED turned off.\n"); // Debug message
}

int main() {
    UART_init();              // Initialize UART for debugging
    UART_print("System initialized.\n"); // Debug message
    GPIO_init();              // Initialize GPIO pins
    Interrupt_init();         // Initialize interrupts

    while (1) {
        // Main loop remains empty since interrupt handles tamper detection
    }
}
