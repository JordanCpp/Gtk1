# Gtk1-Win32 (MSVC 2022)

The legendary GTK+ 1.3 resurrected for modern Windows.

This repository contains a working port of GTK+ 1.3 compiled with MSVC 2022 and CMake. It proves that 25-year-old native C code can outperform modern UI frameworks by orders of magnitude while running flawlessly on Windows 11.

## ❓ Why GTK 1.3?

Modern GTK versions (3 and 4) have moved towards CSS-based styling, complex scene graphs, and heavy abstractions. While flexible, this came at a cost. GTK 1.3 represents a "Golden Era" of UI development:

*   **Pixel-Perfect Native Feel**: It doesn't emulate widgets; it builds them from the ground up using fundamental GDK primitives.
*   **No CSS Overhead**: Layouts are calculated using fast, imperative C code, not by parsing stylesheets at runtime.
*   **Simple Object Model**: The early GObject implementation is lightweight and doesn't require the massive boilerplate of modern versions.
*   **Pure Win32 GDK**: This port uses the native GDK Win32 backend, which means it speaks the language of the Windows OS without unnecessary translation layers.


## 🛠 Build Instructions
1. Install **Perl** (e.g., Strawberry Perl) and ensure it's in your PATH.
2. Open the folder in **Visual Studio 2022** (via CMake support) or run:
   ```bash
   cmake -B build
   cmake --build build --config Release
   ```

## 🚀 Performance & Efficiency
This isn't just a nostalgic experiment; it's a showcase of rational software engineering:
*   Memory Footprint: ~1.7 MB (Working Set) for a standard window.
*   CPU Usage: 0% at idle.
*   Startup: Instantaneous.
*   Architecture: Direct Win32 API calls with a dedicated HWND for every widget.

## 🛠 Tech Specs
*   Compiler: Microsoft Visual C++ 2022 (v143)
*   Build System: Modern CMake
*   OS Compatibility: Tested on Windows 11 (x64), theoretically compatible back to Windows 98/NT.
*   Dependencies: Includes glib, iconv, and gdk backends.

## 💡 The Manifesto of Rational Software
In an era where a simple calculator consumes 150MB of RAM, this project serves as a reminder:
1. Bloat is not inevitable.
2. Native code is immortal.
3. Efficiency is a feature, not an afterthought.

## 🚧 Status & Limitations
- [x] Full build with MSVC 2022.
- [x] All standard examples (helloworld, testgtk) are working.
- [x] Minimal dependency tree.
- [ ] Unicode: Limited (Standard GTK 1.x ANSI legacy).
- [ ] Warnings: Expect a "waterfall" of compiler warnings due to C89/Modern MSVC type mismatches (to be refactored).

## 📸 Screenshots

| | | |
|:---:|:---:|:---:|
| ![](screenshots/calendar.jpg) | ![](screenshots/entry.jpg) | ![](screenshots/list.jpg) |
| ![](screenshots/progressbar.jpg) | ![](screenshots/radiobuttons.jpg) | ![](screenshots/range.jpg) |
| ![](screenshots/rulers.jpg) | ![](screenshots/text.jpg) | ![](screenshots/tree.jpg) |

## ⚖️ License
Licensed under LGPL v2.0 (as per original GTK 1.3 source).
