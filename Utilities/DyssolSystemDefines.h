/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BuildVersion.h"
#include "DyssolHelperDefines.h"

#define VERSION_0			1
#define VERSION_1			3
#define VERSION_2			0
#define CURRENT_VERSION		VERSION_0,VERSION_1,VERSION_2
#define CURRENT_VERSION_STR	MACRO_STRINGIFY(VERSION_0) "." MACRO_STRINGIFY(VERSION_1) "." MACRO_STRINGIFY(VERSION_2)
