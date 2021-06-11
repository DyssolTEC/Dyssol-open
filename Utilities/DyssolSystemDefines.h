/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BuildVersion.h"
#include "DyssolHelperDefines.h"

#define VERSION_0			0
#define VERSION_1			10
#define VERSION_2			2
#define VERSION_3			2
#define CURRENT_VERSION		VERSION_0,VERSION_1,VERSION_2,VERSION_3
#define CURRENT_VERSION_STR	MACRO_TOSTRING(VERSION_0) "." MACRO_TOSTRING(VERSION_1) "." MACRO_TOSTRING(VERSION_2) "." MACRO_TOSTRING(VERSION_3)
