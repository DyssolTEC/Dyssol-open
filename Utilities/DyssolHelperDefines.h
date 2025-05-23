/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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
#define VISUAL_STUDIO_VERSION	"Visual Studio 2017 15.3"
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
#elif (_MSC_VER == 1928)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.8-16.9"
#elif (_MSC_VER == 1929)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2019 16.10-16.11"
#elif (_MSC_VER == 1930)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.0"
#elif (_MSC_VER == 1931)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.1"
#elif (_MSC_VER == 1932)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.2"
#elif (_MSC_VER == 1933)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.3"
#elif (_MSC_VER == 1934)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.4"
#elif (_MSC_VER == 1935)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.5"
#elif (_MSC_VER == 1936)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.6"
#elif (_MSC_VER == 1937)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.7"
#elif (_MSC_VER == 1938)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.8"
#elif (_MSC_VER == 1939)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.9"
#elif (_MSC_VER == 1940)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.10"
#elif (_MSC_VER == 1941)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.11"
#elif (_MSC_VER == 1942)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.12"
#elif (_MSC_VER == 1943)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.13"
#elif (_MSC_VER == 1944)
#define VISUAL_STUDIO_VERSION	"Visual Studio 2022 17.14"
#else
#define VISUAL_STUDIO_VERSION	"Unknown"
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
#define MACRO_STRINGIFY(X) MACRO_STRINGIFY_L2(X)
#define MACRO_STRINGIFY_L2(X) #X

#define EXPAND(x) x
#define MACRO_CONCAT_STR(n1,n2) MACRO_STRINGIFY(EXPAND(n1)EXPAND(n2))
#define MACRO_CONCAT_STR2 MACRO_CONCAT_STR
#define MACRO_CONCAT_STR3(n1,n2,n3) MACRO_STRINGIFY(EXPAND(n1)EXPAND(n2)EXPAND(n3))
