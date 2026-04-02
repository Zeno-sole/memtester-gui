cmake_minimum_required(VERSION 3.16)
project(memtester-gui VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    testworker.cpp
    ../tests.c
)

# Header files
set(HEADERS
    mainwindow.h
    testworker.h
    ../tests.h
    ../sizes.h
    ../types.h
)

# Platform-specific definitions
add_definitions(-DTEST_NARROW_WRITES)

# Create executable
add_executable(memtester-gui ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(memtester-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link libraries
target_link_libraries(memtester-gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(memtester-gui PRIVATE pthread)
endif()

# Set target properties
set_target_properties(memtester-gui PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

# Install
install(TARGETS memtester-gui
    RUNTIME DESTINATION bin
)
