cmake_minimum_required(VERSION 3.20)
project(SearchEngine LANGUAGES CXX)


if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)         
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libpqxx CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale)

add_library(common
    common/Config.cpp
    common/Database.cpp
    common/TextUtils.cpp)
target_link_libraries(common PUBLIC
    libpqxx::pqxx
    Boost::locale)

add_executable(indexer indexer/main.cpp)
target_link_libraries(indexer PRIVATE common)

add_executable(searcher searcher/main.cpp)
target_link_libraries(searcher PRIVATE common)
