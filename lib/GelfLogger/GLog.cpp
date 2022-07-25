#include "GLog.h"


void Logger::addAdditionalField(const char *fieldName, const char *value) {
    if (_additionalFields == nullptr)
    {
        _maxfields = 5;
        _additionalFields = new std::tuple<const char *, const char*>*[_maxfields]();
    }

    if (_fieldCount >= _maxfields)
    {
        auto maxfields = _maxfields + 5;
        auto additionalFields = new std::tuple<const char *, const char*>*[_maxfields]();

        for (int i = 0; i < _fieldCount; ++i)
        {
            additionalFields[i] = _additionalFields[i];
        }

        delete _additionalFields;
        _additionalFields = additionalFields;
        _maxfields = maxfields;
    }

    //TODO: Should update if field already exists
    _additionalFields[_fieldCount] = new std::tuple<const char *, const char *>(fieldName, value);

}

GelfUDPLogger::GelfUDPLogger(const char * serverUrl, const char * host, int port, bool compress) {
    _serverUrl = serverUrl;
    _host = host;
    _port = port;
    _compress = compress;

    _fieldCount = 0;
}

void GelfUDPLogger::begin() {
    //TODO: Connect to the Gelf server

}

size_t GelfUDPLogger::write(uint8_t) {
    return nullptr;
}

size_t GelfUDPLogger::write(const uint8_t *buffer, size_t size) {
    return Print::write(buffer, size);
}

int GelfUDPLogger::availableForWrite() {
    return Print::availableForWrite();
}

void GelfUDPLogger::flush() {
    Print::flush();
}

AggregateLogger::AggregateLogger() : _handlerCount(0), _handlers(nullptr), _maxHandlers(0)
{
}

size_t AggregateLogger::write(uint8_t n)
{
    Serial.write(n);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->write(n);
    }
}
size_t AggregateLogger::write(const uint8_t *buffer, size_t size)
{
    Serial.write(buffer, size);
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->write(buffer, size);
    }
}
int AggregateLogger::availableForWrite()
{

}
void AggregateLogger::flush()
{
    Serial.flush();
    for (int i = 0; i < _handlerCount; ++i)
    {
        _handlers[i]->flush();
    }
}

void AggregateLogger::addHandler(Print * printer)
{
    if (_handlers == NULL)
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
}


