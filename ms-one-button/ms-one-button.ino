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

#include <OneButton.h>

// Connect a pushbutton to pin D2 and GND
OneButton button(2, true);

void setup() {
  Serial.begin(9600);

  button.setClickTicks(600);
  button.setPressTicks(1000);
  
  button.attachClick(onClick);
  button.attachDoubleClick(onDblClick);
  button.attachLongPressStart(onLongPressStart);
  button.attachLongPressStop(onLongPressStop);
}

void loop() {
  button.tick();
  delay(10);   
}

//
// Events to trigger
//
void onClick() {
  Serial.println("click");
}

void onDblClick() {
  Serial.println("click-click");
}

void onLongPressStart() {
  Serial.println("long press start");
}

void onLongPressStop() {
  Serial.println("long press stop");
}





