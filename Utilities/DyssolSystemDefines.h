/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BuildVersion.h"
#include "DyssolHelperDefines.h"

#define VERSION_0			1
#define VERSION_1			1
#define VERSION_2			0
#define CURRENT_VERSION		VERSION_0,VERSION_1,VERSION_2
#define CURRENT_VERSION_STR	MACRO_TOSTRING(VERSION_0) "." MACRO_TOSTRING(VERSION_1) "." MACRO_TOSTRING(VERSION_2)
