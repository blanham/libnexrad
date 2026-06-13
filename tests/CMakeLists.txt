add_executable(test_util test_util.c)
target_link_libraries(test_util PRIVATE nexrad_static)
target_include_directories(test_util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME util_tests COMMAND test_util)

add_executable(test_projection test_projection.c)
target_link_libraries(test_projection PRIVATE nexrad_static)
target_include_directories(test_projection PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME projection_tests COMMAND test_projection)

add_executable(test_image test_image.c)
target_link_libraries(test_image PRIVATE nexrad_static)
target_include_directories(test_image PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME image_tests COMMAND test_image)

add_executable(test_level2 test_level2.c)
target_link_libraries(test_level2 PRIVATE nexrad_static)
target_include_directories(test_level2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME test_level2 COMMAND test_level2)
