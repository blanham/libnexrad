add_executable(test_util test_util.c)
target_link_libraries(test_util PRIVATE nexrad_static)
target_include_directories(test_util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME util_tests COMMAND test_util)
