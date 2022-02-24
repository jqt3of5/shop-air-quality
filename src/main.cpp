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
#include <HASwitch.h>
#include <sensirion_uart.h>

WiFiClient client;
HADevice device("workshop-environment-controller");
HAMqtt mqtt(client, device);

const char * ssid = "WaitingOnComcast";
const char * pwd = "1594N2640W";
const char * mqtt_host = "tiltpi.equationoftime.org";

const int led_pin = 2;

const int motion_sensor_pin = 3;
HABinarySensor * motionSensor = new HABinarySensor("motion-sensor", false);

HASensor * sps30_pm1 = new HASensor("sps30-sensor-1pm");
HASensor * sps30_pm2_5 = new HASensor("sps30-sensor-2.5pm");
HASensor * sps30_pm4 = new HASensor("sps30-sensor-4pm");
HASensor * sps30_pm10 = new HASensor("sps30-sensor-10pm");
HASensor * sps30_typical = new HASensor("sps30-sensor-typical-particle-size");

HASwitch * sps30_sleep_switch = new HASwitch("sps30-wake-switch", false);
HASwitch * sps30_clean_switch = new HASwitch("sps30-clean-switch", false);

const int dht_pin = 4;
DHT dht22(dht_pin,AM2301, 1);
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

void configureSPS30()
{
    sps30_pm1->setDeviceClass("pm1");
    sps30_pm1->setUnitOfMeasurement("µg/m³");

    sps30_pm2_5->setDeviceClass("pm25");
    sps30_pm2_5->setUnitOfMeasurement("µg/m³");

    sps30_pm4->setDeviceClass("pm25");
    sps30_pm4->setUnitOfMeasurement("µg/m³");

    sps30_pm10->setDeviceClass("pm10");
    sps30_pm10->setUnitOfMeasurement("µg/m³");

    sps30_typical->setDeviceClass("pm10");
    sps30_typical->setUnitOfMeasurement("µm");

    sps30_clean_switch->onStateChanged([](bool state, HASwitch * s) {
        if (state)
        {
            sps30_start_manual_fan_cleaning();
            //TODO: Does this work?
            //momentary switch
            sps30_clean_switch->setState(false);
        }
    });

    sps30_sleep_switch->onStateChanged([](bool state, HASwitch * s) {
        if (state){
            sps30_wake_up();
            //TODO: Is this too fast?
            uint16_t ret = sps30_start_measurement();
            if (ret < 0) {
                printf("error starting measurement\n");
            }
        }
        else {
            uint16_t ret = sps30_stop_measurement();
            if (ret) {
                printf("Stopping measurement failed\n");
            }
            sps30_sleep();
        }
    });

    sps30_clean_switch->setAvailability(false);
    sps30_sleep_switch->setAvailability(false);
    sps30_pm2_5->setAvailability(false);
    sps30_pm4->setAvailability(false);
    sps30_pm10->setAvailability(false);
    sps30_pm1->setAvailability(false);
    sps30_typical->setAvailability(false);

    bool spsAvailable = false;
    for (int i = 0; i < 5; ++i) {
        if (sensirion_uart_open() == 0) {
            spsAvailable = true;
            break;
        }
        Serial.printf("Failed to connect to sps30. Attempt %d\n", i);
    }

    if (spsAvailable)
    {
        for (int i = 0; i < 5; ++i) {
            if (sps30_probe() == 0) {
                spsAvailable = true;
                break;
            }
            Serial.printf("Failed to probe sps30. Attempt %d\n", i);
        }
    }

    if (spsAvailable) {
        sps30_clean_switch->setAvailability(true);
        sps30_sleep_switch->setAvailability(true);

        sps30_pm2_5->setAvailability(true);
        sps30_pm4->setAvailability(true);
        sps30_pm10->setAvailability(true);
        sps30_pm1->setAvailability(true);
        sps30_typical->setAvailability(true);
    }

    if (sps30_pm1->isOnline())
    {
        char serial[SPS30_MAX_SERIAL_LEN];
        const uint8_t AUTO_CLEAN_DAYS = 4;
        int16_t ret;

        ret = sps30_get_serial(serial);
        if (ret)
            Serial.printf("error %d reading serial\n", ret);
        else
            Serial.printf("SPS30 Serial: %s\n", serial);

        ret = sps30_set_fan_auto_cleaning_interval_days(AUTO_CLEAN_DAYS);
        if (ret)
            Serial.printf("error %d setting the auto-clean interval\n", ret);

        sps30_sleep();
    }
}

void readIfPossibleSPS30(){

    if (sps30_sleep_switch->isOnline() && sps30_sleep_switch->getState())
    {
        printf("measurements started\n");

        struct sps30_measurement m;
        uint16_t ret = sps30_read_measurement(&m);
        if (ret < 0) {
            printf("error reading measurement\n");
        } else {
            if (SPS30_IS_ERR_STATE(ret)) {
                Serial.printf(
                        "Chip state: %u - measurements may not be accurate\n",
                        SPS30_GET_ERR_STATE(ret));
            }
            sps30_pm1->setValue(m.mc_1p0);
            sps30_pm2_5->setValue(m.mc_2p5);
            sps30_pm4->setValue(m.mc_4p0);
            sps30_pm10->setValue(m.mc_10p0);
            sps30_typical->setValue(m.typical_particle_size);

//            printf("measured values:\n"
//                   "\t%0.2f pm1.0\n"
//                   "\t%0.2f pm2.5\n"
//                   "\t%0.2f pm4.0\n"
//                   "\t%0.2f pm10.0\n"
//                   "\t%0.2f nc0.5\n"
//                   "\t%0.2f nc1.0\n"
//                   "\t%0.2f nc2.5\n"
//                   "\t%0.2f nc4.5\n"
//                   "\t%0.2f nc10.0\n"
//                   "\t%0.2f typical particle size\n\n",
//                   m.mc_1p0, m.mc_2p5, m.mc_4p0, m.mc_10p0, m.nc_0p5,
//                   m.nc_1p0, m.nc_2p5, m.nc_4p0, m.nc_10p0,
//                   m.typical_particle_size);
        }
    }
}

void configureDHT22(){
    //configure dht22
    dht22Humidity->setName("Workshop Humidity");
    dht22Humidity->setDeviceClass("humidity");
    dht22Humidity->setUnitOfMeasurement("%");

    dht22Temperature->setName("Workshop Temperature");
    dht22Temperature->setDeviceClass("temperature");
    dht22Temperature->setUnitOfMeasurement("°F");

    dht22.begin();

    float humidity = dht22.readHumidity(true);
    float temp = dht22.readTemperature(true, false);

    dht22Humidity->setAvailability(humidity != NAN);
    dht22Temperature->setAvailability(temp != NAN);

    if (humidity != NAN)
    {
        dht22Humidity->setValue(humidity);
    }

    if (temp != NAN)
    {
        dht22Temperature->setValue(temp);
    }
}

void setup(){

    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    WiFi.setAutoReconnect(true);

    //TODO: A webportal to configure this would be awesome.
    do{
        //TODO: Save to internal log for recovery
        Serial.println("Attempting Wifi connection");
        WiFi.begin(ssid, pwd);
    } while (WiFi.waitForConnectResult() != WL_CONNECTED);

    Serial.println("wifi connected");

    //TODO: Publish saved logs some where

    //TODO: Setup logging endpoint
    setupOTA();

    mqtt.begin(mqtt_host);

    while (!mqtt.isConnected())
    {
        Serial.println("Mqtt not connected");
    }

    Serial.println("Mqtt connected");

    device.setName("Workshop Controller");
    device.enableSharedAvailability();
    device.setSoftwareVersion("1.0.0");
    device.enableLastWill();
    device.setAvailability(true);

    configureDHT22();

    configureSPS30();

    //configure motion sensor
    motionSensor->setName("Workshop Motion Sensor");
    motionSensor->setAvailability(true);

    pinMode(motion_sensor_pin, INPUT);
}

void loop(){
    mqtt.loop();
    ArduinoOTA.handle();

    int motion = digitalRead(motion_sensor_pin);
    if (motion != motionSensor->getState())
    {
        motionSensor->setState(motion);
    }

    //TODO: If enough time has passed
    if (false) {
        readIfPossibleSPS30();
    }

    if (false) {
        float humidity = dht22.readHumidity(false);
        float temp = dht22.readTemperature(true, false);

        if (humidity != NAN)
        {
            dht22Humidity->setValue(humidity);
        }

        if (temp != NAN)
        {
            dht22Temperature->setValue(temp);
        }
    }

}

