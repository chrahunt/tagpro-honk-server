find_package( Boost 1.58 COMPONENTS log REQUIRED )
find_package( OpenSSL 1.0.2 REQUIRED )
find_package( ZLIB 1.2.8 REQUIRED )

include_directories(
    # For uWebSockets
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${uWebSockets_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

add_definitions( -DBOOST_LOG_DYN_LINK )
add_executable( honkserver honkserver.cpp )
add_dependencies( honkserver uWebSockets )

target_link_libraries(
    honkserver LINK_PUBLIC
    # For uWebSockets
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${uWebSockets_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

install( PROGRAMS honkserver run DESTINATION bin )
