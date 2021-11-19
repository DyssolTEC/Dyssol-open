/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "DyssolDefines.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>

/*
 * Helper functions for mapping enum values and their string representations.
 */
namespace
{
	/*
	 * Utility type.
	 * Every enum declares its own specialization of this template.
	 * Every enum value can have several string representations, the first one is used for export.
	 */
	template<typename T> struct SEnumStrings
	{
		static std::map<T, std::vector<std::string>> data;
	};

	template<> std::map<EConvergenceMethod, std::vector<std::string>>SEnumStrings<EConvergenceMethod>::data
	{
		{ EConvergenceMethod::DIRECT_SUBSTITUTION , { "DIRECT_SUBSTITUTION" } },
		{ EConvergenceMethod::WEGSTEIN            , { "WEGSTEIN"            } },
		{ EConvergenceMethod::STEFFENSEN          , { "STEFFENSEN"          } },
	};

	template<> std::map<EExtrapolationMethod, std::vector<std::string>>SEnumStrings<EExtrapolationMethod>::data
	{
		{ EExtrapolationMethod::LINEAR , { "LINEAR"           } },
		{ EExtrapolationMethod::SPLINE , { "CUBIC_SPLINE"	  } },
		{ EExtrapolationMethod::NEAREST, { "NEAREST_NEIGHBOR" } },
	};

	template<> std::map<EPhase, std::vector<std::string>>SEnumStrings<EPhase>::data
	{
		{ EPhase::SOLID , { "SOLID"        } },
		{ EPhase::LIQUID, { "LIQUID"       } },
		{ EPhase::VAPOR , { "GAS", "VAPOR" } },
	};

	template<> std::map<EDistrTypes, std::vector<std::string>>SEnumStrings<EDistrTypes>::data
	{
		{ EDistrTypes::DISTR_COMPOUNDS      , { "COMPOUNDS"	        } },
		{ EDistrTypes::DISTR_SIZE           , { "SIZE"	            } },
		{ EDistrTypes::DISTR_PART_POROSITY  , { "PARTICLE_POROSITY" } },
		{ EDistrTypes::DISTR_FORM_FACTOR    , { "FORM_FACTOR"	    } },
		{ EDistrTypes::DISTR_COLOR          , { "COLOR"	            } },
		{ EDistrTypes::DISTR_USER_DEFINED_01, { "USER_DEFINED_01"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_02, { "USER_DEFINED_02"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_03, { "USER_DEFINED_03"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_04, { "USER_DEFINED_04"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_05, { "USER_DEFINED_05"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_06, { "USER_DEFINED_06"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_07, { "USER_DEFINED_07"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_08, { "USER_DEFINED_08"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_09, { "USER_DEFINED_09"	} },
		{ EDistrTypes::DISTR_USER_DEFINED_10, { "USER_DEFINED_10"	} },
	};

	template<> std::map<EPSDTypes, std::vector<std::string>>SEnumStrings<EPSDTypes>::data
	{
		{ EPSDTypes::PSD_MassFrac, { "MASS_FRACTION" } },
		{ EPSDTypes::PSD_Number  , { "NUMBER"        } },
		{ EPSDTypes::PSD_q0      , { "Q0_DENSITY"    } },
		{ EPSDTypes::PSD_Q0      , { "Q0_CUMULATIVE" } },
		{ EPSDTypes::PSD_q2      , { "Q2_DENSITY"    } },
		{ EPSDTypes::PSD_Q2      , { "Q2_CUMULATIVE" } },
		{ EPSDTypes::PSD_q3      , { "Q3_DENSITY"    } },
		{ EPSDTypes::PSD_Q3      , { "Q3_CUMULATIVE" } },
	};

	template<> std::map<EPSDGridType, std::vector<std::string>>SEnumStrings<EPSDGridType>::data
	{
		{ EPSDGridType::DIAMETER, { "DIAMETER" } },
		{ EPSDGridType::VOLUME  , { "VOLUME"   } },
	};

	template<> std::map<EDistrFunction, std::vector<std::string>>SEnumStrings<EDistrFunction>::data
	{
		{ EDistrFunction::Normal   , { "NORMAL"     } },
		{ EDistrFunction::Manual   , { "MANUAL"     } },
		{ EDistrFunction::LogNormal, { "LOG_NORMAL" } },
		{ EDistrFunction::RRSB     , { "RRSB"       } },
		{ EDistrFunction::GGS      , { "GGS"        } },
	};

	template<> std::map<EGridEntry, std::vector<std::string>>SEnumStrings<EGridEntry>::data
	{
		{ EGridEntry::GRID_NUMERIC , { "NUMERIC"  } },
		{ EGridEntry::GRID_SYMBOLIC, { "SYMBOLIC" } },
	};

	template<> std::map<EGridFunction, std::vector<std::string>>SEnumStrings<EGridFunction>::data
	{
		{ EGridFunction::GRID_FUN_MANUAL         , { "MANUAL"          } },
		{ EGridFunction::GRID_FUN_EQUIDISTANT    , { "EQUIDISTANT"     } },
		{ EGridFunction::GRID_FUN_GEOMETRIC_S2L  , { "GEOMETRIC_INC"   } },
		{ EGridFunction::GRID_FUN_GEOMETRIC_L2S  , { "GEOMETRIC_DEC"   } },
		{ EGridFunction::GRID_FUN_LOGARITHMIC_S2L, { "LOGARITHMIC_INC" } },
		{ EGridFunction::GRID_FUN_LOGARITHMIC_L2S, { "LOGARITHMIC_DEC" } },
	};

	// Converts string to enum.
	template<typename T> T Name2Enum(const std::string& _s)
	{
		for (const auto& p : SEnumStrings<T>().data)
			if (std::find(p.second.begin(), p.second.end(), _s) != p.second.end())
				return p.first;
		return static_cast<T>(-1);
	}

	// Converts enum to string.
	template<typename T> std::string Enum2Name(T _e)
	{
		if (SEnumStrings<T>().data.find(_e) == SEnumStrings<T>().data.end()) return {};
		return SEnumStrings<T>().data[_e].front();
	}
}
