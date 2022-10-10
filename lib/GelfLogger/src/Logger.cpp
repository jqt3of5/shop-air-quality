//
// Created by jqt3o on 7/25/2022.
//

#include "Logger.h"
#include <cstring>
#include <algorithm>
#include <stdio.h>

void Logger::addAdditionalField(const char *fieldName, const char *value) {
    if (_additionalFields == nullptr)
    {
        _maxfields = 5;
        _additionalFields = new std::tuple <const char *, const char*>[_maxfields]();
    }

    if (_fieldCount >= _maxfields)
    {
        auto maxfields = _maxfields + 5;
        auto additionalFields = new std::tuple <const char *, const char*>[maxfields]();

        for (int i = 0; i < _fieldCount; ++i)
        {
            additionalFields[i] = _additionalFields[i];
        }

        delete _additionalFields;
        _additionalFields = additionalFields;
        _maxfields = maxfields;
    }

    //TODO: Should update if field already exists, like a dictionary
    _additionalFields[_fieldCount] = std::tuple<const char *, const char *>(fieldName, value);
    _fieldCount += 1;

}

int Logger::nfield(char * buffer, int maxLen, std::tuple<const char *, const char *> fields[], int count)
{
    //TODO: Underscore is an additional field thing
    auto firstParamFormat = R"("_%s":"%s")";
    auto paramFormat = R"(,"_%s":"%s")";

    int start = 0;
    for (int i = 0; i < count; ++i)
    {
        auto remaining = maxLen - start;
        if (remaining < 0)
            remaining = 0;

        if (i != 0) {
            start += snprintf(buffer + start, remaining, paramFormat, std::get<0>(fields[i]), std::get<1>(fields[i]));
        } else {
            start += snprintf(buffer + start, remaining, firstParamFormat, std::get<0>(fields[i]), std::get<1>(fields[i]));
        }
    }

    buffer[std::min(start, maxLen)] = 0;

    return start;
}

size_t Logger::nsanitize(char * outBuffer, size_t maxSize, const uint8_t * inBuffer, size_t size)
{
    int j = 0;
    for (int i = 0; i < size; ++i, ++j)
    {
        if (inBuffer[i] == '\0')
        {
            if (j < maxSize-2) {
                outBuffer[j] = '\\';
                outBuffer[j + 1] = '0';
            }
            j += 1;
        }
        else if (inBuffer[i] == '\n')
        {
            if (j < maxSize-2) {
                outBuffer[j] = '\\';
                outBuffer[j + 1] = 'n';
            }
            j += 1;
        }
        else if (inBuffer[i] == '\r')
        {
            if (j < maxSize-2) {
                outBuffer[j] = '\\';
                outBuffer[j + 1] = 'r';
            }
            j += 1;
        }
        else {
            outBuffer[j] = inBuffer[i];
        }
    }
    //Ensure last byte is null terminating
    outBuffer[std::min((int)maxSize-1, j)] = 0;
    return j;
}

