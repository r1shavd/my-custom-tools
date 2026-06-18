#ifndef COLORS_H
#define COLORS_H

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(_WIN32)
    #define DEFAULT "\033[0m"
    #define BLACK   "\033[0;90m"
    #define RED     "\033[0;91m"
    #define GREEN   "\033[0;92m"
    #define YELLOW  "\033[0;93m"
    #define BLUE    "\033[0;94m"
    #define MAGENTA "\033[0;95m"
    #define CYAN    "\033[0;96m"
    #define WHITE   "\033[0;97m"
#else
    #define DEFAULT ""
    #define BLACK   ""
    #define RED     ""
    #define GREEN   ""
    #define YELLOW  ""
    #define BLUE    ""
    #define MAGENTA ""
    #define CYAN    ""
    #define WHITE   ""
#endif

#endif
