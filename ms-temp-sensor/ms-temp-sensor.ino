/**
 * 
 * ms-temp-sensor 
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

// MIN settings is safe and works, but I perhaps should try to increase power
#define MY_RF24_PA_LEVEL RF24_PA_MIN

//#include <SPI.h>
#include <MySensors.h>
 
#define SENSOR_TEMP_CHILD_ID 1
#define SENSOR_HUMI_CHILD_ID 2
#define SENSOR_TIND_CHILD_ID 3

// Init one or more messages for anouncing state changes.
// For available variable types, see
// https://www.mysensors.org/download/serial_api_20#presentation
MyMessage msgT(SENSOR_TEMP_CHILD_ID, V_TEMP);
MyMessage msgH(SENSOR_HUMI_CHILD_ID, V_HUM);
MyMessage msgI(SENSOR_TIND_CHILD_ID, V_TEMP);

#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 2 

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  dht.begin();
}

void presentation()  {
  // Send the sketch version information 
  // to the gateway and controller
  sendSketchInfo("ms-temp-sensor", "1.0");

  // Register one or more sensors.
  // For available sensor types, see
  // https://www.mysensors.org/download/serial_api_20#message-type
  present(SENSOR_TEMP_CHILD_ID, S_TEMP);
  present(SENSOR_HUMI_CHILD_ID, S_HUM);
  present(SENSOR_TIND_CHILD_ID, S_TEMP);
}

float last_t = -1;
float last_h = -1;
float last_i = -1;

void loop() {
  sleep(5000); // with sleep, it draws ~3mA

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds old (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    #ifdef MY_DEBUG
    Serial.println("Failed to read from DHT sensor");
    #endif
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float i = dht.computeHeatIndex(t, h, false);

  if (t != last_t) {
    send(msgT.set(t, 2));  
    last_t = t;
  }
  
  if (h != last_h) {
    send(msgH.set(h, 2));  
    last_h = h;
  }

  if (i != last_i) {
    send(msgI.set(i, 2));  
    last_i = i;
  }
  
  #ifdef MY_DEBUG
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("*C ");
  Serial.print("Heat index: ");
  Serial.print(i);
  Serial.println("*C ");
  #endif
}
