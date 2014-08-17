#pragma once

#if defined(__APPLE__)
    #include <TargetConditionals.h>
#endif

#if defined(__GNUC__)
    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 6
        #define BLACKHOLE_HAVE_AT_LEAST_GCC46
    #endif

    #if defined(__clang__)
        #define BLACKHOLE_HAVE_CLANG
    #endif
#endif
