/* Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

 /*
  * Safely includes <Windows.h> un-defining some Windows defines.
  */

#pragma once

#ifdef _WIN32
	#ifdef NOMINMAX
		#include <Windows.h>
		#undef AddPort
		#undef CreateFile
		#undef GetObject
	#else
		#define NOMINMAX
		#include <Windows.h>
		#undef NOMINMAX
		#undef AddPort
		#undef CreateFile
		#undef GetObject
	#endif
#endif