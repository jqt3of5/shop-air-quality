#include <Arduino.h>
#include <Wifi.h>
#include <HardwareSerial.h>
#include <HASensor.h>
#include <HADevice.h>
#include <HAMqtt.h>
#include <sps30.h>
#include <ArduinoOTA.h>
#include <HABinarySensor.h>
#include <DHT.h>

WiFiClient client;
HADevice device("workshop-environment-controller");
HAMqtt mqtt(client, device);

const int led_pin = 2;

const int motion_sensor_pin = 3;
HABinarySensor * motionSensor = new HABinarySensor("motion-sensor", false);

HASensor * sps30_pm1 = new HASensor("sps30-sensor-1pm");
HASensor * sps30_pm2_5 = new HASensor("sps30-sensor-2.5pm");
HASensor * sps30_pm4 = new HASensor("sps30-sensor-4pm");
HASensor * sps30_pm10 = new HASensor("sps30-sensor-10pm");

HASensor * dht22Humidity = new HASensor("dht22-sensor-humidity");
HASensor * dht22Temperature = new HASensor("dht22-sensor-temperature");


void setupOTA() {
    ArduinoOTA
            .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else // U_SPIFFS
                    type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
            })
            .onEnd([]() {
                Serial.println("\nEnd");
            })
            .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
            });

    ArduinoOTA.begin();
}
void setup(){

    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    //TODO: A webportal to configure this would be awesome.
    WiFi.begin("WaitingOnComcast", "1594N2640W");
    WiFi.setAutoReconnect(true);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    setupOTA();

    device.setName("Workshop Controller");
    device.enableSharedAvailability();
    device.setSoftwareVersion("1.0.0");
    device.enableLastWill();
    device.setAvailability(true);

    //configure dht22
    dht22Humidity->setName("Workshop Humidity");
    dht22Humidity->setDeviceClass("humidity");
    dht22Humidity->setUnitOfMeasurement("%");

    dht22Temperature->setName("Workshop Temperature");
    dht22Temperature->setDeviceClass("temperature");
//    dht22Temperature->setUnitOfMeasurement("%");

    dht22Humidity->setAvailability(true);
    dht22Temperature->setAvailability(true);

    //detect sps30



    //configure motion sensor
    motionSensor->setName("Workshop Motion Sensor");

    pinMode(motion_sensor_pin, INPUT);
    int motion = digitalRead(motion_sensor_pin);
    motionSensor->setState(motion);
    motionSensor->setAvailability(true);
}

void loop(){


}

