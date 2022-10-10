#include <cstring>
#include "GLog.h"

AggregateLogger Log;

GelfUDPLogger::GelfUDPLogger(WiFiClient *client) {

    _client = client;
    _fieldCount = 0;
}

void GelfUDPLogger::begin(const char *address, const char *host, int port, bool compress) {
    _host = host;
    _compress = compress;

    _client->connect(address, port);
    if (!_client->connected())
    {
        Log.printf("Failed to connect to Gelf Endpoint");
    }
}

void GelfUDPLogger::begin(const IPAddress& address, const char * host, int port, bool compress) {
    _host = host;
    _compress = compress;

    _client->connect(address, port);
    if (!_client->connected())
    {
        Log.printf("Failed to connect to Gelf Endpoint");
    }
}

size_t GelfUDPLogger::write(uint8_t n) {
    return write(&n, 1);
}


size_t GelfUDPLogger::write(const uint8_t * message, size_t size) {

    if (!_client->connected())
    {
        return 0;
    }
    //TODO:
//    auto len = nsanitize(buffer, maxLen, message, size);

    int maxLen = 128;
    char m[128] = {0};
    char * buffer = m;

    auto len = serialize(buffer, maxLen, message, size);
    if (len + 1 > maxLen)
    {
        buffer = new char[len + 1]();
        serialize(buffer, len + 1, message, size);
    }

    _client->write(buffer, len+1);

    if (len + 1 > maxLen)
    {
        delete buffer;
    }

    return len;
}

void GelfUDPLogger::logf(char *format, ...) {
    if (!_client->connected())
    {
        return;
    }

    int maxMsgLen = 128;

    char msg[128] = {0};
    char * msgBuffer = msg;

    va_list args;
    va_start (args, format);
    auto msgLen = vsnprintf(msgBuffer, maxMsgLen, format, args);
    va_end (args);

    if (msgLen + 1 > maxMsgLen)
    {
       msgBuffer = new char[msgLen + 1]();
       maxMsgLen = msgLen + 1;
       va_start (args, format);
       msgLen = vsnprintf(msgBuffer, maxMsgLen, format, args);
       va_end (args);
    }

    int maxEventLen = 128;

    char event[128] = {0};
    char * eventBuffer = msg;
    //TODO: somehow aggregate the name and values for structured logging
    int eventLen = serialize(eventBuffer, maxEventLen, (const uint8_t*)msgBuffer, msgLen);

    //TODO:
//    auto len = nsanitize(buffer, maxLen, message, size);

    if (eventLen + 1 > maxEventLen)
    {
        eventBuffer = new char[eventLen + 1]();
        //TODO: somehow aggregate the name and values for structured logging
        eventLen = serialize(eventBuffer, maxEventLen, (const uint8_t*)msgBuffer, msgLen);
    }

    _client->write(eventBuffer, eventLen+1);

    if (eventLen + 1 > maxEventLen)
    {
        delete eventBuffer;
    }

//    return eventLen;
}

size_t GelfUDPLogger::serialize(char * buffer, size_t maxLen, const uint8_t * message, size_t size)
{
    return serialize(buffer, maxLen, message, size, {}, 0);
}

size_t GelfUDPLogger::serialize(char * buffer, size_t maxLen, const uint8_t * message, size_t size, std::tuple<const char*, const char*> fields[], int fieldCount)
{
    auto messageWithFieldsFormat = R"({ "version":"1.1", "host":"%s", "short_message":"%s", "level":"1")";
    auto endMessage = " }";

    size_t len = snprintf(buffer, maxLen, messageWithFieldsFormat, _host, message);
    auto total = len;
    auto remaining = maxLen < len ? 0 : maxLen - len;

    if (_fieldCount > 0)
    {
        strncat(buffer + total, ",", remaining);
        total += strlen(",");
    }
    len = nfield(buffer+ total, remaining, _additionalFields, _fieldCount);
    total += len;
    remaining = remaining < len ? 0 : remaining - len;

    len += nfield(buffer + total, maxLen, fields, fieldCount);
    total += len;
    maxLen = remaining < len ? 0 : remaining - len;

    strncat(buffer + total, endMessage, remaining);
    total += strlen(endMessage);

    buffer[std::min(total, maxLen)] = 0;
    return total;
}

int GelfUDPLogger::availableForWrite() {
    return _client->availableForWrite();
}



//void GelfUDPLogger::flush() {
//    Print::flush();
//}

AggregateLogger::AggregateLogger() : _maxHandlers(0), _handlerCount(0), _handlers(nullptr)
{

}

void AggregateLogger::logf(char *format, ...) {
    va_list args;
    va_start (args, format);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->logf(format, args);
    }
    va_end (args);
}

size_t AggregateLogger::write(uint8_t n)
{
//    Serial.write(n);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->write(n);
    }
    return 1;
}
size_t AggregateLogger::write(const uint8_t *buffer, size_t size)
{
//    Serial.write(buffer, size);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->write(buffer, size);
    }
   return size;
}
int AggregateLogger::availableForWrite()
{
    //TODO: Not sure what to do here...
    return 0;
}
//void AggregateLogger::flush()
//{
//    Serial.flush();
//    for (int i = 0; i < _handlerCount; ++i)
//    {
//        _handlers[i]->flush();
//    }
//}
void AggregateLogger::addAdditionalField(const char *fieldName, const char *value) {
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->addAdditionalField(fieldName, value);
    }
}
void AggregateLogger::addHandler(Logger * printer)
{
    if (_handlers == nullptr)
    {
        _maxHandlers = 5;
        _handlers = new Logger*[_maxHandlers];
    }

    if (_handlerCount >= _maxHandlers)
    {
        auto maxHandlers = _maxHandlers + 5;
        auto handlers = new Logger*[maxHandlers];

        for (int i = 0; i < _handlerCount; ++i)
        {
            handlers[i] = _handlers[i];
        }

        delete _handlers;
        _handlers = handlers;
        _maxHandlers = maxHandlers;
    }

    _handlers[_handlerCount] = printer;
    _handlerCount += 1;
}


