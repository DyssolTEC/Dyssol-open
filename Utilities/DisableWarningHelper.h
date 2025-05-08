#pragma once

#if defined(_MSC_VER)
#include <CodeAnalysis/Warnings.h>
#define PRAGMA_WARNING_PUSH    __pragma(warning(push))
#define PRAGMA_WARNING_POP     __pragma(warning(pop))
#define PRAGMA_WARNING_DISABLE __pragma(warning(disable : 4005 4062 4244 4251 4267 4305 4309 4700 ALL_CODE_ANALYSIS_WARNINGS))
#elif defined(__clang__)
#define PRAGMA_WARNING_PUSH    _Pragma("clang diagnostic push")
#define PRAGMA_WARNING_POP     _Pragma("clang diagnostic pop")
#define PRAGMA_WARNING_DISABLE _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"")
#define PRAGMA_WARNING_DISABLE _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#else
#define PRAGMA_WARNING_PUSH
#define PRAGMA_WARNING_POP
#define PRAGMA_WARNING_DISABLE
#endif
