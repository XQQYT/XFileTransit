set(DRIVER_SOURCES
    impl/Nlohmann.cpp
    impl/TcpDriver.cpp
    impl/OuterMsgBuilder.cpp
    impl/OpensslDriver.cpp
    impl/OuterMsgParser.cpp
    impl/FileSyncEngine/FileReceiver.cpp
    impl/FileSyncEngine/FileSender.cpp
    impl/FileSyncEngine/FileParser.cpp
    impl/FileSyncEngine/FileMsgBuilder.cpp
)

set(DRIVER_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/impl/Nlohmann.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/JsonFactoryInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/OuterMsgBuilderInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/NetworkInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/SecurityInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/FileSyncEngine/FileMsgBuilderInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/FileSyncEngine/FileParserInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/FileSyncEngine/FileReceiverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/driver/interface/FileSyncEngine/FileSenderInterface.h
)

set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT/libssl.lib")

if(NOT EXISTS "${OPENSSL_CRYPTO_LIBRARY}")
    message(WARNING "OpenSSL crypto library not found at: ${OPENSSL_CRYPTO_LIBRARY}")
endif()
if(NOT EXISTS "${OPENSSL_SSL_LIBRARY}")
    message(WARNING "OpenSSL ssl library not found at: ${OPENSSL_SSL_LIBRARY}")
endif()

add_library(driver STATIC
    ${DRIVER_SOURCES}
    ${DRIVER_HEADERS}
)

target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(driver PUBLIC 
    "${OPENSSL_SSL_LIBRARY}"
    "${OPENSSL_CRYPTO_LIBRARY}"
    ws2_32
    crypt32
    bcrypt
)