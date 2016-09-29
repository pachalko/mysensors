/**
 * 
 * ms-relay-switch-4ch
 * 
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * Copyright (c) 2016 Piotr Pachalko
 * MIT License: https://opensource.org/licenses/MIT
 *
 */

// Enable debug prints to serial monitor
// remove for production
// #define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24

// MIN settings is safe and works, but I try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_LOW

// Modify one default pin for radio
#define MY_RF24_CE_PIN 14

#include <SPI.h>
#include <MySensors.h>  

// ids of the ir sensors 
#define SENSOR_1_CHILD_ID 1
#define SENSOR_2_CHILD_ID 2
#define SENSOR_3_CHILD_ID 3
#define SENSOR_4_CHILD_ID 4

// pins of the ir sensors 
#define SENSOR_1_PIN 5
#define SENSOR_2_PIN 4
#define SENSOR_3_PIN 3
#define SENSOR_4_PIN 2

#define RELAY_1_PIN 6  
#define RELAY_2_PIN 7  
#define RELAY_3_PIN 8  
#define RELAY_4_PIN 9  

#define RELAY_ON 0  
#define RELAY_OFF 1 

// wait time between update (in milliseconds)
unsigned long WAIT_TIME = 2000; 

// Init one or more messages for anouncing state changes.
// For available variable types, see
// https://www.mysensors.org/download/serial_api_20#presentation
MyMessage msg1(SENSOR_1_CHILD_ID, V_STATUS);
MyMessage msg2(SENSOR_2_CHILD_ID, V_STATUS);
MyMessage msg3(SENSOR_3_CHILD_ID, V_STATUS);
MyMessage msg4(SENSOR_4_CHILD_ID, V_STATUS);

void setup()  
{ 
  pinMode(SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(SENSOR_3_PIN, INPUT_PULLUP);
  pinMode(SENSOR_4_PIN, INPUT_PULLUP);

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);

  // Make sure relays are off when starting up
  digitalWrite(RELAY_1_PIN, RELAY_OFF);
  digitalWrite(RELAY_2_PIN, RELAY_OFF);
  digitalWrite(RELAY_3_PIN, RELAY_OFF);
  digitalWrite(RELAY_4_PIN, RELAY_OFF);     
}

void presentation()  {
  // Send the sketch version information 
  // to the gateway and controller
  sendSketchInfo("ms-relay-switch_4ch", "1.0");

  // Register one or more sensors.
  // For available sensor types, see
  // https://home-assistant.io/components/sensor.mysensors/
  present(SENSOR_1_CHILD_ID, S_BINARY);
  present(SENSOR_2_CHILD_ID, S_BINARY);
  present(SENSOR_3_CHILD_ID, S_BINARY);
  present(SENSOR_4_CHILD_ID, S_BINARY);
}

int state_1 = RELAY_OFF;
int state_2 = RELAY_OFF;
int state_3 = RELAY_OFF;
int state_4 = RELAY_OFF;

void loop()      
{     
  // check status of ir sensors and send changes to gw 
  int state;
  state = digitalRead(SENSOR_1_PIN) ? RELAY_ON : RELAY_OFF;
  if (state != state_1) {
    send(msg1.set(state));  
    state_1 = state;
  }
  state = digitalRead(SENSOR_2_PIN) ? RELAY_ON : RELAY_OFF;
  if (state != state_2) {
    send(msg2.set(state));  
    state_2 = state;
  }
  state = digitalRead(SENSOR_3_PIN) ? RELAY_ON : RELAY_OFF;
  if (state != state_3) {
    send(msg3.set(state));  
    state_3 = state;
  }  
  state = digitalRead(SENSOR_4_PIN) ? RELAY_ON : RELAY_OFF;
  if (state != state_4) {
    send(msg4.set(state));  
    state_4 = state;
  }
  
  // wait for incoming messages
  wait(WAIT_TIME);
}

void receive(const MyMessage &message) {
  
  // only handle one type of message from gw
  if (message.type == V_LIGHT && 
      message.sensor >= SENSOR_1_CHILD_ID && 
      message.sensor <= SENSOR_4_CHILD_ID) {

     // kick the relay
     int relay_pin = message.sensor + RELAY_1_PIN - 1;
     digitalWrite(relay_pin, RELAY_ON);
     delay(100);
     digitalWrite(relay_pin, RELAY_OFF);
     
     #ifdef MY_DEBUG
     Serial.print("Incoming msg for relay:");
     Serial.print(message.sensor);
     Serial.print(" value: ");
     Serial.println(message.getBool());
     #endif
   } 
}
