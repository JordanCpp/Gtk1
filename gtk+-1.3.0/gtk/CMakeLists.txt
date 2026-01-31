cmake_minimum_required(VERSION 3.25)
project(gtk-lib C)

# 0. Поиск инструментов
find_program(PERL_EXECUTABLE perl PATHS "C:/Program Files/Git/usr/bin" REQUIRED)
find_program(AWK_EXECUTABLE awk PATHS "C:/Program Files/Git/usr/bin" REQUIRED)

set(GTK_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GTK_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# 1. Подготовка конфига
configure_file("${GTK_SRC_DIR}/../config.h.win32" "${GTK_SRC_DIR}/../config.h" COPYONLY)

# 2. Глобальные дефайны
add_definitions(-DHAVE_CONFIG_H -Dnear=xxnear -DGTK_DISABLE_COMPAT_H -DGTK_COMPILATION -DG_LOG_DOMAIN="Gtk")

# --- БЛОК ГЕНЕРАЦИИ ФАЙЛОВ ---

# Список заголовков (из Makefile.msc)
set(GTK_HDRS
    gtk.h gtkaccelgroup.h gtkaccellabel.h gtkadjustment.h gtkalignment.h gtkarg.h
    gtkarrow.h gtkaspectframe.h gtkbin.h gtkbindings.h gtkbbox.h gtkbox.h
    gtkbutton.h gtkcalendar.h gtkcheckbutton.h gtkcheckmenuitem.h gtkclist.h
    gtkcolorsel.h gtkcombo.h gtkcontainer.h gtkctree.h gtkcurve.h gtkdata.h
    gtkdialog.h gtkdnd.h gtkdrawingarea.h gtkeditable.h gtkentry.h gtkenums.h
    gtkeventbox.h gtkfilesel.h gtkfixed.h gtkfontsel.h gtkframe.h gtkgamma.h
    gtkgc.h gtkhandlebox.h gtkhbbox.h gtkhbox.h gtkhpaned.h gtkhruler.h
    gtkhscale.h gtkhscrollbar.h gtkhseparator.h gtkimage.h gtkinputdialog.h
    gtkinvisible.h gtkitem.h gtkitemfactory.h gtklabel.h gtklayout.h gtklist.h
    gtklistitem.h gtkmain.h gtkmenu.h gtkmenubar.h gtkmenufactory.h gtkmenuitem.h
    gtkmenushell.h gtkmisc.h gtknotebook.h gtkobject.h gtkoptionmenu.h gtkpacker.h
    gtkpaned.h gtkpixmap.h gtkplug.h gtkpreview.h gtkprogress.h gtkprogressbar.h
    gtkradiobutton.h gtkradiomenuitem.h gtkrange.h gtkrc.h gtkruler.h gtkscale.h
    gtkscrollbar.h gtkscrolledwindow.h gtkselection.h gtkseparator.h gtksignal.h
    gtksocket.h gtkspinbutton.h gtkstyle.h gtkstatusbar.h gtktable.h
    gtktearoffmenuitem.h gtktext.h gtkthemes.h gtktipsquery.h gtktogglebutton.h
    gtktoolbar.h gtktooltips.h gtktree.h gtktreeitem.h gtktypeutils.h
    gtkvbbox.h gtkvbox.h gtkviewport.h gtkvpaned.h gtkvruler.h gtkvscale.h
    gtkvscrollbar.h gtkvseparator.h gtkwidget.h gtkwindow.h
)

# Создаем заголовочные части файлов заранее, чтобы избежать проблем с кавычками в командах echo
file(WRITE "${GTK_BIN_DIR}/header_builtins.h" "#include \"gtktypebuiltins.h\"\n")
file(WRITE "${GTK_BIN_DIR}/header_gtk.h" "#include \"gtk.h\"\n")

# А. gtk.defs
add_custom_command(
    OUTPUT "${GTK_BIN_DIR}/gtk.defs"
    COMMAND ${PERL_EXECUTABLE} "${GTK_SRC_DIR}/makeenums.pl" defs ${GTK_HDRS} > "${GTK_BIN_DIR}/gtk.defs"
    WORKING_DIRECTORY "${GTK_SRC_DIR}"
    DEPENDS "${GTK_SRC_DIR}/makeenums.pl" ${GTK_HDRS}
)

# Б. gtktypebuiltins.h
add_custom_command(
    OUTPUT "${GTK_BIN_DIR}/gtktypebuiltins.h"
    COMMAND ${AWK_EXECUTABLE} -f "${GTK_SRC_DIR}/maketypes.awk" "${GTK_BIN_DIR}/gtk.defs" macros > "${GTK_BIN_DIR}/gtktypebuiltins.h"
    DEPENDS "${GTK_BIN_DIR}/gtk.defs" "${GTK_SRC_DIR}/maketypes.awk"
)

# В. gtktypebuiltins_vars.c (ТОЛЬКО ДАННЫЕ)
add_custom_command(
    OUTPUT "${GTK_BIN_DIR}/gtktypebuiltins_vars.c"
    COMMAND ${AWK_EXECUTABLE} -f "${GTK_SRC_DIR}/maketypes.awk" "${GTK_BIN_DIR}/gtk.defs" variables > "${GTK_BIN_DIR}/gtktypebuiltins_vars.c"
    DEPENDS "${GTK_BIN_DIR}/gtk.defs" "${GTK_SRC_DIR}/maketypes.awk"
)

# Д. gtktypebuiltins_ids.c (ТОЛЬКО ДАННЫЕ)
add_custom_command(
    OUTPUT "${GTK_BIN_DIR}/gtktypebuiltins_ids.c"
    COMMAND ${AWK_EXECUTABLE} -f "${GTK_SRC_DIR}/maketypes.awk" "${GTK_BIN_DIR}/gtk.defs" entries > "${GTK_BIN_DIR}/gtktypebuiltins_ids.c"
    DEPENDS "${GTK_BIN_DIR}/gtk.defs" "${GTK_SRC_DIR}/maketypes.awk"
)

# Г. gtktypebuiltins_evals.c (ТОЛЬКО ДАННЫЕ)
add_custom_command(
    OUTPUT "${GTK_BIN_DIR}/gtktypebuiltins_evals.c"
    COMMAND ${PERL_EXECUTABLE} "${GTK_SRC_DIR}/makeenums.pl" arrays ${GTK_HDRS} > "${GTK_BIN_DIR}/gtktypebuiltins_evals.c"
    WORKING_DIRECTORY "${GTK_SRC_DIR}"
    DEPENDS "${GTK_SRC_DIR}/makeenums.pl" ${GTK_HDRS}
)


# --- СБОРКА БИБЛИОТЕКИ ---

set(GTK_SOURCES
    fnmatch.c gtkaccelgroup.c gtkaccellabel.c gtkadjustment.c gtkalignment.c
    gtkarg.c gtkarrow.c gtkaspectframe.c gtkbbox.c gtkbin.c
    gtkbindings.c gtkbox.c gtkbutton.c gtkcalendar.c gtkcheckbutton.c
    gtkcheckmenuitem.c gtkclist.c gtkcolorsel.c gtkcombo.c gtkcontainer.c
    gtkctree.c gtkcurve.c gtkdata.c gtkdialog.c gtkdnd.c
    gtkdrawingarea.c gtkeditable.c gtkentry.c gtkeventbox.c gtkfilesel.c
    gtkfixed.c gtkfontsel.c gtkframe.c gtkgamma.c gtkgc.c
    gtkhandlebox.c gtkhbbox.c gtkhbox.c gtkhpaned.c gtkhruler.c
    gtkhscale.c gtkhscrollbar.c gtkhseparator.c gtkimage.c gtkinputdialog.c
    gtkinvisible.c gtkitem.c gtkitemfactory.c gtklabel.c gtklayout.c
    gtklist.c gtklistitem.c gtkmain.c gtkmenu.c gtkmenubar.c
    gtkmenufactory.c gtkmenuitem.c gtkmenushell.c gtkmisc.c gtknotebook.c
    gtkobject.c gtkoptionmenu.c gtkpacker.c gtkpaned.c gtkpixmap.c
    gtkplug.c gtkpreview.c gtkprogress.c gtkprogressbar.c gtkradiobutton.c
    gtkradiomenuitem.c gtkrange.c gtkrc.c gtkruler.c gtkscale.c
    gtkscrollbar.c gtkscrolledwindow.c gtkselection.c gtkseparator.c gtksignal.c
    gtksocket.c gtkspinbutton.c gtkstatusbar.c gtkstyle.c gtktable.c
    gtktearoffmenuitem.c gtktext.c gtkthemes.c gtktipsquery.c gtktogglebutton.c
    gtktoolbar.c gtktooltips.c gtktree.c gtktreeitem.c gtktypeutils.c
    gtkvbbox.c gtkvbox.c gtkviewport.c gtkvpaned.c gtkvruler.c
    gtkvscale.c gtkvscrollbar.c gtkvseparator.c gtkwidget.c gtkwindow.c

    gtkmarshal.c
    #"${GTK_BIN_DIR}/gtktypebuiltins_vars.c"
    #"${GTK_BIN_DIR}/gtktypebuiltins_ids.c"
    #"${GTK_BIN_DIR}/gtktypebuiltins_evals.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/../../glib-2.2.1/build/win32/dirent/dirent.c"
    "${GTK_BIN_DIR}/../../glib-2.2.1/intl_stub/intl_fix.c"
)

add_library(gtk-1.3 SHARED ${GTK_SOURCES} gtk.def)

target_include_directories(gtk-1.3 PRIVATE 
    "${GTK_SRC_DIR}" "${GTK_BIN_DIR}" "${GTK_SRC_DIR}/.." "${GTK_SRC_DIR}/../gdk"
    "${GTK_SRC_DIR}/../../glib-2.2.1"
    "${GTK_SRC_DIR}/../../glib-2.2.1/glib"
    "${GTK_SRC_DIR}/../../glib-2.2.1/gobject"
    "${GTK_SRC_DIR}/../../glib-2.2.1/gmodule"
    "${GTK_BIN_DIR}/../../glib-2.2.1/intl_stub"
    "${GTK_SRC_DIR}/../../glib-2.2.1/build/win32/dirent"
)

if(MSVC)
    file(TO_NATIVE_PATH "${GTK_SRC_DIR}/gtk.h" GTK_H_NATIVE)
    target_compile_options(gtk-1.3 PRIVATE "/FI${GTK_H_NATIVE}")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(gtk-1.3 PRIVATE gdk glib-2.0 gobject-2.0 gmodule-2.0 gdi32 user32 advapi32 imm32 shell32 ole32)
set_target_properties(gtk-1.3 PROPERTIES OUTPUT_NAME "libgtk-0" PREFIX "")
