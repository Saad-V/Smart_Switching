/* Fill in information from your Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL_xxxxxxx"
#define BLYNK_TEMPLATE_NAME         "Smart Switch"
#define BLYNK_AUTH_TOKEN            "Your_Blynk_Auth_Token"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <IRremote.hpp> // Ensure you are using IRremote library version 3.0 or higher

// --- Network Credentials ---
char ssid[] = "Your_WiFi_SSID";
char pass[] = "Your_WiFi_Password";

// --- Pin Definitions (Based on Schematic) ---
#define IR_RECEIVER_PIN 15
#define RELAY_1 5
#define RELAY_2 18
#define RELAY_3 19
#define RELAY_4 21

// Most multi-channel relay modules are Active LOW. Change these if yours is Active HIGH.
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// --- IR Remote Command Codes (PLACEHOLDERS) ---
// Note: Read your Serial Monitor to find the specific commands for your remote
#define IR_BUTTON_1 0x12 // Toggles Relay 1
#define IR_BUTTON_2 0x34 // Master Off (All Relays)
#define IR_BUTTON_3 0x56 // Toggles Relay 3
#define IR_BUTTON_4 0x78 // Toggles Relay 4

// --- State Variables for Live Synchronization ---
bool state1 = false;
bool state2 = false; // Relay 2 can still be controlled via app/web, or add an IR button for it
bool state3 = false;
bool state4 = false;

void setup() {
  Serial.begin(115200);

  // Initialize Relay Pins
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

  // Set initial state to OFF
  digitalWrite(RELAY_1, RELAY_OFF);
  digitalWrite(RELAY_2, RELAY_OFF);
  digitalWrite(RELAY_3, RELAY_OFF);
  digitalWrite(RELAY_4, RELAY_OFF);

  // Initialize IR Receiver
  IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("IR Receiver Initialized.");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// --- Blynk App/Web Control (Virtual Pins V1 - V4) ---
BLYNK_WRITE(V1) {
  state1 = param.asInt();
  digitalWrite(RELAY_1, state1 ? RELAY_ON : RELAY_OFF);
}

BLYNK_WRITE(V2) {
  state2 = param.asInt();
  digitalWrite(RELAY_2, state2 ? RELAY_ON : RELAY_OFF);
}

BLYNK_WRITE(V3) {
  state3 = param.asInt();
  digitalWrite(RELAY_3, state3 ? RELAY_ON : RELAY_OFF);
}

BLYNK_WRITE(V4) {
  state4 = param.asInt();
  digitalWrite(RELAY_4, state4 ? RELAY_ON : RELAY_OFF);
}

// --- IR Remote Control Logic ---
void handleIR() {
  if (IrReceiver.decode()) {
    uint16_t command = IrReceiver.decodedIRData.command;
    Serial.print("IR Command Received: 0x");
    Serial.println(command, HEX);

    switch (command) {
      case IR_BUTTON_1:
        state1 = !state1;
        digitalWrite(RELAY_1, state1 ? RELAY_ON : RELAY_OFF);
        Blynk.virtualWrite(V1, state1); // Sync live status to App/Web
        break;

      case IR_BUTTON_2:
        // Master OFF Switch
        state1 = state2 = state3 = state4 = false;
        digitalWrite(RELAY_1, RELAY_OFF);
        digitalWrite(RELAY_2, RELAY_OFF);
        digitalWrite(RELAY_3, RELAY_OFF);
        digitalWrite(RELAY_4, RELAY_OFF);
        
        // Sync all statuses to App/Web
        Blynk.virtualWrite(V1, state1);
        Blynk.virtualWrite(V2, state2);
        Blynk.virtualWrite(V3, state3);
        Blynk.virtualWrite(V4, state4);
        break;

      case IR_BUTTON_3:
        state3 = !state3;
        digitalWrite(RELAY_3, state3 ? RELAY_ON : RELAY_OFF);
        Blynk.virtualWrite(V3, state3); // Sync live status to App/Web
        break;

      case IR_BUTTON_4:
        state4 = !state4;
        digitalWrite(RELAY_4, state4 ? RELAY_ON : RELAY_OFF);
        Blynk.virtualWrite(V4, state4); // Sync live status to App/Web
        break;
        
      default:
        // Ignore unmapped buttons
        break;
    }
    
    IrReceiver.resume(); // Prepare for the next signal
  }
}

void loop() {
  Blynk.run();
  handleIR();
}