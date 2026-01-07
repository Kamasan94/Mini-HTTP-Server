#include <string.h>
#ifndef __UTIL_H__
#define __UTIL_H__

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
};

#endif /* __UTIL_H__ */