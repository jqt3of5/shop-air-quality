//
// Created by jqt3o on 7/25/2022.
//

#include <cstdio>
#include <Logger.h>
#include <GLog.h>

int main()
{
    WiFiClient client;
    auto logger = new GelfUDPLogger(&client);

    logger->addAdditionalField("fieldA", "abcdefgh");
    logger->addAdditionalField("fieldB", "abc");
    logger->addAdditionalField("fieldC", "abcd");
    logger->addAdditionalField("fieldD", "abcde");

    printf("field count: %d\n", logger->_fieldCount);
    printf("max fields : %d\n", logger->_maxfields);
    logger->begin("localhost", "test" );
    logger->printf("Test message");
//    printf("%s", logger->);
}

