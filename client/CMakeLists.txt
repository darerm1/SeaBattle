find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CLIENT_SOURCES
    src/main.cpp
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

add_executable(seabattle_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_link_libraries(seabattle_client
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
)

target_include_directories(seabattle_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
