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


size_t GelfUDPLogger::write(const uint8_t *buffer, size_t size) {

    int len = 0;
    char m[64] = {0};
    char * message = m;

    len = nsanitize(message, sizeof(m), buffer, size);
    if (len + 1 > 64)
    {
        message = new char[len+1];
        nsanitize(message, len+1, buffer, size);
    }

    char t[64] = {0};
    char * temp = t;

    if (_fieldCount > 0) {
        auto messageWithFieldsFormat = R"({ "version":"1.1", "host":"%s", "short_message":"%s", "level":"1", %s })";
        auto fields = getFieldString();
        len = snprintf(t, sizeof(t), messageWithFieldsFormat, _host, message, fields);

        if (len+2 > 64)
        {
            temp = new char[len+2]();
            snprintf(temp, len+2, messageWithFieldsFormat, _host, message, fields);
        }
        Serial.printf("%s", temp);
    } else {
        auto messageWithoutFieldsFormat = R"({ "version":"1.1", "host":"%s", "short_message":"%s", "level":"1" })";
        len = snprintf(t, sizeof(t), messageWithoutFieldsFormat, _host, message);

        if (len+2 > 64)
        {
            temp = new char[len+2]();
            snprintf(temp, len+2, messageWithoutFieldsFormat, _host, message);
        }
        Serial.printf("%s", temp);
    }

    _client->write(temp, len+2);

    if (len +2 > 64)
    {
        delete temp;
    }
    return len + 2;
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

size_t AggregateLogger::write(uint8_t n)
{
    Serial.write(n);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->write(n);
    }
    return 1;
}
size_t AggregateLogger::write(const uint8_t *buffer, size_t size)
{
    Serial.write(buffer, size);
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

void AggregateLogger::addHandler(Print * printer)
{
    if (_handlers == nullptr)
    {
        _maxHandlers = 5;
        _handlers = new Print*[_maxHandlers];
    }

    if (_handlerCount >= _maxHandlers)
    {
        auto maxHandlers = _maxHandlers + 5;
        auto handlers = new Print*[_maxHandlers];

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


