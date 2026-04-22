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

add_qt_test(tst_client_network  test_client_network.cpp)
add_qt_test(tst_field_widget    test_field_widget.cpp)
add_qt_test(tst_field_setting_page test_field_setting_page.cpp)
add_qt_test(tst_game_page       test_game_page.cpp)
