/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#ifdef _MSC_VER
#include "BuildVersion.h"
#else
//TODO Define version
#define CURRENT_BUILD_VERSION 111
#define VISUAL_STUDIO_VERSION 111
#endif

#include "DyssolHelperDefines.h"

#define VERSION_0			0
#define VERSION_1			10
#define VERSION_2			2
#define VERSION_3			0
#define CURRENT_VERSION		VERSION_0,VERSION_1,VERSION_2,VERSION_3
#define CURRENT_VERSION_STR	MACRO_TOSTRING(VERSION_0) "." MACRO_TOSTRING(VERSION_1) "." MACRO_TOSTRING(VERSION_2) "." MACRO_TOSTRING(VERSION_3)
