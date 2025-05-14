/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#if defined(_MSC_VER)
    #include <CodeAnalysis/Warnings.h>
    #define PRAGMA_WARNING_DISABLE  \
        __pragma(warning(push))     \
        __pragma(warning(disable : 4005 4062 4244 4251 4267 4305 4309 4700 ALL_CODE_ANALYSIS_WARNINGS))
    #define PRAGMA_WARNING_RESTORE __pragma(warning(pop))
#elif defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define PRAGMA_WARNING_DISABLE        \
        DO_PRAGMA(clang diagnostic push)  \
        DO_PRAGMA(clang diagnostic ignored "-Winconsistent-missing-override") \
        DO_PRAGMA(clang diagnostic ignored "-Wdeprecated-declarations")       \
        DO_PRAGMA(clang diagnostic ignored "-Wsign-conversion")               \
        DO_PRAGMA(clang diagnostic ignored "-Wfloat-equal")                   \
        DO_PRAGMA(clang diagnostic ignored "-Wmissing-noreturn")
    #define PRAGMA_WARNING_RESTORE DO_PRAGMA(clang diagnostic pop)
#elif defined(__GNUC__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define PRAGMA_WARNING_DISABLE        \
        DO_PRAGMA(GCC diagnostic push)    \
        DO_PRAGMA(GCC diagnostic ignored "-Wsign-conversion") \
        DO_PRAGMA(GCC diagnostic ignored "-Wmissing-field-initializers") \
        DO_PRAGMA(GCC diagnostic ignored "-Wmaybe-uninitialized")
    #define PRAGMA_WARNING_RESTORE DO_PRAGMA(GCC diagnostic pop)
#else
    #define PRAGMA_WARNING_DISABLE
    #define PRAGMA_WARNING_RESTORE
#endif
