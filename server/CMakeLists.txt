find_package(Boost REQUIRED COMPONENTS thread)
find_package(Threads REQUIRED)
find_library(SQLITE3_LIBRARY sqlite3 REQUIRED)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h REQUIRED)

set(CORE_SOURCES
    src/core/game_field.cpp
    src/core/player.cpp
    src/core/queue.cpp
    src/core/session.cpp
    src/core/session_manager.cpp
    src/logger/logger.cpp
)

set(NETWORK_SOURCES
    src/network/connection.cpp
    src/network/network_manager.cpp
)

set(UTILS_SOURCES
    src/database/database_manager.cpp
    src/auth/auth_manager.cpp
    src/logger/logger.cpp
    src/utils/thread_pool.cpp
)

set(COMMAND_SOURCES
    src/network/commands/base_command.cpp
    src/network/commands/login_command.cpp
    src/network/commands/sign_up_command.cpp
    src/network/commands/disconnect_command.cpp
    src/network/commands/clear_field_command.cpp
    src/network/commands/move_command.cpp
    src/network/commands/place_ship_command.cpp
    src/network/commands/play_command.cpp
    src/network/commands/ready_command.cpp
)

add_library(seabattle_server_lib STATIC 
    ${CORE_SOURCES} 
    ${NETWORK_SOURCES} 
    ${COMMAND_SOURCES} 
    ${UTILS_SOURCES}
)

target_include_directories(seabattle_server_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(seabattle_server_lib PUBLIC
    Boost::headers
    Boost::thread
    Threads::Threads
    ${SQLITE3_LIBRARY}
)

add_executable(run_server main.cpp)

target_link_libraries(run_server PRIVATE seabattle_server_lib)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(googletest)

add_executable(run_server_tests
    tests/unit/test_game_field.cpp
    tests/unit/test_session.cpp
    tests/unit/test_queue.cpp
    tests/integration/test_session_manager.cpp
    tests/integration/test_database.cpp 
    tests/network/test_protocol.cpp 
    tests/network/test_commands.cpp 
)

target_link_libraries(run_server_tests
    PRIVATE
    seabattle_server_lib
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_server_tests)
