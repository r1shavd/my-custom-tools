/*
	COLORS

This is a custom header file generated for adding ANSII color themes to the
CLI based tools that we develop. These generally work for Linux/Unix/Darwin
kernel based platforms, but with modern support of Powershell rendering, these
color codes are also usable for windows CLI application.

The color codes are defined as a macros, and are cross platform.
The color codes are blank strings in case of unsupported platforms, thus
feel free to include them in your application without the worry.

steps:
 * copy this header file to include/ or inc/ or src/ folder of your project
 * add the line to the source file <file>.c
 	```
	#include "colors.h"
	```
 * use the compiler flags: gcc -I include/ -o <binary> <source-files>

 note:
 * Disable most of the macros in max cases as they will clutter
   micro space. Instead use just the standards + a few of your only needed bold, italics, etc.
 * If in case you need to use other configuration, then use a more efficient
   color based printing module.

Author: Rishav (github.com/r1shavd)
Creation date: Sep, 2020

*/

#ifndef COLORS_H
#define COLORS_H

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(_WIN32)
    // Default - resets all the ANSI color mods
	#define DEFAULT         "\033[0m"

    // Standard High-Intensity colors
    #define BLACK           "\033[0;90m"
    #define RED             "\033[0;91m"
    #define GREEN           "\033[0;92m"
    #define YELLOW          "\033[0;93m"
    #define BLUE            "\033[0;94m"
    #define MAGENTA         "\033[0;95m"
    #define CYAN            "\033[0;96m"
    #define WHITE           "\033[0;97m"

    // High-Intensity colors + Underline
    #define BLACK_UNDERLINE   "\033[4;90m"
    #define RED_UNDERLINE     "\033[4;91m"
    #define GREEN_UNDERLINE   "\033[4;92m"
    #define YELLOW_UNDERLINE  "\033[4;93m"
    #define BLUE_UNDERLINE    "\033[4;94m"
    #define MAGENTA_UNDERLINE "\033[4;95m"
    #define CYAN_UNDERLINE    "\033[4;96m"
    #define WHITE_UNDERLINE   "\033[4;97m"

	// High-Intensity colors + Bold
	#define BLACK_BOLD        "\033[1;90m"
    #define RED_BOLD          "\033[1;91m"
    #define GREEN_BOLD        "\033[1;92m"
    #define YELLOW_BOLD       "\033[1;93m"
    #define BLUE_BOLD         "\033[1;94m"
    #define MAGENTA_BOLD      "\033[1;95m"
    #define CYAN_BOLD         "\033[1;96m"
    #define WHITE_BOLD        "\033[1;97m"

	// High-Intensity colors + Italics
	#define BLACK_ITALIC      "\033[3;90m"
    #define RED_ITALIC        "\033[3;91m"
    #define GREEN_ITALIC      "\033[3;92m"
    #define YELLOW_ITALIC     "\033[3;93m"
    #define BLUE_ITALIC       "\033[3;94m"
    #define MAGENTA_ITALIC    "\033[3;95m"
    #define CYAN_ITALIC       "\033[3;96m"
    #define WHITE_ITALIC      "\033[3;97m"
#else
	// Default ANSI mods fallback
    #define DEFAULT         ""

    // Standard High-Intensity colors fallback
    #define BLACK           ""
    #define RED             ""
    #define GREEN           ""
    #define YELLOW          ""
    #define BLUE            ""
    #define MAGENTA         ""
    #define CYAN            ""
    #define WHITE           ""

    // Underline colors fallback
    #define BLACK_UNDERLINE   ""
    #define RED_UNDERLINE     ""
    #define GREEN_UNDERLINE   ""
    #define YELLOW_UNDERLINE  ""
    #define BLUE_UNDERLINE    ""
    #define MAGENTA_UNDERLINE ""
    #define CYAN_UNDERLINE    ""
    #define WHITE_UNDERLINE   ""

	// Bold colors fallback
    #define BLACK_BOLD        ""
    #define RED_BOLD          ""
    #define GREEN_BOLD        ""
    #define YELLOW_BOLD       ""
    #define BLUE_BOLD         ""
    #define MAGENTA_BOLD      ""
    #define CYAN_BOLD         ""
    #define WHITE_BOLD        ""

	// Italics colors fallback
	#define BLACK_ITALIC      ""
    #define RED_ITALIC        ""
    #define GREEN_ITALIC      ""
    #define YELLOW_ITALIC     ""
    #define BLUE_ITALIC       ""
    #define MAGENTA_ITALIC    ""
    #define CYAN_ITALIC       ""
    #define WHITE_ITALIC      ""
#endif

#endif
