#pragma once

#if defined(_MSC_VER)
#include <CodeAnalysis/Warnings.h>
#define PRAGMA_WARNING_PUSH    __pragma(warning(push))
#define PRAGMA_WARNING_POP     __pragma(warning(pop))
#define PRAGMA_WARNING_DISABLE __pragma(warning(disable : 4005 4244 4267 4700 ALL_CODE_ANALYSIS_WARNINGS))
#else
#define PRAGMA_WARNING_PUSH
#define PRAGMA_WARNING_POP
#define PRAGMA_WARNING_DISABLE
#endif
