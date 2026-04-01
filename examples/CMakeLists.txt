set(EXAMPLES
    display drawarc savepng proj
    showproj psychedelic level2_info proj_test
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.c)
    target_link_libraries(${example} PRIVATE nexrad)
endforeach()
