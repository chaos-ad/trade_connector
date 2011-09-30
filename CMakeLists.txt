cmake_minimum_required(VERSION 2.6)
project(trade_connector)

find_package(Boost 1.38 REQUIRED system thread)

add_executable(trade_connector
    transaq_client.cpp
    transaq_wrapper.cpp
    transaq_client.cpp
    transaq_loader_stub.cpp
    main.cpp)

target_link_libraries(trade_connector ${Boost_LIBRARIES})

