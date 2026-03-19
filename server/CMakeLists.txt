set(CORE_SOURCES
    src/core/game_field.cpp
    src/core/player.cpp
    src/core/queue.cpp
    src/core/session.cpp
    src/core/session_manager.cpp
    src/logger/logger.cpp
)
find_package(Boost REQUIRED COMPONENTS system thread)

add_library(seabattle_core STATIC ${CORE_SOURCES})

target_include_directories(seabattle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(seabattle_core PUBLIC 
    Boost::thread 
    Boost::system
    Threads::Threads
)

target_compile_definitions(seabattle_core PUBLIC BOOST_THREAD_VERSION=4)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(googletest)

add_executable(run_server_tests
    tests/test_game_field.cpp
    tests/test_session.cpp
    tests/test_queue.cpp
    tests/test_session_manager.cpp
)

target_link_libraries(run_server_tests
    seabattle_core
    gtest_main
)

target_include_directories(run_server_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_test(NAME ServerTests COMMAND run_server_tests)

#find_package(Boost REQUIRED COMPONENTS system thread)
find_package(SQLite3 REQUIRED)

set(NETWORK_SOURCES
    src/network/connection.cpp
    src/network/network_manager.cpp
    src/database/database_manager.cpp
    src/utils/thread_pool.cpp
    src/auth/auth_manager.cpp
)

add_library(seabattle_network STATIC ${NETWORK_SOURCES})
#target_compile_definitions(seabattle_network PRIVATE BOOST_THREAD_VERSION=4)
target_include_directories(seabattle_network PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(seabattle_network
    seabattle_core
    Boost::thread
    Boost::system
    SQLite::SQLite3
    Threads::Threads
)

add_executable(seabattle_server src/main.cpp)
target_link_libraries(seabattle_server seabattle_network)
target_include_directories(seabattle_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
