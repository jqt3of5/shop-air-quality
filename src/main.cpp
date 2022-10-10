#include <Arduino.h>
#include <WiFi.h>
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
#include <esp_task_wdt.h>
#include <GLog.h>
#include <ESPmDNS.h>
#include <wenfilter.h>
#include "HAFan.h"

const char * mDNSname= "workshopcontroller1";
const char * deviceId = "workshop-environment-controller-1";
const char * entityPrefix = "wec1";
const char * ssid = "WaitingOnComcast";
const char * pwd = "1594N2640W";
const char * mqtt_host = "tiltpi.equationoftime.tech";
const char * graylog_host = "dockervm";
const char * SOFTWARE_VERSION = "1.0.0";

const int WDT_TIMEOUT = 5;
TaskHandle_t _taskHandle;

WiFiClient client;
HADevice device(deviceId);
HAMqtt mqtt(client, device);

HASensor * uptime = new HASensor("wec1_uptime");

const int motion_sensor_pin = 35;
HABinarySensor * motionSensor = new HABinarySensor("wec1_motion_sensor", false);

HASensor * sps30_pm1 = new HASensor("wec1_sps30_1pm");
HASensor * sps30_pm2_5 = new HASensor("wec1_sps30_2_5pm");
HASensor * sps30_pm4 = new HASensor("wec1_sps30_4pm");
HASensor * sps30_pm10 = new HASensor("wec1_sps30_10pm");
HASensor * sps30_typical = new HASensor("wec1_sps30_typical_particle_size");

HASwitch * sps30_sleep_switch = new HASwitch("wec1_sps30_wake", false);
HASwitch * sps30_clean_switch = new HASwitch("wec1_sps30_clean", false);

const int current_sensor1_pin = 32;
HASensor * current_sensor1 = new HASensor("wec1_current_sensor1");

const int dht_pin = 26;
DHT dht22(dht_pin,AM2301, 1);
HASensor * dht22Humidity = new HASensor("wec1_dht22_humidity");
HASensor * dht22Temperature = new HASensor("wec1_dht22_temperature");

const int transmitter433_pin = 12;
WenFilter wenAirFilterDevice (transmitter433_pin);
HAFan * wenAirFilter = new HAFan("wec1_wen_filter", HAFan::SpeedsFeature);

void setupOTA() {
    ArduinoOTA
            .onStart([]() {
                esp_task_wdt_delete(_taskHandle);
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else // U_SPIFFS
                    type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Log.printf("Start OTA type: %s", type);
            })
            .onEnd([]() {
                Log.printf("End OTA");
            })
            .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
                Log.printf("OTA Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Log.printf("Auth Failed\n");
                else if (error == OTA_BEGIN_ERROR) Log.printf("Begin Failed\n");
                else if (error == OTA_CONNECT_ERROR) Log.printf("Connect Failed\n");
                else if (error == OTA_RECEIVE_ERROR) Log.printf("Receive Failed\n");
                else if (error == OTA_END_ERROR) Log.printf("End Failed\n");
            });

    ArduinoOTA.begin();
}

void configureSPS30()
{
    sps30_pm1->setName("0.3 - 1.0 µm");
    sps30_pm1->setDeviceClass("pm1");
    sps30_pm1->setUnitOfMeasurement("µg/m³");

    sps30_pm2_5->setName("0.3 - 2.5 µm");
    sps30_pm2_5->setDeviceClass("pm25");
    sps30_pm2_5->setUnitOfMeasurement("µg/m³");

    sps30_pm4->setName("0.3 - 4.0 µm");
    sps30_pm4->setDeviceClass("pm25");
    sps30_pm4->setUnitOfMeasurement("µg/m³");

    sps30_pm10->setName("0.3 - 10.0 µm");
    sps30_pm10->setDeviceClass("pm10");
    sps30_pm10->setUnitOfMeasurement("µg/m³");

    sps30_typical->setName("pm typical");
    sps30_typical->setDeviceClass("pm10");
    sps30_typical->setUnitOfMeasurement("µm");

    sps30_clean_switch->setName("Start Self Cleaning");
    sps30_clean_switch->onStateChanged([](bool state, HASwitch * s) {
        if (state)
        {
            sps30_start_manual_fan_cleaning();
            //TODO: Does this work?
            //momentary switch
            sps30_clean_switch->setState(false);
        }
    });

    sps30_sleep_switch->setName("Actively Reading");
    sps30_sleep_switch->onStateChanged([](bool state, HASwitch * s) {
        if (state){
//            sps30_wake_up();
            //TODO: Is this too fast?
            uint16_t ret = sps30_start_measurement();
            if (ret < 0) {
                Log.printf("Error starting measurement. Code: %d\n", ret);
            }

            sps30_pm2_5->setAvailability(ret >= 0);
            sps30_pm4->setAvailability(ret >= 0);
            sps30_pm10->setAvailability(ret >= 0);
            sps30_pm1->setAvailability(ret >= 0);
            sps30_typical->setAvailability(ret >= 0);
        }
        else {
            uint16_t ret = sps30_stop_measurement();
            if (ret) {
                Log.printf("Stopping measurement failed. Code: %d\n", ret);
            }
//            sps30_sleep();

            sps30_pm2_5->setAvailability(false);
            sps30_pm4->setAvailability(false);
            sps30_pm10->setAvailability(false);
            sps30_pm1->setAvailability(false);
            sps30_typical->setAvailability(false);
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
        Log.printf("Failed to connect to sps30. Attempt %d\n", i);
    }

    if (spsAvailable)
    {
        for (int i = 0; i < 5; ++i) {
            if (sps30_probe() == 0) {
                spsAvailable = true;
                break;
            }
            Log.printf("Failed to probe sps30. Attempt %d\n", i);
        }
    }

    if (spsAvailable) {
        Log.printf("sps available\n");
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
            Log.printf("error reading serial. Code: %d\n", ret);
        else
            Log.printf("SPS30 Serial: %s\n", serial);

        ret = sps30_set_fan_auto_cleaning_interval_days(AUTO_CLEAN_DAYS);
        if (ret)
            Log.printf("error %d setting the auto-clean interval\n", ret);

//        Serial.printf("sleeping..");
//        ret = sps30_start_measurement();
//        if (ret < 0) {
//            Serial.printf("error starting measurement\n");
//        }
        ret = sps30_stop_measurement();
        if (ret < 0) {
            Log.printf("error stopping measurement\n");
        }
//        sps30_sleep();
    }
}

void readIfPossibleSPS30(){

    if (sps30_sleep_switch->isOnline() && sps30_sleep_switch->getState())
    {
        struct sps30_measurement m;
        uint16_t ret = sps30_read_measurement(&m);
        if (ret < 0) {
            Log.printf("error reading measurement. Code: %d\n", ret);
        } else {
            if (SPS30_IS_ERR_STATE(ret)) {
                Log.printf(
                        "Chip state: %u - measurements may not be accurate\n",
                        SPS30_GET_ERR_STATE(ret));
            }
            sps30_pm1->setValue(m.mc_1p0);
            sps30_pm2_5->setValue(m.mc_2p5);
            sps30_pm4->setValue(m.mc_4p0);
            sps30_pm10->setValue(m.mc_10p0);
            sps30_typical->setValue(m.typical_particle_size);
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

void setupWenRadio() {

    //configure air filter radio
    wenAirFilter->setAvailability(true);
    wenAirFilter->setName("WEN Air Filter");
    //I tried to set this to 0, because a 0 speed makes sense to be "off". But the underlying calculation uses a log10() which returns -1... causing off by one errors in the json serialization... causing frequent diconnects from the mqtt server
    wenAirFilter->setSpeedRangeMin(1);
    wenAirFilter->setSpeedRangeMax(3);
    wenAirFilter->onSpeedChanged([](uint16_t speed) {
        switch(speed){
            default:
                wenAirFilter->setState(false);
                wenAirFilterDevice.setOffClear();
                break;
            case 1:
                wenAirFilter->setState(true);
                wenAirFilterDevice.setOnState(WenFilterSpeed::Low, WenFilterTime::None);
                break;
            case 2:
                wenAirFilter->setState(true);
                wenAirFilterDevice.setOnState(WenFilterSpeed::Medium, WenFilterTime::None);
                break;
            case 3:
                wenAirFilter->setState(true);
                wenAirFilterDevice.setOnState(WenFilterSpeed::High, WenFilterTime::None);
                break;
        }
    });

    wenAirFilter->onStateChanged([](bool state) {
        if (state) {
            if (wenAirFilter->getSpeed() == 0)
                wenAirFilter->setSpeed(1);
        } else {
            if (wenAirFilter->getSpeed() > 0)
                wenAirFilter->setSpeed(0);
        }
    });

    wenAirFilterDevice.begin();
}
void setup(){

    esp_task_wdt_init(WDT_TIMEOUT, true);
    _taskHandle = xTaskGetCurrentTaskHandle();
    esp_task_wdt_add(_taskHandle);

    Serial.begin(115200);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }
    WiFi.setAutoReconnect(true);

    //TODO: A webportal to configure wifi stuff would be awesome.
    //TODO: Save logs to internal memory for recovery
    Serial.println("Attempting Wifi connection");
    do{
        WiFi.begin(ssid, pwd);
    } while (WiFi.waitForConnectResult() != WL_CONNECTED);

//    auto address = MDNS.queryHost(graylog_host);
//    if (((uint32_t)address) == 0)
//    {
//        Serial.printf("Query for %s failed\n", graylog_host);
//    }
//    else
//    {
        auto gelfClient = new WiFiClient();
        auto gelfStream = new GelfUDPLogger(gelfClient);
        gelfStream->begin("192.168.4.177", deviceId);

        Log.addHandler(gelfStream);
//    }
    Log.addAdditionalField("Version", SOFTWARE_VERSION);
    Log.printf("wifi connected\n");
    setupOTA();

    device.enableSharedAvailability();
    device.setAvailability(true);

    device.setName("Workshop Controller");
    device.setSoftwareVersion("1.0.0");
    device.enableLastWill();

    uptime->setName("Workshop Controller Uptime");
    uptime->setDeviceClass("duration");

    setupWenRadio();

    configureDHT22();
    configureSPS30();

    //configure motion sensor
    motionSensor->setAvailability(true);
    motionSensor->setName("Workshop Motion Sensor");

    pinMode(motion_sensor_pin, INPUT);

    //TODO: configure current sensors
    current_sensor1->setName("Workshop Current Sensor 1");
    current_sensor1->setDeviceClass("current");
    current_sensor1->setUnitOfMeasurement("A");

    analogSetAttenuation(ADC_2_5db);
    adcAttachPin(current_sensor1_pin);
    int current1 = analogRead(current_sensor1_pin);
    current_sensor1->setValue(current1);

    current_sensor1->setAvailability(true);

    mqtt.begin(mqtt_host,1883);

    while (!mqtt.isConnected())
    {
        Log.printf("Mqtt not connected\n");
        mqtt.loop();
    }

    Log.printf("Mqtt connected\n");

    MDNS.begin(mDNSname);
}

unsigned long read_time_10s = 0;
unsigned long read_time_500ms = 0;
unsigned long read_time_2s = 0;
void loop(){

    esp_task_wdt_reset();

    mqtt.loop();
    ArduinoOTA.handle();

    if (!mqtt.isConnected())
    {
        Log.printf("Mqtt wasn't connected, restarting\n");
        esp_restart();
    }

    int motion = digitalRead(motion_sensor_pin);
    if (motion != motionSensor->getState())
    {
        motionSensor->setState(motion);
    }

    if (millis() - read_time_10s > 10000) {
        float humidity = dht22.readHumidity(false);
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

    if (millis() - read_time_2s > 2000) {
        uptime->setValue((uint32_t)millis()/1000.0);
        read_time_2s = millis();
    }
    if (millis() - read_time_10s > 10000) {
        readIfPossibleSPS30();
        read_time_10s = millis();
    }

    if (millis() - read_time_500ms > 500) {
        int current1 = analogRead(current_sensor1_pin);
        current_sensor1->setValue(current1);

        read_time_500ms = millis();
    }
}

