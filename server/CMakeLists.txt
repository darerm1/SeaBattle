set(CORE_SOURCES
    src/core/game_field.cpp
    src/core/player.cpp
    src/core/queue.cpp
    src/core/session.cpp
    src/core/session_manager.cpp
    src/logger/logger.cpp
)

add_library(seabattle_core STATIC ${CORE_SOURCES})

target_include_directories(seabattle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(seabattle_core PUBLIC Threads::Threads)

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
