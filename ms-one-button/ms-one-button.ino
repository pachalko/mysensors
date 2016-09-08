/* 
 * ms-one-button
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
 * One Button library: https://github.com/mathertel/OneButton
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

// Using safe MIN settings, must try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_MIN

#include <SPI.h>
#include <MySensors.h>
#include <OneButton.h>

// id of the child sensor 
#define SENSOR_CHILD_ID 1 
   
// sleep time between update (in milliseconds)
unsigned long SLEEP_TIME = 120 * 1000; 

// Init one or more messages for anouncing state changes.
// For available variable types, see
// https://home-assistant.io/components/sensor.mysensors/
MyMessage msg(SENSOR_CHILD_ID, V_VAR1);

#define BUTTON_PIN 2 


// Connect a pushbutton to pin D2 and GND
OneButton button(BUTTON_PIN, true);

void setup() {
  button.setClickTicks(600);
  button.setPressTicks(1000);
  
  button.attachClick(onClick);
  button.attachDoubleClick(onDblClick);
  button.attachLongPressStart(onLongPressStart);
  button.attachLongPressStop(onLongPressStop);
}

void presentation()  {
  // Send the sketch version information 
  // to the gateway and controller
  sendSketchInfo("ms-generic-sensor", "1.0");

  // Register one or more sensors.
  // For available sensor types, see
  // https://home-assistant.io/components/sensor.mysensors/
  present(SENSOR_CHILD_ID, S_CUSTOM);
}

void loop() {
  // Check the button state
  button.tick();  

  // Sleep until interrupted or timeout
  sleep(digitalPinToInterrupt(BUTTON_PIN), CHANGE, SLEEP_TIME);
}

//
// Events to trigger
//
void onClick() {
  send(msg.set(1));  
}

void onDblClick() {
  send(msg.set(2));  
}

void onLongPressStart() {
  send(msg.set(3));  
}

void onLongPressStop() {
  send(msg.set(4));  
}





