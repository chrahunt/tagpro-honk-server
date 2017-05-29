include( ExternalProject )

# uWebSockets
find_package( OpenSSL 1.0.2 REQUIRED )
find_package( ZLIB 1.2.8 REQUIRED )

set( uWebSockets_ROOT ${CMAKE_SOURCE_DIR}/dependencies/uWebSockets )
ExternalProject_Add(
    uWebSockets

    GIT_REPOSITORY "https://github.com/uNetworking/uWebSockets.git"
    GIT_TAG "v0.14.3"

    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""

    SOURCE_DIR ${uWebSockets_ROOT}
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1

    INSTALL_COMMAND ""
    #TEST_COMMAND ""
)

set( uWebSockets_INCLUDE_DIR ${uWebSockets_ROOT}/src )
set( uWebSockets_LIBRARIES
    ${uWebSockets_ROOT}/${CMAKE_SHARED_LIBRARY_PREFIX}uWS${CMAKE_SHARED_LIBRARY_SUFFIX} )
install( PROGRAMS ${uWebSockets_LIBRARIES} DESTINATION lib )
