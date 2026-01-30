cmake_minimum_required(VERSION 3.25)
project(libintl C)
add_definitions(-Dlibintl_gettext=gettext)
add_definitions(-Dlibintl_dgettext=dgettext)
add_definitions(-Dlibintl_bindtextdomain=bindtextdomain)
# 1. Настройки компилятора
if(MSVC)
    add_compile_options(/W1 /TC /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4013)
endif()

set(INTL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/intl")

# 2. Генерируем libintl.h (чтобы GLib видел прототипы функций)
file(WRITE "${INTL_SRC_DIR}/libintl.h" "
#ifndef _LIBINTL_H
#define _LIBINTL_H

#ifdef __cplusplus
extern \"C\" {
#endif

char *gettext(const char *msgid);
char *dgettext(const char *domainname, const char *msgid);
char *dcgettext(const char *domainname, const char *msgid, int category);
char *textdomain(const char *domainname);
char *bindtextdomain(const char *domainname, const char *dirname);
char *bind_textdomain_codeset(const char *domainname, const char *codeset);

/* Фикс для GLib: мапим имена, если линкер их не видит */
#define _dgettext dgettext
#define _bindtextdomain bindtextdomain

#ifdef __cplusplus
}
#endif

#endif
")

# 3. Генерируем glib_ext.c для недостающей функции bind_textdomain_codeset
file(WRITE "${INTL_SRC_DIR}/glib_ext.c" "
#include <stddef.h>
char *bind_textdomain_codeset (const char *domainname, const char *codeset) { 
    return (char *)codeset; 
}
")

# 4. Генерируем минимальный config.h
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/config.h" "
#define HAVE_STRCHR 1
#define HAVE_MEMCPY 1
#define HAVE_LOCALE_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_ICONV 1
#define ENABLE_NLS 1
#define HAVE_BIND_TEXTDOMAIN_CODESET 1
")

# 5. Исходники библиотеки intl
set(INTL_SOURCES
    "${INTL_SRC_DIR}/bindtextdom.c"
    "${INTL_SRC_DIR}/dcgettext.c"
    "${INTL_SRC_DIR}/dgettext.c"
    "${INTL_SRC_DIR}/gettext.c"
    "${INTL_SRC_DIR}/finddomain.c"
    "${INTL_SRC_DIR}/loadmsgcat.c"
    "${INTL_SRC_DIR}/localealias.c"
    "${INTL_SRC_DIR}/textdomain.c"
    "${INTL_SRC_DIR}/l10nflist.c"
    "${INTL_SRC_DIR}/explodename.c"
    "${INTL_SRC_DIR}/dcigettext.c"
    "${INTL_SRC_DIR}/glib_ext.c"
)

# 6. Сборка статической библиотеки
add_library(intl STATIC ${INTL_SOURCES})

target_compile_definitions(intl PRIVATE 
    HAVE_CONFIG_H 
    BUILDING_LIBINTL 
    IN_LIBINTL
    LOCALEDIR=\"./locale\"
)

# Пути поиска инклудов
target_include_directories(intl PUBLIC 
    "${INTL_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libiconv-1.8-w32-1/include"
)
