cmake_minimum_required(VERSION 3.16)
project("shop-air-quality" C CXX)

# require C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(tests src)
add_library(logger src/Logger.cpp src/GLog.cpp)

add_executable(tests tests/tests.cpp)
target_link_libraries(tests PRIVATE logger)

include(CTest)
enable_testing()
add_test(NAME test1 COMMAND tests)