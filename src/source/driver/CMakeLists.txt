set(DRIVER_SOURCES
    impl/UserJsonDriver.cpp
    impl/SignalJsonDriver.cpp
    impl/SettingsFile.cpp
    impl/Network/TcpDriver.cpp
    impl/OuterMsgBuilder.cpp
    impl/OpensslDriver.cpp
    impl/OuterMsgParser.cpp
    impl/Network/P2PDriver.cpp
    impl/Network/WebSocket.cpp
    impl/FileSyncEngine/Tcp/TcpFileReceiver.cpp
    impl/FileSyncEngine/Tcp/TcpFileSender.cpp
    impl/FileSyncEngine/P2P/P2PFileSender.cpp
    impl/FileSyncEngine/FileParser.cpp
    impl/FileSyncEngine/FileMsgBuilder.cpp
)

# Windows 特定的 OpenSSL 设置
if(WIN32)
    if(NOT TOOLCHAIN_LOADED)
        message(WARNING "Toolchain not properly loaded!")
    endif()
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
    set(OPENSSL_LIBS_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MT")
    
    if(NOT EXISTS "${OPENSSL_LIBS_DIR}/libcrypto.lib")
        message(WARNING "OpenSSL crypto library not found at: ${OPENSSL_LIBS_DIR}/libcrypto.lib")
    endif()
    if(NOT EXISTS "${OPENSSL_LIBS_DIR}/libssl.lib")
        message(WARNING "OpenSSL ssl library not found at: ${OPENSSL_LIBS_DIR}/libssl.lib")
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

# 查找 libdatachannel
find_path(LibDataChannel_INCLUDE_DIR rtc/rtc.h
    PATHS ${LIBDATACHANNEL_ROOT}/include
          /usr/local/include
)

find_library(LibDataChannel_LIBRARY NAMES datachannel libdatachannel
    PATHS ${LIBDATACHANNEL_ROOT}/lib
          /usr/local/lib
)

if(LibDataChannel_INCLUDE_DIR AND LibDataChannel_LIBRARY)
    message(STATUS "Found libdatachannel: ${LibDataChannel_LIBRARY}")
    
    add_library(LibDataChannel::LibDataChannel UNKNOWN IMPORTED)
    set_target_properties(LibDataChannel::LibDataChannel PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LibDataChannel_INCLUDE_DIR}"
        IMPORTED_LOCATION "${LibDataChannel_LIBRARY}"
    )
    set(LibDataChannel_FOUND TRUE)
else()
    message(WARNING "libdatachannel not found, P2PDriver will not work")
endif()

if(LibDataChannel_FOUND)
    target_link_libraries(driver PUBLIC LibDataChannel::LibDataChannel)
endif()


target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${OPENSSL_INCLUDE_DIR}
    ${BOOST_ROOT}
)

if(WIN32)
    target_link_libraries(driver PUBLIC 
        "${OPENSSL_LIBS_DIR}/libssl.lib"
        "${OPENSSL_LIBS_DIR}/libcrypto.lib"
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