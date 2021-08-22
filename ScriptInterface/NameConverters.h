/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * Helper functions to convert named values to their enumerations/keys.
 */

#pragma once
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <functional>
#include <iostream>

namespace ScriptInterface
{
	// Converts the name of a convergence method to its enumeration key.
	inline EConvergenceMethod ConvergenceName2Enum(const std::string& _s)
	{
		if (_s == "DIRECT_SUBSTITUTION")	return EConvergenceMethod::DIRECT_SUBSTITUTION;
		if (_s == "WEGSTEIN")				return EConvergenceMethod::WEGSTEIN;
		if (_s == "STEFFENSEN")				return EConvergenceMethod::STEFFENSEN;
		return static_cast<EConvergenceMethod>(-1);
	};

	// Converts the name of an extrapolation method to its enumeration key.
	inline EExtrapolationMethod ExtrapolationName2Enum(const std::string& _s)
	{
		if (_s == "LINEAR")				return EExtrapolationMethod::LINEAR;
		if (_s == "CUBIC_SPLINE")		return EExtrapolationMethod::SPLINE;
		if (_s == "NEAREST_NEIGHBOR")	return EExtrapolationMethod::NEAREST;
		return static_cast<EExtrapolationMethod>(-1);
	};

	// Converts the name of a phase to its enumeration key.
	inline EPhase PhaseName2Enum(const std::string& _s)
	{
		if (_s == "SOLID")					return EPhase::SOLID;
		if (_s == "LIQUID")					return EPhase::LIQUID;
		if (_s == "GAS" || _s == "VAPOR")	return EPhase::VAPOR;
		return static_cast<EPhase>(-1);
	}

	// Converts the name of a distribution type to its enumeration key.
	inline EDistrTypes DistributionName2Enum(const std::string& _s)
	{
		if (_s == "COMPOUNDS")			return EDistrTypes::DISTR_COMPOUNDS;
		if (_s == "SIZE")				return EDistrTypes::DISTR_SIZE;
		if (_s == "PARTICLE_POROSITY")	return EDistrTypes::DISTR_PART_POROSITY;
		if (_s == "FORM_FACTOR")		return EDistrTypes::DISTR_FORM_FACTOR;
		if (_s == "COLOR")				return EDistrTypes::DISTR_COLOR;
		if (_s == "USER_DEFINED_01")	return EDistrTypes::DISTR_USER_DEFINED_01;
		if (_s == "USER_DEFINED_02")	return EDistrTypes::DISTR_USER_DEFINED_02;
		if (_s == "USER_DEFINED_03")	return EDistrTypes::DISTR_USER_DEFINED_03;
		if (_s == "USER_DEFINED_04")	return EDistrTypes::DISTR_USER_DEFINED_04;
		if (_s == "USER_DEFINED_05")	return EDistrTypes::DISTR_USER_DEFINED_05;
		if (_s == "USER_DEFINED_06")	return EDistrTypes::DISTR_USER_DEFINED_06;
		if (_s == "USER_DEFINED_07")	return EDistrTypes::DISTR_USER_DEFINED_07;
		if (_s == "USER_DEFINED_08")	return EDistrTypes::DISTR_USER_DEFINED_08;
		if (_s == "USER_DEFINED_09")	return EDistrTypes::DISTR_USER_DEFINED_09;
		if (_s == "USER_DEFINED_10")	return EDistrTypes::DISTR_USER_DEFINED_10;
		return static_cast<EDistrTypes>(-1);
	}

	// Converts the name of a PSD type to its enumeration key.
	inline EPSDTypes PSDTypeName2Enum(const std::string& _s)
	{
		if (_s == "MASS_FRACTION")	return EPSDTypes::PSD_MassFrac;
		if (_s == "NUMBER")			return EPSDTypes::PSD_Number;
		if (_s == "Q0_DENSITY")		return EPSDTypes::PSD_q0;
		if (_s == "Q0_CUMULATIVE")	return EPSDTypes::PSD_Q0;
		if (_s == "Q2_DENSITY")		return EPSDTypes::PSD_q2;
		if (_s == "Q2_CUMULATIVE")	return EPSDTypes::PSD_Q2;
		if (_s == "Q3_DENSITY")		return EPSDTypes::PSD_q3;
		if (_s == "Q3_CUMULATIVE")	return EPSDTypes::PSD_Q3;
		return static_cast<EPSDTypes>(-1);
	}

	// Converts the name of a type of the PSD mean values to its enumeration key.
	inline EPSDGridType PSDMeanName2Enum(const std::string& _s)
	{
		if (_s == "DIAMETER")	return EPSDGridType::DIAMETER;
		if (_s == "VOLUME")		return EPSDGridType::VOLUME;
		return static_cast<EPSDGridType>(-1);
	}

	// Converts the name of a PSD distribution function to its enumeration key.
	inline EDistrFunction DistributionFunctionName2Enum(const std::string& _s)
	{
		if (_s == "MANUAL")		return EDistrFunction::Manual;
		if (_s == "NORMAL")		return EDistrFunction::Normal;
		if (_s == "LOG_NORMAL")	return EDistrFunction::LogNormal;
		if (_s == "RRSB")		return EDistrFunction::RRSB;
		if (_s == "GGS")		return EDistrFunction::GGS;
		return static_cast<EDistrFunction>(-1);
	}

	// Converts the name of a grid entry type to its enumeration key.
	inline EGridEntry GridEntryName2Enum(const std::string& _s)
	{
		if (_s == "NUMERIC")	return EGridEntry::GRID_NUMERIC;
		if (_s == "SYMBOLIC")	return EGridEntry::GRID_SYMBOLIC;
		return static_cast<EGridEntry>(-1);
	}

	// Converts the name of a grid function to its enumeration key.
	inline EGridFunction GridFunctionName2Enum(const std::string& _s)
	{
		if (_s == "MANUAL")				return EGridFunction::GRID_FUN_MANUAL;
		if (_s == "EQUIDISTANT")		return EGridFunction::GRID_FUN_EQUIDISTANT;
		if (_s == "GEOMETRIC_INC")		return EGridFunction::GRID_FUN_GEOMETRIC_S2L;
		if (_s == "GEOMETRIC_DEC")		return EGridFunction::GRID_FUN_GEOMETRIC_L2S;
		if (_s == "LOGARITHMIC_INC")	return EGridFunction::GRID_FUN_LOGARITHMIC_S2L;
		if (_s == "LOGARITHMIC_DEC")	return EGridFunction::GRID_FUN_LOGARITHMIC_L2S;
		return static_cast<EGridFunction>(-1);
	}

	// Applies one of the above functions to convert the name to the enumeration key.
	template<typename T>
	SNameOrKey Convert(const SNameOrKey& _entry, const std::function<T(const std::string&)>& _func)
	{
		SNameOrKey res = _entry;
		if (!res.HasKey())
		{
			res.key = E2I<T>(_func(StringFunctions::ToUpperCase(_entry.name)));
			if (res.key == -1)
				std::cout << StrConst::DyssolC_WarningUnknown(_entry.name) << std::endl;
		}
		return res;
	}
}
