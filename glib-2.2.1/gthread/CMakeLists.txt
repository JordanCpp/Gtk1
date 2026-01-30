cmake_minimum_required(VERSION 3.25)
project(gthread-lib C)

add_definitions(-DHAVE_CONFIG_H -DG_LOG_DOMAIN="GThread" -DG_THREADS_MANDATORY)

add_library(gthread-2.0 SHARED 
    "gthread-impl.c"
    "gthread.def"
)

target_include_directories(gthread-2.0 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../glib"
)

target_link_libraries(gthread-2.0 PRIVATE glib-2.0)

set_target_properties(gthread-2.0 PROPERTIES OUTPUT_NAME "libgthread-2.0-0" PREFIX "")
