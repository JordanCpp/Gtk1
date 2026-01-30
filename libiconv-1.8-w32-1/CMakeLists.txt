cmake_minimum_required(VERSION 3.25)
project(Gtk1-Iconv-Build C)

if(MSVC)
    add_compile_options(/W3 /TC /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /wd4005)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/stubs")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stubs/strings.h" "#include <string.h>\n")
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/stubs")
    add_definitions(-Dstrcasecmp=_stricmp -Dstrncasecmp=_strnicmp)
endif()

set(ICONV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")

# 1. ПОДГОТОВКА CONFIG.H (Включаем системные функции wchar)
file(READ "${ICONV_DIR}/lib/config.h.msvc" CONFIG_CONTENT)
string(REPLACE "#undef HAVE_MBRTOWC" "#define HAVE_MBRTOWC 1" CONFIG_FIXED "${CONFIG_CONTENT}")
string(REPLACE "#undef HAVE_WCRTOMB" "#define HAVE_WCRTOMB 1" CONFIG_FIXED "${CONFIG_FIXED}")
string(REPLACE "#undef HAVE_MBSINIT" "#define HAVE_MBSINIT 1" CONFIG_FIXED "${CONFIG_FIXED}")
file(WRITE "${ICONV_DIR}/lib/config.h" "${CONFIG_FIXED}")

# Остальные конфиги просто копируем
configure_file("${ICONV_DIR}/libcharset/config.h.msvc" "${ICONV_DIR}/libcharset/config.h" COPYONLY)
configure_file("${ICONV_DIR}/libcharset/include/libcharset.h.in" "${ICONV_DIR}/libcharset/include/libcharset.h" COPYONLY)

# 2. ГЕНЕРАЦИЯ ICONV.H
file(WRITE "${ICONV_DIR}/include/iconv.h" "
#ifndef _LIBICONV_H
#define _LIBICONV_H
#include <stddef.h>
#define LIBICONV_DLL_EXPORTED
#define ICONV_CONST const
typedef void* iconv_t;
extern size_t iconv (iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
extern iconv_t iconv_open (const char* tocode, const char* fromcode);
extern int iconv_close (iconv_t cd);
#define _LIBICONV_VERSION 0x0108
#endif
")

# 3. СБОРКА
add_library(iconv STATIC "${ICONV_DIR}/lib/iconv.c" "${ICONV_DIR}/libcharset/lib/localcharset.c")

target_compile_definitions(iconv PRIVATE 
    HAVE_CONFIG_H=1 BUILDING_LIBICONV=1
    _LIBICONV_VERSION=0x0108 ICONV_TRIVIALP=0 ICONV_GET_TRANSLITERATE=1
    ICONV_SET_TRANSLITERATE=2 ICONV_GET_DISCARD_ILSEQ=3 ICONV_SET_DISCARD_ILSEQ=4
)

target_include_directories(iconv PUBLIC 
    "${ICONV_DIR}/lib" "${ICONV_DIR}/include" "${ICONV_DIR}/libcharset/include"
    "${CMAKE_CURRENT_BINARY_DIR}/stubs"
)
