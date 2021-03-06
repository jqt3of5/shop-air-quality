#ifndef GELFLOGGER_LIBRARY_H
#define GELFLOGGER_LIBRARY_H

#include <tuple>
#include <Print.h>
#include <HardwareSerial.h>
#include <WiFiClient.h>

class Logger : public Print
{
public:
    void addAdditionalField(const char * fieldName, const char * value);

protected:
    int _fieldCount = 0;
    int _maxfields = 0;
    std::tuple <const char *, const char*> ** _additionalFields = nullptr;
};

class GelfUDPLogger : public Logger
{
public:
    GelfUDPLogger(WiFiClient * client);
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;
//    void flush();
    void begin(const char * serverUrl, const char * host, int port = 122012, bool compress = true);

private:
    WiFiClient * _client;
    bool _compress;
    const char * _serverUrl;
    int _port;
    const char * _host;
};

class AggregateLogger : public Logger
{
public:
    AggregateLogger();
    void addHandler(Print * printer);

    size_t write(uint8_t n) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;

//    void flush() override;
private:
    int _maxHandlers;
    int _handlerCount;
    Print ** _handlers;
};

AggregateLogger Log;

#endif //GELFLOGGER_LIBRARY_H
