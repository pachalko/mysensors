/**
 * 
 * ms-kitchen
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
 * This sketch controls:
 * - dimmable led light, with manual switch override
 * - door sensor
 * - temp/hum sensor
 * - motion sensor
 *
 * Copyright (c) 2016 Piotr Pachalko
 * MIT License: https://opensource.org/licenses/MIT
 *
 */

// Enable debug prints to serial monitor
// remove for "production"
// #define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24

// MIN settings is safe and works, but I perhaps should try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_LOW

#include <SPI.h>
#include <MySensors.h>

// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// https://github.com/gioblu/Agenda
#include <Agenda.h>

// https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>

#define LIGHT_CHILD_ID 1
#define LIGHT_SWITCH_PIN 2
#define LIGHT_DIMMER_PIN 3

#define DOOR_CHILD_ID 2
#define DOOR_SENSOR_PIN 4

#define MOTION_CHILD_ID 3
#define MOTION_PIN 7

 
#define TEMP_CHILD_ID 4
#define HUMI_CHILD_ID 5
#define TIND_CHILD_ID 6
#define DHT_TYPE DHT11
#define DHT_PIN 8

// Init one or more messages for anouncing state changes.
// For available variable types, see
// https://www.mysensors.org/download/serial_api_20#presentation
MyMessage msgLightStatus(LIGHT_CHILD_ID, V_STATUS);
MyMessage msgLightDimmer(LIGHT_CHILD_ID, V_DIMMER);

MyMessage msgDoor(DOOR_CHILD_ID, V_TRIPPED);

MyMessage msgMotion(MOTION_CHILD_ID, V_TRIPPED);

MyMessage msgTemp(TEMP_CHILD_ID, V_TEMP);
MyMessage msgHumi(HUMI_CHILD_ID, V_HUM);
MyMessage msgTempIndex(TIND_CHILD_ID, V_TEMP);

// connect a SPST manual switch to pin LIGHT_SWITCH_PIN and GND
// it controls light
Bounce lightSwitchDebouncer = Bounce();

// connect a SPST magnetic switch to pin DOOR_SENSOR_PIN and GND
// it checks door status
Bounce doorSensorDebouncer = Bounce();

// connect motion sensor to pin MOTION_PIN, GND and 3v3
// this sensor does not need debouncing, but using this API is nice
Bounce motionSensorDebouncer = Bounce();

// for periodic status updates
Agenda scheduler;

// connect DHT sensor to pin DHT_PIN, GND and 3v3
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {

  // light
  pinMode(LIGHT_DIMMER_PIN, OUTPUT);
  pinMode(LIGHT_SWITCH_PIN, INPUT_PULLUP);
  lightSwitchDebouncer.attach(LIGHT_SWITCH_PIN);
  lightSwitchDebouncer.interval(20); // msec

  // door
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);  
  doorSensorDebouncer.attach(DOOR_SENSOR_PIN);
  doorSensorDebouncer.interval(20); // msec

  // motion
  pinMode(MOTION_PIN, INPUT);
  motionSensorDebouncer.attach(MOTION_PIN);
  motionSensorDebouncer.interval(5); // msec

  // temp/hum
  dht.begin();
  
  // refresh state even if it didn't changed
  // (needed in case controler is reset)
  scheduler.insert(refreshLight, 1000000 * 60);
  scheduler.insert(refreshDoor, 1000000 * 60);

  // switch light off at reset
  analogWrite(LIGHT_DIMMER_PIN, LOW);
}

void presentation()  {
  
  // Send the sketch version information 
  // to the gateway and controller
  sendSketchInfo("ms-kitchen", "1.0");

  // Register one or more sensors.
  // For available sensor types, see
  // https://www.mysensors.org/download/serial_api_20#message-type
  present(LIGHT_CHILD_ID, S_DIMMER);
  present(DOOR_CHILD_ID, S_DOOR);
  present(MOTION_CHILD_ID, S_MOTION);
  present(TEMP_CHILD_ID, S_TEMP);
  present(HUMI_CHILD_ID, S_HUM);
  present(TIND_CHILD_ID, S_TEMP);
}

int dimmer = 0;
int memorized_dimmer = 100;
int fade_from = 0;
int last_switched_time = 0;


void loop()
{
  // handle light switch change
  if(lightSwitchDebouncer.update()) {
    
    // reset dimmer on double click
    int now = millis();
    if (now - last_switched_time < 500) { // ms
      memorized_dimmer = 100;
    }
    last_switched_time = now;

    fade_from = dimmer;
    dimmer    = dimmer == 0 ? memorized_dimmer : 0;
    
    #ifdef MY_DEBUG
      Serial.print(" light switch tripped > new state ");
      Serial.println(dimmer);
    #endif
    
    refreshLight();
  }

  // handle door sensor change
  if(doorSensorDebouncer.update()) {  
    #ifdef MY_DEBUG
      Serial.print(" door sensor tripped > new state ");
      Serial.println(doorSensorDebouncer.read() == 0 ? 1: 0);
    #endif
    
    send(msgDoor.set(doorSensorDebouncer.read() == 0 ? 1: 0));
  }

  // handle motion sensor update
  if (motionSensorDebouncer.update()) {
    #ifdef MY_DEBUG
      Serial.print(" motion sensor tripped > new state ");
      Serial.println(motionSensorDebouncer.read());
    #endif   
    
    send(msgMotion.set(motionSensorDebouncer.read()));     
  }

  // check and announce DHT state changes
  updateDHT();

  // must update scheduler once a loop
  scheduler.update();

  // wait and process incoming messages
  wait(10);  
}

void receive(const MyMessage &message) {
  
  if (message.type == V_DIMMER || message.type == V_STATUS) {   
     #ifdef MY_DEBUG
       Serial.print(" incoming msg > type ");
       Serial.print(message.type);
       Serial.print(" > sensor ");
       Serial.println(message.sensor);
       Serial.print(" > value ");
       Serial.println(message.getInt());
     #endif
     
     fade_from = dimmer;

     if (message.type == V_DIMMER) {
       dimmer = message.getInt();
       dimmer = dimmer > 100 ? 100 : dimmer;
       dimmer = dimmer < 0   ? 0   : dimmer;
       memorized_dimmer = dimmer == 0 ? memorized_dimmer : dimmer;
     } else { // V_STATUS
       dimmer = message.getInt() == 0 ? 0 : memorized_dimmer;
     }
     
     refreshLight();
   }
}

void refreshLight() {  
  
  while (dimmer != fade_from) {
    #ifdef MY_DEBUG
      Serial.print(" fade from ");
      Serial.println(fade_from);
    #endif
    
    int delta = dimmer - fade_from;
    int direction = delta > 0 ? 1 : -1;
    int step = min(abs(delta), 5);
    fade_from = fade_from + (step * direction);
    fade_from = fade_from > 100 ? 100 : fade_from;
    fade_from = fade_from < 0   ? 0   : fade_from;
    analogWrite(LIGHT_DIMMER_PIN, (int)(fade_from / 100. * 255));
    wait(50);
  }  
 
  send(msgLightStatus.set(dimmer > 0 ? 1 : 0));
  send(msgLightDimmer.set(dimmer));  
}

void refreshDoor() {  
  doorSensorDebouncer.update();
  send(msgDoor.set(doorSensorDebouncer.read() == 0 ? 1 : 0));
}

float last_temp = -1;
float last_humi = -1;
float last_tind = -1;

void updateDHT() {
  // Reading temperature or humidity takes about 250 milliseconds
  // Sensor readings may also be up to 2 seconds old (it's a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    #ifdef MY_DEBUG
      Serial.println(" failed to read from DHT sensor");
    #endif
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float i = dht.computeHeatIndex(t, h, false);

  if (t != last_temp) {
    #ifdef MY_DEBUG
      Serial.print(" temp > new state ");
      Serial.println(t);
    #endif
    
    send(msgTemp.set(t, 2));  
    last_temp = t;
  }
  
  if (h != last_humi) {
    #ifdef MY_DEBUG
      Serial.print(" humi > new state ");
      Serial.println(h);
    #endif
    
    send(msgHumi.set(h, 2));  
    last_humi = h;
  }

  if (i != last_tind) {
    #ifdef MY_DEBUG
      Serial.print(" temp index > new state ");
      Serial.println(i);
    #endif
    
    send(msgTempIndex.set(i, 2));  
    last_tind = i;
  }
}


