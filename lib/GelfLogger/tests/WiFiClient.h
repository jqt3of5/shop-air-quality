//
// Created by jqt3o on 8/2/2022.
//

#ifndef SHOP_AIR_QUALITY_WIFICLIENT_H
#define SHOP_AIR_QUALITY_WIFICLIENT_H

#include <iostream>

struct IPAddress {

};
class WiFiClient: public Print {

public:
    bool _connected = false;
    void connect(const IPAddress & address, int port) {
       _connected = true;
    }
    void connect(const char * address, int port){
       _connected = true;
    }
    bool connected() {
        return _connected;
    }

    size_t write(uint8_t n) override {
        std::cout << n;
    }
    size_t write(const char *buffer, size_t size) {
        std::cout<<buffer;
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        std::cout<<buffer;
    }
    int availableForWrite() override {
       return 0;
    }

};
#endif //SHOP_AIR_QUALITY_WIFICLIENT_H
