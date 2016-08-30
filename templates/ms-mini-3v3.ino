/**
 * 
 * ms-motion-sensor
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
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24

// Using safe MIN settings, but must try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_MIN

#include <SPI.h>
#include <MySensors.h>  

// id of the child sensor 
#define SENSOR_CHILD_ID 1 
   
// sleep time between update (in milliseconds)
unsigned long SLEEP_TIME = 3000; 

// Init one or more messages for anouncing state changes.
// For available variable types, see
// https://home-assistant.io/components/sensor.mysensors/
MyMessage msg(SENSOR_CHILD_ID, V_VAR1);

void setup()  
{ 
  // do the setup, if necessary    
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

void loop()      
{     
  // Read the current state.
  // Typically, get it from attached sensor
  unsigned long state = millis(); 

  // Send value to gw 
  send(msg.set(state));  

  // Sleep a while
  sleep(SLEEP_TIME);
}
