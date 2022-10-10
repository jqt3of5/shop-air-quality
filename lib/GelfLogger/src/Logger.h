//
// Created by jqt3o on 7/25/2022.
//

#ifndef SHOP_AIR_QUALITY_LOGGER_H
#define SHOP_AIR_QUALITY_LOGGER_H

#include <Print.h>
#include <tuple>

class Logger : public Print
{
public:

    virtual void logf(char* format, ...) = 0;
    virtual void addAdditionalField(const char * fieldName, const char * value);

//protected:
    int nfield(char * buffer, int maxLen, std::tuple<const char *, const char*> fields[], int count);
    size_t nsanitize(char * outBuffer, size_t maxSize, const uint8_t * inBuffer, size_t size);
    int _fieldCount = 0;
    int _maxfields = 0;
    std::tuple <const char *, const char*> * _additionalFields = nullptr;
};

#endif //SHOP_AIR_QUALITY_LOGGER_H
