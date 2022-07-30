//
// Created by jqt3o on 7/25/2022.
//

#include <cstdio>
#include <Logger.h>

int main()
{
    auto logger = new Logger();

    printf("adding fields\n");
    logger->addAdditionalField("fieldA", "abcdefgh");
    logger->addAdditionalField("fieldB", "abc");
    logger->addAdditionalField("fieldC", "abcd");
    logger->addAdditionalField("fieldD", "abcde");

    printf("field count: %d\n", logger->_fieldCount);
    printf("max fields : %d\n", logger->_maxfields);
     printf("%s", logger->getFieldString());
}

