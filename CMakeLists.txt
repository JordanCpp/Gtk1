cmake_minimum_required(VERSION 3.25)
project(Gtk1-Full-Project C)

if(MSVC)
    add_compile_options(/W3 /TC /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /wd4005 /wd4273)
    
    # Стаб для strings.h
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/stubs")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stubs/strings.h" "#include <string.h>\n")
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/stubs")
    
    add_definitions(-Dstrcasecmp=_stricmp -Dstrncasecmp=_strnicmp)

    add_definitions(-DGLIB_INTL_STATIC_BUILD) 
endif()

add_definitions(-DGETTEXT_PACKAGE="glib20")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/intl_stubs.c" [[
#include <stddef.h>
char *dgettext (const char *domainname, const char *msgid) { return (char *)msgid; }
char *bindtextdomain (const char *domainname, const char *dirname) { return (char *)dirname; }
char *bind_textdomain_codeset (const char *domainname, const char *codeset) { return (char *)codeset; }
]])

# Добавляем все подпроекты по очереди
add_subdirectory(libiconv-1.8-w32-1)
add_subdirectory(gettext-0.10.40)
add_subdirectory(glib-2.2.1)
add_subdirectory(glib-2.2.1/gobject)
add_subdirectory(glib-2.2.1/gmodule)
add_subdirectory(glib-2.2.1/gthread)

add_subdirectory(gtk+-1.3.0)

message(STATUS "Все модули Gtk1 сконфигурированы.")

function(add_gtk_example target_name source_file)
    # 1. Создаем исполняемый файл
    add_executable(${target_name} ${source_file})

    # 2. Подключаем инклуды
    target_include_directories(${target_name} PRIVATE 
        "gtk+-1.3.0"
        "gtk+-1.3.0/gtk"
        "gtk+-1.3.0/gdk"
        "glib-2.2.1"
        "glib-2.2.1/glib"
        "${CMAKE_CURRENT_BINARY_DIR}/stubs" # для strings.h
    )

    # 3. Линкуем библиотеки
    target_link_libraries(${target_name} PRIVATE 
        gtk-1.3 
        gdk 
        glib-2.0 
        gobject-2.0
        gmodule-2.0
        legacy_stdio_definitions.lib
    )

    # 4. Копируем DLL после сборки (Post-Build)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gtk-1.3> $<TARGET_FILE_DIR:${target_name}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gdk> $<TARGET_FILE_DIR:${target_name}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glib-2.0> $<TARGET_FILE_DIR:${target_name}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gobject-2.0> $<TARGET_FILE_DIR:${target_name}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gmodule-2.0> $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

# --- ТЕПЕРЬ ДОБАВЛЯТЬ ПРИМЕРЫ ОЧЕНЬ ПРОСТО ---

add_gtk_example(calendar "gtk+-1.3.0/examples/calendar/calendar.c")
add_gtk_example(buttons "gtk+-1.3.0/examples/buttons/buttons.c")
add_gtk_example(aspectframe "gtk+-1.3.0/examples/aspectframe/aspectframe.c")
add_gtk_example(base "gtk+-1.3.0/examples/base/base.c")
add_gtk_example(clist "gtk+-1.3.0/examples/clist/clist.c")
add_gtk_example(entry "gtk+-1.3.0/examples/entry/entry.c")
add_gtk_example(eventbox "gtk+-1.3.0/examples/eventbox/eventbox.c")
add_gtk_example(filesel "gtk+-1.3.0/examples/filesel/filesel.c")
add_gtk_example(fixed "gtk+-1.3.0/examples/fixed/fixed.c")
add_gtk_example(helloworld "gtk+-1.3.0/examples/helloworld/helloworld.c")
add_gtk_example(helloworld2 "gtk+-1.3.0/examples/helloworld2/helloworld2.c")
add_gtk_example(list "gtk+-1.3.0/examples/list/list.c")
add_gtk_example(menu "gtk+-1.3.0/examples/menu/menu.c" "gtk+-1.3.0/examples/menu/itemfactory.c")
add_gtk_example(notebook "gtk+-1.3.0/examples/notebook/notebook.c")
add_gtk_example(packbox "gtk+-1.3.0/examples/packbox/packbox.c")
add_gtk_example(packer "gtk+-1.3.0/examples/packer/pack.c")
add_gtk_example(paned "gtk+-1.3.0/examples/paned/paned.c")
add_gtk_example(pixmap "gtk+-1.3.0/examples/pixmap/pixmap.c")
add_gtk_example(progressbar "gtk+-1.3.0/examples/progressbar/progressbar.c")
add_gtk_example(radiobuttons "gtk+-1.3.0/examples/radiobuttons/radiobuttons.c")
add_gtk_example(rangewidgets "gtk+-1.3.0/examples/rangewidgets/rangewidgets.c")
add_gtk_example(rulers "gtk+-1.3.0/examples/rulers/rulers.c")
add_gtk_example(scribble-simple "gtk+-1.3.0/examples/scribble-simple/scribble-simple.c")
add_gtk_example(scrolledwin "gtk+-1.3.0/examples/scrolledwin/scrolledwin.c")
add_gtk_example(selection "gtk+-1.3.0/examples/selection/gettargets.c" "gtk+-1.3.0/examples/selection/setselection.c")
add_gtk_example(spinbutton "gtk+-1.3.0/examples/spinbutton/spinbutton.c")
add_gtk_example(statusbar "gtk+-1.3.0/examples/statusbar/statusbar.c")
add_gtk_example(table "gtk+-1.3.0/examples/table/table.c")
add_gtk_example(text "gtk+-1.3.0/examples/text/text.c")
add_gtk_example(tree "gtk+-1.3.0/examples/tree/tree.c")

add_gtk_example(testgtk "gtk+-1.3.0/gtk/testgtk.c")
