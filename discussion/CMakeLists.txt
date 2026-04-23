if(NOT DEFINED OJ_SUBPROJECT_BUILD)
    set(OJ_SUBPROJECT_BUILD OFF)
endif()

if(NOT OJ_SUBPROJECT_BUILD)
    cmake_minimum_required(VERSION 3.14)
    project(OJ_Discussion LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(Threads REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/oj_common_standalone)
endif()

find_package(Threads REQUIRED)

add_library(discussion_core
    src/discussion_service.cpp
    src/discussion_handler.cpp
)

target_include_directories(discussion_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../judge/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../judge/include/asio
)

target_link_libraries(discussion_core PUBLIC oj_common Threads::Threads)

if(WIN32)
    # Crow requires Winsock libraries on Windows.
    target_link_libraries(discussion_core PUBLIC ws2_32 mswsock)
endif()

add_executable(discussion_server src/main.cpp)
target_link_libraries(discussion_server PRIVATE discussion_core)

enable_testing()

add_executable(test_discussion_service tests/test_discussion_service.cpp)
target_link_libraries(test_discussion_service PRIVATE discussion_core)

add_test(NAME DiscussionServiceTest COMMAND test_discussion_service)
