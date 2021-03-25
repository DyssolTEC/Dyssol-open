/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <string>

//////////////////////////////////////////////////////////////////////////
/// Default and service values

namespace MDBDescriptors
{
	const std::string SIGNATURE_STRING = "DyssolMaterialsDatabase";
	const unsigned VERSION = 3;

	const std::wstring DEFAULT_MDB_FILE_NAME = L"Materials.dmdb";

	const double TEMP_MIN = 10;
	const double TEMP_MAX = 10000;
	const double PRES_MIN = 10e+2;
	const double PRES_MAX = 10e+8;

	enum class EPropertyType : unsigned
	{
		CONSTANT = 0,
		TP_DEPENDENT = 1,
		INTERACTION = 2
	};

	const size_t MAX_USER_DEFINED_PROP_NUMBER = 20; /// Number of properties of each type, which user is allowed to add (0, 50].

	const std::string NEW_LINE_REPLACER = "%@#";
}


//////////////////////////////////////////////////////////////////////////
/// Correlations

// Identifiers of correlation types
enum class ECorrelationTypes : unsigned
{
	LIST_OF_T_VALUES = 0,	// y = {T1:val1, T2:val2, T3:val3, ...}
	LIST_OF_P_VALUES = 1,	// y = {P1:val1, P2:val2, P3:val3, ...}
	CONSTANT		 = 2,	// y = a
	LINEAR           = 3,	// y = a·T + b·P + c
	EXPONENT_1       = 4,	// y = a·b^(c + d·T + (e·T + f) / (g·T + h)) + i
	POW_1            = 5,	// y = a·(T)^b
	POLYNOMIAL_1     = 6,	// y = a + b·T + c·T^2 + d·T^3 + e·T^4 + f·T^5 + g·T^6 + h·T^7
	POLYNOMIAL_CP    = 7,	// y = a + b·T + c·T^2 + d·T^3 + e/(T^2)
	POLYNOMIAL_H     = 8,	// y = a·T + b·(T^2)/2 + c·(T^3)/3 + d·(T^4)/4 − e/T + f − g
	POLYNOMIAL_S     = 9,	// y = a·ln(T) + b·T + c·(T^2)/2 + d·(T^3)/3 − e/(2·T^2) + f
};

namespace MDBDescriptors
{
	struct SCorrelationDescriptor
	{
		std::string name;
		std::wstring formula;
		size_t parametersNumber;
	};

	// List of descriptors of correlations
	static std::map<ECorrelationTypes, SCorrelationDescriptor> correlations
	{
		{ ECorrelationTypes::LIST_OF_T_VALUES ,	{ "List of T-values" ,			L"y = {T1:val1, T2:val2, T3:val3, ...}" ,																				0 } },
		{ ECorrelationTypes::LIST_OF_P_VALUES ,	{ "List of P-values" ,			L"y = {P1:val1, P2:val2, P3:val3, ...}" ,																				0 } },
		{ ECorrelationTypes::CONSTANT ,			{ "Constant" ,					L"y = a" ,																												1 } },
		{ ECorrelationTypes::LINEAR ,			{ "Linear" ,					L"y = aT + bP + c" ,																									3 } },
		{ ECorrelationTypes::EXPONENT_1 ,		{ "Exponential" ,				L"y = ab<sup>(c + dT + (eT + f) / (gT + h))</sup> + i" ,																9 } },
		{ ECorrelationTypes::POW_1 ,			{ "Power function" ,			L"y = aT<sup>b</sup>" ,																									2 } },
		{ ECorrelationTypes::POLYNOMIAL_1 ,		{ "Polynomial" ,				L"y = a + bT + cT<sup>2</sup> + dT<sup>3</sup> + eT<sup>4</sup> + fT<sup>5</sup> + gT<sup>6</sup> + hT<sup>7</sup>" ,	8 } },
		{ ECorrelationTypes::POLYNOMIAL_CP ,	{ "Shomate heat capacity" ,		L"y = a + bT + cT<sup>2</sup> + dT<sup>3</sup> + e/T<sup>2</sup>" ,														5 } },
		{ ECorrelationTypes::POLYNOMIAL_H ,		{ "Shomate standard enthalpy" ,	L"y = aT + bT<sup>2</sup>/2 + cT<sup>3</sup>/3 + dT<sup>4</sup>/4 - e/T + f - g" ,										7 } },
		{ ECorrelationTypes::POLYNOMIAL_S ,		{ "Shomate standard entropy" ,	L"y = a·ln(T) + bT + cT<sup>2</sup>/2 + dT<sup>3</sup>/3 - e/(2T<sup>2</sup>) + f" ,									6 } }
	};
}


//////////////////////////////////////////////////////////////////////////
/// Constant properties of pure compounds [100 .. 199]

// Identifiers of constant material properties
// TODO: make enum class
// TODO: rename enum
enum ECompoundConstProperties : unsigned
{
	CONST_PROP_NO_PROERTY                        = 100,
	CRITICAL_PRESSURE                            = 107,
	CRITICAL_TEMPERATURE                         = 108,
	HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT      = 114,
	HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT = 115,
	MOLAR_MASS                                   = 121,
	NORMAL_BOILING_POINT                         = 122,
	NORMAL_FREEZING_POINT                        = 123,
	//REACTIVITY_TYPE                              = 128,
	STANDARD_FORMATION_ENTHALPY                  = 131,
	BOND_WORK_INDEX								 = 132,
	SOA_AT_NORMAL_CONDITIONS                     = 141,
	CONST_PROP_USER_DEFINED_01                   = 150,
	CONST_PROP_USER_DEFINED_02                   = 151,
	CONST_PROP_USER_DEFINED_03                   = 152,
	CONST_PROP_USER_DEFINED_04                   = 153,
	CONST_PROP_USER_DEFINED_05                   = 154,
	CONST_PROP_USER_DEFINED_06                   = 155,
	CONST_PROP_USER_DEFINED_07                   = 156,
	CONST_PROP_USER_DEFINED_08                   = 157,
	CONST_PROP_USER_DEFINED_09                   = 158,
	CONST_PROP_USER_DEFINED_10                   = 159,
	CONST_PROP_USER_DEFINED_11                   = 160,
	CONST_PROP_USER_DEFINED_12                   = 161,
	CONST_PROP_USER_DEFINED_13                   = 162,
	CONST_PROP_USER_DEFINED_14                   = 163,
	CONST_PROP_USER_DEFINED_15                   = 164,
	CONST_PROP_USER_DEFINED_16                   = 165,
	CONST_PROP_USER_DEFINED_17                   = 166,
	CONST_PROP_USER_DEFINED_18                   = 167,
	CONST_PROP_USER_DEFINED_19                   = 168,
	CONST_PROP_USER_DEFINED_20                   = 169,
};

namespace MDBDescriptors
{
	// Base structure to describe initial values of all parameters
	struct SCompoundPropertyDescriptor
	{
		std::string name;
		std::wstring units;
		std::string description;
		SCompoundPropertyDescriptor() = default;
		SCompoundPropertyDescriptor(std::string _name, std::wstring _units, std::string _description)
			: name{std::move(_name)}, units{std::move(_units)}, description{std::move(_description)} {}
		virtual ~SCompoundPropertyDescriptor() = default;
		SCompoundPropertyDescriptor(const SCompoundPropertyDescriptor& _other) = default;
		SCompoundPropertyDescriptor(SCompoundPropertyDescriptor&& _other) = default;
		SCompoundPropertyDescriptor& operator=(const SCompoundPropertyDescriptor& _other) = default;
		SCompoundPropertyDescriptor& operator=(SCompoundPropertyDescriptor&& _other) = default;
	};

	// Initial values of all parameters of a CConstProperty
	struct SCompoundConstPropertyDescriptor : SCompoundPropertyDescriptor
	{
		double defaultValue{};
		SCompoundConstPropertyDescriptor() = default;
		SCompoundConstPropertyDescriptor(const std::string& _name, const std::wstring& _units, const std::string& _description, double _defaultValue)
			: SCompoundPropertyDescriptor{ _name, _units, _description }, defaultValue{ _defaultValue } {}
	};

	using constDescr = std::map<ECompoundConstProperties, SCompoundConstPropertyDescriptor>;

	// List of initial values of const properties
	static constDescr defaultConstProperties
	{
		{ CRITICAL_PRESSURE ,							 { "Critical pressure" ,	 L"Pa" ,	 ""                                                                                                     , 0 } },
		{ CRITICAL_TEMPERATURE ,						 { "Critical temperature" ,	 L"K" ,		 ""                                                                                                     , 0 } },
		{ HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT ,		 { "Heat of fusion" ,		 L"J/mol" ,	 "Heat of fusion at normal freezing point"                                                              , 0 } },
		{ HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT , { "Heat of vaporization " , L"J/mol" ,	 "Heat of vaporization at normal boiling point"                                                         , 0 } },
		{ MOLAR_MASS ,									 { "Molar mass" ,			 L"kg/mol" , ""                                                                                                     , 0 } },
		{ NORMAL_BOILING_POINT ,						 { "Normal boiling point" ,	 L"K" ,		 ""                                                                                                     , 0 } },
		{ NORMAL_FREEZING_POINT ,						 { "Normal freezing point" , L"K" ,		 ""                                                                                                     , 0 } },
		//{ REACTIVITY_TYPE ,								 { "Reactivity type" ,		 L"-"	,	 "Reactivity type\n[ 0 - reactive ], [ 1 - inert ], [ 2 - solvent ], [ 3 - insolvent ], [ 4 - solute ]" , 0 } },
		{ STANDARD_FORMATION_ENTHALPY ,					 { "Formation enthalpy" ,	 L"J/mol" ,	 "Standard enthalpy of formation"                                                                       , 0 } },
		{ BOND_WORK_INDEX,							     { "Bond work index" ,	     L"kWh/t" ,	 "Bond work index"																						, 0 } },
		{ SOA_AT_NORMAL_CONDITIONS ,					 { "State of aggregation" ,	 L"-" ,		 "State of aggregation at normal conditions\n[ 0 - solid ] [ 1 - liquid ] [ 2 - gas ] [ 3 - unknown ]"  , 3 } },
	};
}


//////////////////////////////////////////////////////////////////////////
/// Temperature/pressure-dependent properties of pure compounds [200 .. 299]

// Identifiers of temperature/pressure-dependent properties
// TODO: make enum class
// TODO: rename enum
enum ECompoundTPProperties : unsigned
{
	TP_PROP_NO_PROERTY      = 200,
	ENTHALPY                = 208,
	THERMAL_CONDUCTIVITY    = 222,
	VAPOR_PRESSURE          = 224,
	VISCOSITY               = 226,
	DENSITY                 = 234,
	PERMITTIVITY            = 235,
	TP_PROP_USER_DEFINED_01 = 250,
	TP_PROP_USER_DEFINED_02 = 251,
	TP_PROP_USER_DEFINED_03 = 252,
	TP_PROP_USER_DEFINED_04 = 253,
	TP_PROP_USER_DEFINED_05 = 254,
	TP_PROP_USER_DEFINED_06 = 255,
	TP_PROP_USER_DEFINED_07 = 256,
	TP_PROP_USER_DEFINED_08 = 257,
	TP_PROP_USER_DEFINED_09 = 258,
	TP_PROP_USER_DEFINED_10 = 259,
	TP_PROP_USER_DEFINED_11 = 260,
	TP_PROP_USER_DEFINED_12 = 261,
	TP_PROP_USER_DEFINED_13 = 262,
	TP_PROP_USER_DEFINED_14 = 263,
	TP_PROP_USER_DEFINED_15 = 264,
	TP_PROP_USER_DEFINED_16 = 265,
	TP_PROP_USER_DEFINED_17 = 266,
	TP_PROP_USER_DEFINED_18 = 267,
	TP_PROP_USER_DEFINED_19 = 268,
	TP_PROP_USER_DEFINED_20 = 269,
};

namespace MDBDescriptors
{
	// Initial values of all parameters of a CTPDProperty
	struct SCompoundTPDPropertyDescriptor : SCompoundPropertyDescriptor
	{
		ECorrelationTypes defuaultType{ ECorrelationTypes::CONSTANT };
		std::vector<double> defaultParameters{0.0};
		SCompoundTPDPropertyDescriptor() = default;
		SCompoundTPDPropertyDescriptor(const std::string& _name, const std::wstring& _units, const std::string& _description, ECorrelationTypes _defuaultType, std::vector<double> _defaultValue)
			: SCompoundPropertyDescriptor{ _name, _units, _description }, defuaultType{ _defuaultType }, defaultParameters{std::move(_defaultValue)} {}
	};

	using tpdepDescr = std::map<ECompoundTPProperties, MDBDescriptors::SCompoundTPDPropertyDescriptor>;

	// List of initial values of temperature/pressure - dependent properties
	static tpdepDescr defaultTPDProperties =
	{
		{ ENTHALPY ,			 { "Enthalpy" ,				L"J/kg" 			, "" , ECorrelationTypes::LINEAR   , { 4185.6, 0, 1e+6 } } },
		{ THERMAL_CONDUCTIVITY , { "Thermal conductivity" ,	L"W/(m*K)" 			, "" , ECorrelationTypes::CONSTANT , { 0.0 } } },
		{ VAPOR_PRESSURE ,		 { "Vapor pressure" ,		L"Pa" 				, "" , ECorrelationTypes::CONSTANT , { 0.0 } } },
		{ VISCOSITY ,			 { "Dynamic Viscosity" ,	L"Pa*s" 			, "" , ECorrelationTypes::CONSTANT , { 0.0 } } },
		{ DENSITY ,				 { "Density" ,				L"kg/m<sup>3</sup>" , "" , ECorrelationTypes::CONSTANT , { 0.0 } } },
		{ PERMITTIVITY ,		 { "Permittivity" ,			L"F/m" 				, "" , ECorrelationTypes::CONSTANT , { 0.0 } } },
	};
}


//////////////////////////////////////////////////////////////////////////
/// Interactions properties between two pure compounds [300 .. 399]

// Identifiers of properties, defined for interaction of two compounds
// TODO: make enum class
// TODO: rename enum
enum EInteractionProperties : unsigned
{
	INT_PROP_NO_PROERTY      = 300,
	INTERFACE_TENSION        = 303,
	INT_PROP_USER_DEFINED_01 = 350,
	INT_PROP_USER_DEFINED_02 = 351,
	INT_PROP_USER_DEFINED_03 = 352,
	INT_PROP_USER_DEFINED_04 = 353,
	INT_PROP_USER_DEFINED_05 = 354,
	INT_PROP_USER_DEFINED_06 = 355,
	INT_PROP_USER_DEFINED_07 = 356,
	INT_PROP_USER_DEFINED_08 = 357,
	INT_PROP_USER_DEFINED_09 = 358,
	INT_PROP_USER_DEFINED_10 = 359,
	INT_PROP_USER_DEFINED_11 = 360,
	INT_PROP_USER_DEFINED_12 = 361,
	INT_PROP_USER_DEFINED_13 = 362,
	INT_PROP_USER_DEFINED_14 = 363,
	INT_PROP_USER_DEFINED_15 = 364,
	INT_PROP_USER_DEFINED_16 = 365,
	INT_PROP_USER_DEFINED_17 = 366,
	INT_PROP_USER_DEFINED_18 = 367,
	INT_PROP_USER_DEFINED_19 = 368,
	INT_PROP_USER_DEFINED_20 = 369,
};

namespace MDBDescriptors
{
	using interDescr = std::map<EInteractionProperties, SCompoundTPDPropertyDescriptor>;

	// List of descriptors of temperature/pressure-dependent properties
	static interDescr defaultInteractionProperties
	{
		{ INTERFACE_TENSION , { "Interface tension" , L"N/m" , "" , ECorrelationTypes::CONSTANT , { 0 } } },
	};
}

namespace MDBDescriptors
{
	const unsigned FIRST_CONST_USER_PROP = CONST_PROP_USER_DEFINED_01;
	const unsigned FIRST_TPDEP_USER_PROP = TP_PROP_USER_DEFINED_01;
	const unsigned FIRST_INTER_USER_PROP = INT_PROP_USER_DEFINED_01;

	struct SPropertyDescriptor
	{
		EPropertyType type{ EPropertyType::CONSTANT };
		unsigned key{ FIRST_CONST_USER_PROP };
		double value{ 1.0 };
		std::string name;
		std::wstring units;
		std::string description;
	};
}
