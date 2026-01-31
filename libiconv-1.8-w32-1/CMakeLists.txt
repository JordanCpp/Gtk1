cmake_minimum_required(VERSION 3.25)

project(iconv C)

set(ICONV_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

configure_file("${ICONV_DIR}/include/iconv.h.win32"              "${ICONV_DIR}/include/iconv.h" COPYONLY)
configure_file("${ICONV_DIR}/lib/config.h.win32"                 "${ICONV_DIR}/lib/config.h" COPYONLY)
configure_file("${ICONV_DIR}/libcharset/config.h.msvc"           "${ICONV_DIR}/libcharset/config.h" COPYONLY)
configure_file("${ICONV_DIR}/libcharset/include/libcharset.h.in" "${ICONV_DIR}/libcharset/include/libcharset.h" COPYONLY)

# 3. СБОРКА БИБЛИОТЕКИ
add_library(iconv STATIC 
    "${ICONV_DIR}/lib/iconv.c" 
    "${ICONV_DIR}/libcharset/lib/localcharset.c"
)

target_include_directories(iconv PUBLIC 
    "${ICONV_DIR}/lib" 
    "${ICONV_DIR}/include" 
    "${ICONV_DIR}/libcharset/include"
    "${ICONV_DIR}/libcharset/lib")

message(STATUS "Iconv CMake configured successfully.")
