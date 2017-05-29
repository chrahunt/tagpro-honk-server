cmake_minimum_required( VERSION 2.8.11 )
project( HonkServer )

set( CMAKE_CXX_STANDARD 11 )

include( dependencies/CMakeLists.txt )
add_subdirectory( src )
