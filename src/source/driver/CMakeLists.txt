set(DRIVER_SOURCES
    impl/Nlohmann.cpp
)

set(DRIVER_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/impl/Nlohmann.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/JsonFactoryInterface.h
)

add_library(driver STATIC
    ${DRIVER_SOURCES}
    ${DRIVER_HEADERS}
)

target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
