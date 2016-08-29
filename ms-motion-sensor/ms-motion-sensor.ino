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

// Using safe MIN settings, must try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_MIN

#include <SPI.h>
#include <MySensors.h>  

// id of the child sensor 
#define CHILD_ID_MOTION 1 

// The D input where motion sensor is attached (only 2 and 3 generate interrupts)
#define INPUT_MOTION 3    

// Sleep time between reads (in milliseconds)
unsigned long SLEEP_TIME = 30000; 

// Init motion message
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);

void setup()  
{ 
  // Sets the sensor D pin as input 
  pinMode(INPUT_MOTION, INPUT);      
}

void presentation()  {
  // Send the sketch version information 
  // to the gateway and controller
  sendSketchInfo("ms-motion-sensor", "1.0");

  // Register all sensors to gateway 
  // (they will be created as child devices)
  present(CHILD_ID_MOTION, S_MOTION);
}

void loop()      
{     
  // Read motion state
  boolean tripped = digitalRead(INPUT_MOTION) == HIGH; 

  // Send tripped value to gw 
  send(msgMotion.set(tripped?"1":"0"));  

  // Sleep until interrupted, or timeout, whichever sooner
  sleep(digitalPinToInterrupt(INPUT_MOTION), CHANGE, SLEEP_TIME);
}



