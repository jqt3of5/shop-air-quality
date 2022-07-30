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

    //TODO: Should update if field already exists, like a dictionary
    _additionalFields[_fieldCount] = new std::tuple<const char *, const char *>(fieldName, value);
    _fieldCount += 1;

}

char *Logger::getFieldString() {
    if (_fieldCount > 0)
    {
        if (_additionalFieldString != nullptr)
        {
            return _additionalFieldString;
        }
        char param[256] = {0};
        auto paramFormat = R"("_%s":"%s")";

        int maxLen = 256;
        int len = 0;
        char * params = new char[maxLen]();

        int l = snprintf(param, maxLen, paramFormat, std::get<0>(*_additionalFields[0]), std::get<1>(*_additionalFields[0]));
        if (l + len > maxLen)
        {
            maxLen = l + len + 1;
            auto p = new char[maxLen]();
            strncpy(p, params, l + len);
            delete[] params;
            params = p;
        }
        strncat(params, param, l);
        len = l + len;

        for (int i = 1; i < _fieldCount; ++i)
        {
            int l = snprintf(param, maxLen, paramFormat, std::get<0>(*_additionalFields[i]), std::get<1>(*_additionalFields[i]));
            if (l + len > maxLen)
            {
                maxLen = l + len + 1;
                auto p = new char[maxLen]();
                strncpy(p, params, l + len);
                delete[] params;
                params = p;
            }
            strncat(params, ", ", 2);
            strncat(params, param, l);
            len = l + len;
        }
        _additionalFieldString = params;
    }

    return _additionalFieldString;
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