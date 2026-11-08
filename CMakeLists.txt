cmake_minimum_required(VERSION 3.21)
project(Ganj VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)   # Persian string literals in source files
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

file(GLOB_RECURSE GANJ_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)

set(GANJ_RESOURCES resources/resources.qrc)
if(WIN32)
    list(APPEND GANJ_RESOURCES resources/app.rc)
endif()

qt_add_executable(Ganj WIN32 ${GANJ_SOURCES} ${GANJ_RESOURCES})
target_include_directories(Ganj PRIVATE src)
target_link_libraries(Ganj PRIVATE Qt6::Widgets Qt6::Sql)
