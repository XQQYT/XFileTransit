set(DRIVER_SOURCES
    impl/UserJsonDriver.cpp
    impl/SignalJsonDriver.cpp
    impl/SettingsFile.cpp
    impl/TcpDriver.cpp
    impl/OuterMsgBuilder.cpp
    impl/OpensslDriver.cpp
    impl/OuterMsgParser.cpp
    impl/P2PDriver.cpp
    impl/WebSocket.cpp
    impl/FileSyncEngine/FileReceiver.cpp
    impl/FileSyncEngine/FileSender.cpp
    impl/FileSyncEngine/FileParser.cpp
    impl/FileSyncEngine/FileMsgBuilder.cpp
)

# Windows 特定的 OpenSSL 设置
if(WIN32)
    set(BOOST_INCLUDE_DIR "D:/msys64/mingw64/include")
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
else()
    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
        set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_CRYPTO_LIBRARIES})
        set(OPENSSL_SSL_LIBRARY ${OPENSSL_SSL_LIBRARIES})
    endif()
endif()

add_library(driver STATIC
    ${DRIVER_SOURCES}
)

target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${OPENSSL_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
)

if(WIN32)
    target_link_libraries(driver PUBLIC 
        "${OPENSSL_SSL_LIBRARY}"
        "${OPENSSL_CRYPTO_LIBRARY}"
        ws2_32      # Windows Socket
        crypt32     # Windows 加密 API
        bcrypt      # Windows 加密库
    )
else()
    target_link_libraries(driver PUBLIC 
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
    )
endif()

if(WIN32)
    target_compile_definitions(driver PRIVATE _WIN32)
else()
    target_compile_definitions(driver PRIVATE __linux__)
endif()