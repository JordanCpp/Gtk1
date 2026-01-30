cmake_minimum_required(VERSION 3.25)
project(gdk-win32 C)



# 1. Конфиги (исправляем пути)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../config.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/../config.h" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gdkconfig.h.win32" "${CMAKE_CURRENT_SOURCE_DIR}/gdkconfig.h" COPYONLY)

# 2. Дефайны
add_definitions(-DHAVE_CONFIG_H -DGDK_COMPILATION -DG_LOG_DOMAIN="Gdk" -DG_ENABLE_DEBUG)

add_definitions(
    -DX11_EXT_TABLET_SUPPORT=0 
    -DIGNORE_WINTAB 
    -DNO_WINTAB
)



# Обязательно добавляем путь к этой заглушке





add_compile_options(/D"HAVE_WINTAB=0")
# 3. Сборка БЭКЕНДА (win32/*.c)
file(GLOB GDK_WIN32_SOURCES "win32/*.c")
add_library(gdk_win32_backend STATIC ${GDK_WIN32_SOURCES})

# КРИТИЧНО: прописываем пути для файлов в папке win32
target_include_directories(gdk_win32_backend PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}"          # Чтобы видели gdkprivate.h, gdkcolor.h и т.д.
    "${CMAKE_CURRENT_SOURCE_DIR}/.."       # Чтобы видели config.h
    "${CMAKE_CURRENT_SOURCE_DIR}/win32"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../glib-2.2.1"       # Путь к glibconfig.h
    "${CMAKE_CURRENT_SOURCE_DIR}/../../glib-2.2.1/glib"  # Путь к glib.h
)

# 4. Сборка основной DLL
set(GDK_SOURCES
    gdk.c gdkcolor.c gdkcursor.c gdkdraw.c gdkevents.c
    gdkfont.c gdkgc.c gdkglobals.c gdkimage.c gdkrgb.c
    gdkrectangle.c gdkwindow.c
)

add_library(gdk SHARED ${GDK_SOURCES} gdk.def)

# Инклуды для основной DLL
target_include_directories(gdk PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../../glib-2.2.1"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../glib-2.2.1/glib"
)

# 5. Линковка
target_link_libraries(gdk PRIVATE 
    gdk_win32_backend
    glib-2.0
    gdi32 user32 imm32 shell32 ole32 uuid
)

set_target_properties(gdk PROPERTIES OUTPUT_NAME "libgdk-0" PREFIX "")
