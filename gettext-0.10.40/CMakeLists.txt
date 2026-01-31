project(libintl C)

set(INTL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/intl")

configure_file("intl_config.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/config.h" COPYONLY)
configure_file("libintl.h.win32"     "${INTL_SRC_DIR}/libintl.h"            COPYONLY)

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
    "${INTL_SRC_DIR}/glib_ext.c")

add_library(intl STATIC ${INTL_SOURCES})

target_compile_definitions(intl PRIVATE HAVE_CONFIG_H=1 )

target_include_directories(intl PRIVATE 
    "${INTL_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libiconv-1.8-w32-1/include")