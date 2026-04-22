find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CLIENT_LIB_SOURCES
    src/ui/main_window.cpp
    src/network/client_network.cpp
    src/ui/field_setting_page.cpp
    src/ui/field_widget.cpp
    src/ui/game_page.cpp
    src/ui/login_page.cpp
)

set(CLIENT_HEADERS
    include/ui/main_window.hpp
    include/network/client_network.hpp
    include/ui/field_setting_page.hpp
    include/ui/field_widget.hpp
    include/ui/game_page.hpp
    include/ui/login_page.hpp
)

add_library(seabattle_client_lib STATIC ${CLIENT_LIB_SOURCES} ${CLIENT_HEADERS})
target_include_directories(seabattle_client_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(seabattle_client_lib PUBLIC
    Qt6::Core Qt6::Network Qt6::Widgets
)

add_executable(seabattle_client src/main.cpp)
target_link_libraries(seabattle_client PRIVATE seabattle_client_lib)

find_package(Qt6 REQUIRED COMPONENTS Test Widgets Network)

function(add_qt_test target source)
    add_executable(${target} ${source})
    set_target_properties(${target} PROPERTIES AUTOMOC ON)
    target_link_libraries(${target} PRIVATE
        seabattle_client_lib
        Qt6::Test
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_qt_test(tst_client_network  tests/test_client_network.cpp)
add_qt_test(tst_field_widget    tests/test_field_widget.cpp)
add_qt_test(tst_field_setting_page tests/test_field_setting_page.cpp)
add_qt_test(tst_game_page       tests/test_game_page.cpp)

