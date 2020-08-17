/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#if   (_MSC_VER == 1600)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2010"
#elif (_MSC_VER == 1700)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2012"
#elif (_MSC_VER == 1800)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2013"
#elif (_MSC_VER == 1900)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2015"
#elif (_MSC_VER == 1910)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017"
#elif (_MSC_VER == 1911)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.4"
#elif (_MSC_VER == 1912)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.5"
#elif (_MSC_VER == 1913)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.6"
#elif (_MSC_VER == 1914)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.7"
#elif (_MSC_VER == 1915)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.8"
#elif (_MSC_VER == 1916)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.9"
#elif (_MSC_VER == 1920)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.0"
#elif (_MSC_VER == 1921)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.1"
#elif (_MSC_VER == 1922)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.2"
#elif (_MSC_VER == 1923)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.3"
#elif (_MSC_VER == 1924)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.4"
#elif (_MSC_VER == 1925)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.5"
#elif (_MSC_VER == 1926)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.6"
#elif (_MSC_VER == 1927)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.7"
#endif

#ifdef _WIN64
#define ARCHITECTURE "x64"
#else
#define ARCHITECTURE "x32"
#endif


#ifdef _MSC_VER
#define COMPILER_VERSION _MSC_VER
#else
#define COMPILER_VERSION MACRO_CONCAT(__GNUC__, __GNUC_MINOR__)
#endif

#define MACRO_CONCAT(X,Y) MACRO_CONCAT_L2(X,Y)
#define MACRO_CONCAT_L2(X,Y) X ## Y
#define MACRO_TOSTRING(X) MACRO_TOSTRING_L2(X)
#define MACRO_TOSTRING_L2(X) #X

#define TO_ARG_STR(X) std::string(MACRO_TOSTRING(X)).substr(12, std::string(MACRO_TOSTRING(X)).length()).append(12 + 20 - std::string(MACRO_TOSTRING(X)).length(), ' ')
