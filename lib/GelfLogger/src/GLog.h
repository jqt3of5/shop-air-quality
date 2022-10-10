#ifndef GELFLOGGER_LIBRARY_H
#define GELFLOGGER_LIBRARY_H

#include <tuple>
#include <Print.h>
#include <WiFiClient.h>
#include "Logger.h"

class GelfUDPLogger : public Logger
{
public:
    GelfUDPLogger(WiFiClient * client);
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;
    size_t serialize(char * buffer, size_t maxLen, const uint8_t * message, size_t size, std::tuple<const char*, const char*> fields[], int fieldCount);
    size_t serialize(char * buffer, size_t maxLen, const uint8_t * message, size_t size);
    void logf(char* format, ...);
//    void flush();
    void begin(const IPAddress& address, const char * host, int port = 12201, bool compress = true);
    void begin(const char* address, const char * host, int port = 12201, bool compress = true);

private:
    WiFiClient * _client;
    bool _compress;
    const char * _host;
};

class AggregateLogger : public Logger
{
public:
    AggregateLogger();
    void addHandler(Logger * printer);

    void logf(char *format, ...);
    size_t write(uint8_t n) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;

    void addAdditionalField(const char *fieldName, const char *value);
//    void flush() override;
private:
    int _maxHandlers;
    int _handlerCount;
    Logger ** _handlers;

};

extern AggregateLogger Log;

#endif //GELFLOGGER_LIBRARY_H
