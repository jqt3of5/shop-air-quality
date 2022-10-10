//
// Created by jqt3o on 7/25/2022.
//

#ifndef SHOP_AIR_QUALITY_PRINT_H
#define SHOP_AIR_QUALITY_PRINT_H

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
class Print {
public:
    virtual size_t write(uint8_t n) {
       printf("%d", n);
       return 0;
    }
    virtual size_t write(const uint8_t *buffer, size_t size) {
        printf("%s", buffer);
        return 0;
    }
    virtual int availableForWrite() {
        return 0;
    }

    void printf(const char * format, ...)
    {
        char buffer [512] = {0};
        va_list args;
        va_start (args, format);
        vsprintf(buffer, format, args);
        va_end (args);
        write(reinterpret_cast<const uint8_t *>(buffer), strlen(buffer));
    }
};
#endif //SHOP_AIR_QUALITY_PRINT_H
