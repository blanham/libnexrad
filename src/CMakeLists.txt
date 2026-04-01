set(NEXRAD_SOURCES
    message.c chunk.c product.c symbology.c graphic.c
    tabular.c packet.c radial.c raster.c image.c
    color.c date.c error.c geo.c poly.c dvl.c
    eet.c util.c pnglite.c geodesic.c level2.c
)

add_library(nexrad SHARED ${NEXRAD_SOURCES})
add_library(nexrad_static STATIC ${NEXRAD_SOURCES})
set_target_properties(nexrad_static PROPERTIES OUTPUT_NAME nexrad)

add_library(nexrad::nexrad ALIAS nexrad)

target_include_directories(nexrad PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(nexrad_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(nexrad PRIVATE BZip2::BZip2 ZLIB::ZLIB ${MATH_LIBRARY})
target_link_libraries(nexrad_static PRIVATE BZip2::BZip2 ZLIB::ZLIB ${MATH_LIBRARY})

include(GNUInstallDirs)

install(TARGETS nexrad nexrad_static
    EXPORT nexradTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ../include/nexrad DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../include/nexrad/config.h 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nexrad)
