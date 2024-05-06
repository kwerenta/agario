cmake_minimum_required(VERSION 3.10)
project(AgarIO)

# Find SDL2 library
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
include_directories(${SDL2_INCLUDE_DIRS})

# Set sources for client, server, and shared files
file(GLOB CLIENT_SOURCES client/*.c)
file(GLOB SERVER_SOURCES server/*.c)
file(GLOB SHARED_SOURCES shared/*.c)

# Client executable
add_executable(client ${CLIENT_SOURCES} ${SHARED_SOURCES})
# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(client PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(client PRIVATE SDL2::SDL2)
# Server executable
add_executable(server ${SERVER_SOURCES} ${SHARED_SOURCES})


