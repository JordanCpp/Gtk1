cmake_minimum_required(VERSION 3.25)
project(gmodule-lib C)

configure_file("gmoduleconf.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/gmoduleconf.h" COPYONLY)

add_definitions(-DHAVE_CONFIG_H -DG_LOG_DOMAIN="GModule")

# В Windows используется реализация gmodule-win32.c
add_library(gmodule-2.0 SHARED 
    "gmodule.c"
    "gmodule.def"
)

target_include_directories(gmodule-2.0 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../glib"
)

target_link_libraries(gmodule-2.0 PRIVATE glib-2.0)

set_target_properties(gmodule-2.0 PROPERTIES OUTPUT_NAME "libgmodule-2.0-0" PREFIX "")
