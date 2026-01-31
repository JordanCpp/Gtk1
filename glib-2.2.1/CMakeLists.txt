cmake_minimum_required(VERSION 3.25)
project(glib-lib C)

set(GLIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glib")
set(DIRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/win32/dirent")

# --- ГЕНЕРИРУЕМ ХЕДЕР-ПУСТЫШКУ И СТАБ ---
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/intl_stub")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/intl_stub/libintl.h" "
#ifndef _LIBINTL_H
#define _LIBINTL_H
#define gettext(S) (S)
#define dgettext(D,S) (S)
#define dcgettext(D,S,C) (S)
#define bindtextdomain(D,P) (P)
#define textdomain(D) (D)
#define bind_textdomain_codeset(D,C) (C)
#endif
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/intl_stub/intl_fix.c" "
#include <stddef.h>
char *dgettext (const char *d, const char *m) { return (char *)m; }
char *bindtextdomain (const char *d, const char *p) { return (char *)p; }
char *bind_textdomain_codeset (const char *d, const char *c) { return (char *)c; }
/* Имена с подчеркиванием для x86 линкера */
char *_dgettext (const char *d, const char *m) { return (char *)m; }
char *_bindtextdomain (const char *d, const char *p) { return (char *)p; }
char *_bind_textdomain_codeset (const char *d, const char *c) { return (char *)c; }
")

# Конфиги
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/config.h" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/glibconfig.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/glibconfig.h" COPYONLY)

set(GLIB_SOURCES
    "${GLIB_SRC_DIR}/garray.c" "${GLIB_SRC_DIR}/gasyncqueue.c" "${GLIB_SRC_DIR}/gbacktrace.c"
    "${GLIB_SRC_DIR}/gbsearcharray.c" "${GLIB_SRC_DIR}/gcache.c" "${GLIB_SRC_DIR}/gcompletion.c"
    "${GLIB_SRC_DIR}/gconvert.c" "${GLIB_SRC_DIR}/gdataset.c" "${GLIB_SRC_DIR}/gdate.c"
    "${GLIB_SRC_DIR}/gdir.c" "${GLIB_SRC_DIR}/gerror.c" "${GLIB_SRC_DIR}/gfileutils.c"
    "${GLIB_SRC_DIR}/ghash.c" "${GLIB_SRC_DIR}/ghook.c" "${GLIB_SRC_DIR}/giochannel.c"
    "${GLIB_SRC_DIR}/giowin32.c" "${GLIB_SRC_DIR}/glist.c" "${GLIB_SRC_DIR}/gmain.c"
    "${GLIB_SRC_DIR}/gmarkup.c" "${GLIB_SRC_DIR}/gmem.c" "${GLIB_SRC_DIR}/gmessages.c"
    "${GLIB_SRC_DIR}/gnode.c" "${GLIB_SRC_DIR}/gprimes.c" "${GLIB_SRC_DIR}/gqsort.c"
    "${GLIB_SRC_DIR}/gqueue.c" "${GLIB_SRC_DIR}/gpattern.c" "${GLIB_SRC_DIR}/grand.c"
    "${GLIB_SRC_DIR}/grel.c" "${GLIB_SRC_DIR}/gscanner.c" "${GLIB_SRC_DIR}/gshell.c"
    "${GLIB_SRC_DIR}/gslist.c" "${GLIB_SRC_DIR}/gspawn-win32.c" "${GLIB_SRC_DIR}/gstrfuncs.c"
    "${GLIB_SRC_DIR}/gstring.c" "${GLIB_SRC_DIR}/gthread.c" "${GLIB_SRC_DIR}/gthreadpool.c"
    "${GLIB_SRC_DIR}/gtimer.c" "${GLIB_SRC_DIR}/gtree.c" "${GLIB_SRC_DIR}/gunibreak.c"
    "${GLIB_SRC_DIR}/gunicollate.c" "${GLIB_SRC_DIR}/gunidecomp.c" "${GLIB_SRC_DIR}/guniprop.c"
    "${GLIB_SRC_DIR}/gutf8.c" "${GLIB_SRC_DIR}/gutils.c" "${GLIB_SRC_DIR}/gwin32.c"
    "${GLIB_SRC_DIR}/gprintf.c"
    "${GLIB_SRC_DIR}/trio/trio.c"
    "${GLIB_SRC_DIR}/trio/triostr.c"
    "${GLIB_SRC_DIR}/trio/trionan.c"
    "${GLIB_SRC_DIR}/libcharset/localcharset.c"
    "${DIRENT_DIR}/dirent.c"
    "${CMAKE_CURRENT_BINARY_DIR}/intl_stub/intl_fix.c"
)

add_library(glib-2.0 SHARED ${GLIB_SOURCES} "${GLIB_SRC_DIR}/glib.def")

target_compile_definitions(glib-2.0 PRIVATE HAVE_CONFIG_H GLIB_COMPILATION G_LOG_DOMAIN="GLib" HAVE_ICONV=1)

target_include_directories(glib-2.0 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${GLIB_SRC_DIR}" 
    "${GLIB_SRC_DIR}/trio"
    "${GLIB_SRC_DIR}/libcharset"
    "${DIRENT_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libiconv-1.8-w32-1/include"
    "${CMAKE_CURRENT_BINARY_DIR}/intl_stub" # ЧТОБЫ НАШЕЛ libintl.h
)

target_link_libraries(glib-2.0 PRIVATE iconv user32 advapi32 wsock32 ole32 shell32)
set_target_properties(glib-2.0 PROPERTIES OUTPUT_NAME "libglib-2.0-0" PREFIX "")
