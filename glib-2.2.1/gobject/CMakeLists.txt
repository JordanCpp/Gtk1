cmake_minimum_required(VERSION 3.25)
project(gobject-lib C)

set(GOBJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Список исходников GObject
set(GOBJECT_SOURCES
    "${GOBJECT_SRC_DIR}/gboxed.c"
    "${GOBJECT_SRC_DIR}/gclosure.c"
    "${GOBJECT_SRC_DIR}/genums.c"
    "${GOBJECT_SRC_DIR}/gobject.c"
    "${GOBJECT_SRC_DIR}/gparam.c"
    "${GOBJECT_SRC_DIR}/gparamspecs.c"
    "${GOBJECT_SRC_DIR}/gsignal.c"
    "${GOBJECT_SRC_DIR}/gsourceclosure.c"
    "${GOBJECT_SRC_DIR}/gtype.c"
    "${GOBJECT_SRC_DIR}/gtypemodule.c"
    "${GOBJECT_SRC_DIR}/gtypeplugin.c"
    "${GOBJECT_SRC_DIR}/gvalue.c"
    "${GOBJECT_SRC_DIR}/gvaluearray.c"
    "${GOBJECT_SRC_DIR}/gvaluetransform.c"
    "${GOBJECT_SRC_DIR}/gvaluetypes.c"
)

add_definitions(-DHAVE_CONFIG_H -DGOBJECT_COMPILATION -DG_LOG_DOMAIN="GObject")

# Сборка DLL. Используем gobject.def для экспорта
add_library(gobject-2.0 SHARED ${GOBJECT_SOURCES} "${GOBJECT_SRC_DIR}/gobject.def")

# Инклуды: нам нужны заголовки GLib (config.h и glib.h)
target_include_directories(gobject-2.0 PRIVATE 
    "${GOBJECT_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."       # Путь к config.h
    "${CMAKE_CURRENT_SOURCE_DIR}/../glib"  # Путь к glib.h
)

# Линковка с нашей свежесобранной glib-2.0
target_link_libraries(gobject-2.0 PRIVATE glib-2.0)

set_target_properties(gobject-2.0 PROPERTIES OUTPUT_NAME "libgobject-2.0-0" PREFIX "")
