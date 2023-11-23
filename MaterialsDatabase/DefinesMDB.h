/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

 /**
 \file DefinesMDB.h
 \details This definition is needed for Doxygen to properly generate documentation.
 */

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
	const std::string SIGNATURE_STRING = "DyssolMaterialsDatabase"; ///< Signature string to recognize materials database file.
	const unsigned VERSION = 3;										///< Version of the materials database file.

	const std::string DEFAULT_MDB_FILE_NAME = "Materials.dmdb";		///< Default name of the materials database file.

	const double TEMP_MIN = 10;										///< Minimum temperature.
	const double TEMP_MAX = 10000;									///< Maximum temperature.
	const double PRES_MIN = 10e+2;									///< Minimum pressure.
	const double PRES_MAX = 10e+8;									///< Maximum pressure.

	/**
	 * \brief Type of the material property.
	 */
	enum class EPropertyType : unsigned
	{
		CONSTANT = 0,
		TP_DEPENDENT = 1,
		INTERACTION = 2
	};

	const size_t MAX_USER_DEFINED_PROP_NUMBER = 20; ///< Number of properties of each type, which user is allowed to add (0, 50].

	const std::string NEW_LINE_REPLACER = "%@#";    ///< Symbols to replace the new line symbol.
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
	SUTHERLAND		 = 10,  // y = a·(b + c)/(T + c)·(T/b)^(3/2)
};

namespace MDBDescriptors
{
	/**
	 * \private
	 * \brief Description of the correlation property.
	 */
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
		{ ECorrelationTypes::POLYNOMIAL_S ,		{ "Shomate standard entropy" ,	L"y = a·ln(T) + bT + cT<sup>2</sup>/2 + dT<sup>3</sup>/3 - e/(2T<sup>2</sup>) + f" ,									6 } },
		{ ECorrelationTypes::SUTHERLAND,		{ "Sutherland's law"          ,	L"y = y = a·(b + c)/(T + c)·(T/b)<sup>3/2</sup>"                                                                    ,   3 } }
	};
}


//////////////////////////////////////////////////////////////////////////
// Constant properties of pure compounds [100 .. 199]

// TODO: make enum class
// TODO: rename enum
/**
 * \brief Identifiers of constant material properties.
 */
enum ECompoundConstProperties : unsigned
{
	CONST_PROP_NO_PROERTY                        = 100, ///< Undefined.
	CRITICAL_PRESSURE                            = 107,	///< Critical pressure [Pa].
	CRITICAL_TEMPERATURE                         = 108,	///< Critical temperature [K].
	HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT      = 114,	///< Heat of fusion at normal freezing point [J/mol].
	HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT = 115,	///< Heat of vaporization at normal freezing point [J/mol].
	MOLAR_MASS                                   = 121,	///< Molar mass [kg/mol].
	NORMAL_BOILING_POINT                         = 122,	///< Normal boiling point [K].
	NORMAL_FREEZING_POINT                        = 123,	///< Normal freezing point [K].
	STANDARD_FORMATION_ENTHALPY                  = 131,	///< Standard enthalpy of formation [J/mol].
	BOND_WORK_INDEX								 = 132,	///< Bond work index [kWh/t].
	SOA_AT_NORMAL_CONDITIONS                     = 141,	///< State of aggregation at normal conditions [-].
	CONST_PROP_USER_DEFINED_01                   = 150,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_02                   = 151,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_03                   = 152,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_04                   = 153,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_05                   = 154,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_06                   = 155,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_07                   = 156,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_08                   = 157,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_09                   = 158,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_10                   = 159,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_11                   = 160,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_12                   = 161,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_13                   = 162,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_14                   = 163,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_15                   = 164,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_16                   = 165,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_17                   = 166,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_18                   = 167,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_19                   = 168,	///< User-defined constant property.
	CONST_PROP_USER_DEFINED_20                   = 169,	///< User-defined constant property.
};

namespace MDBDescriptors
{
	/**
	 * \private
	 * \brief Base structure to describe initial values of all parameters.
	 */
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

	/**
	 * \private
	 * \brief Initial values of all parameters of a CConstProperty.
	 */
	struct SCompoundConstPropertyDescriptor : SCompoundPropertyDescriptor
	{
		double defaultValue{};
		SCompoundConstPropertyDescriptor() = default;
		SCompoundConstPropertyDescriptor(const std::string& _name, const std::wstring& _units, const std::string& _description, double _defaultValue)
			: SCompoundPropertyDescriptor{ _name, _units, _description }, defaultValue{ _defaultValue } {}
	};

	/**
	 * \private
	 */
	using constDescr = std::map<ECompoundConstProperties, SCompoundConstPropertyDescriptor>;

	// List of initial values of const properties
	static constDescr defaultConstProperties
	{
		{ CRITICAL_PRESSURE                            , { "Critical pressure"            , L"Pa"     , ""                                                                                                    , 0 } },
		{ CRITICAL_TEMPERATURE                         , { "Critical temperature"         , L"K"      , ""                                                                                                    , 0 } },
		{ HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT      , { "Heat of fusion"               , L"J/mol"  , "Heat of fusion at normal freezing point"                                                             , 0 } },
		{ HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT , { "Heat of vaporization "        , L"J/mol"  , "Heat of vaporization at normal boiling point"                                                        , 0 } },
		{ MOLAR_MASS                                   , { "Molar mass"                   , L"kg/mol" , ""                                                                                                    , 0.0180154 } },
		{ NORMAL_BOILING_POINT                         , { "Normal boiling point"         , L"K"      , ""                                                                                                    , 0 } },
		{ NORMAL_FREEZING_POINT                        , { "Normal freezing point"        , L"K"      , ""                                                                                                    , 0 } },
		{ STANDARD_FORMATION_ENTHALPY                  , { "Formation enthalpy"           , L"J/mol"  , "Standard enthalpy of formation"                                                                      , 0 } },
		{ BOND_WORK_INDEX                              , { "Bond work index"              , L"kWh/t"  , "Bond work index"																					  , 12 } },
		{ SOA_AT_NORMAL_CONDITIONS                     , { "State of aggregation"         , L"-"      , "State of aggregation at normal conditions\n[ 0 - solid ] [ 1 - liquid ] [ 2 - gas ] [ 3 - unknown ]" , 1 } },
	};
}


//////////////////////////////////////////////////////////////////////////
// Temperature/pressure-dependent properties of pure compounds [200 .. 299]

// TODO: make enum class
// TODO: rename enum
/**
 * \brief Identifiers of temperature/pressure-dependent properties.
 */
enum ECompoundTPProperties : unsigned
{
	TP_PROP_NO_PROERTY           = 200, ///< Undefined.
	HEAT_CAPACITY_CP             = 207, ///< Heat capacity [J/(kg·K)].
	ENTHALPY                     = 208,	///< Enthalpy [J/kg].
	THERMAL_CONDUCTIVITY         = 222,	///< Thermal conductivity [W/(m·K)].
	EQUILIBRIUM_MOISTURE_CONTENT = 223,	///< Equilibrium moisture content [kg/kg].
	VAPOR_PRESSURE               = 224,	///< Vapor pressure [Pa].
	VISCOSITY                    = 226,	///< Dynamic viscosity [Pa·s].
	DENSITY                      = 234,	///< Density [kg/m<sup>3</sup>].
	PERMITTIVITY                 = 235,	///< Permittivity [F/m].
	TP_PROP_USER_DEFINED_01      = 250,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_02      = 251,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_03      = 252,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_04      = 253,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_05      = 254,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_06      = 255,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_07      = 256,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_08      = 257,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_09      = 258,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_10      = 259,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_11      = 260,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_12      = 261,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_13      = 262,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_14      = 263,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_15      = 264,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_16      = 265,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_17      = 266,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_18      = 267,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_19      = 268,	///< User-defined dependent property.
	TP_PROP_USER_DEFINED_20      = 269,	///< User-defined dependent property.
};

namespace MDBDescriptors
{
	/**
	 * \private
	 * \brief Initial values of all parameters of a CTPDProperty.
	 */
	struct SCompoundTPDPropertyDescriptor : SCompoundPropertyDescriptor
	{
		ECorrelationTypes defuaultType{ ECorrelationTypes::CONSTANT };
		std::vector<double> defaultParameters{0.0};
		SCompoundTPDPropertyDescriptor() = default;
		SCompoundTPDPropertyDescriptor(const std::string& _name, const std::wstring& _units, const std::string& _description, ECorrelationTypes _defuaultType, std::vector<double> _defaultValue)
			: SCompoundPropertyDescriptor{ _name, _units, _description }, defuaultType{ _defuaultType }, defaultParameters{std::move(_defaultValue)} {}
	};

	/**
	 * \private
	 */
	using tpdepDescr = std::map<ECompoundTPProperties, MDBDescriptors::SCompoundTPDPropertyDescriptor>;

	// List of initial values of temperature/pressure - dependent properties
	static tpdepDescr defaultTPDProperties =
	{
		{ HEAT_CAPACITY_CP             , { "Heat capacity"                , L"J/(kg·K)" 		, "" , ECorrelationTypes::CONSTANT , { 1000 } } },
		{ ENTHALPY                     , { "Enthalpy"                     , L"J/kg" 			, "" , ECorrelationTypes::CONSTANT , { 4277.4 } } },
		{ THERMAL_CONDUCTIVITY         , { "Thermal conductivity"         , L"W/(m·K)" 			, "" , ECorrelationTypes::CONSTANT , { 0 } } },
		{ EQUILIBRIUM_MOISTURE_CONTENT , { "Equilibrium moisture content" , L"kg/kg"            , "" , ECorrelationTypes::CONSTANT , { 0 } } },
		{ VAPOR_PRESSURE               , { "Vapor pressure"               , L"Pa" 				, "" , ECorrelationTypes::CONSTANT , { 0 } } },
		{ VISCOSITY                    , { "Dynamic Viscosity"            , L"Pa·s" 			, "" , ECorrelationTypes::CONSTANT , { 0 } } },
		{ DENSITY                      , { "Density"                      , L"kg/m<sup>3</sup>" , "" , ECorrelationTypes::CONSTANT , { 1000 } } },
		{ PERMITTIVITY                 , { "Permittivity"                 , L"F/m" 				, "" , ECorrelationTypes::CONSTANT , { 0 } } },
	};
}


//////////////////////////////////////////////////////////////////////////
// Interactions properties between two pure compounds [300 .. 399]

// TODO: make enum class
// TODO: rename enum
/**
 * \brief Identifiers of properties, defined for interaction of two compounds.
 */
enum EInteractionProperties : unsigned
{
	INT_PROP_NO_PROERTY      = 300, ///< Undefined.
	INTERFACE_TENSION        = 303, ///< Interface tension [N/m].
	INT_PROP_USER_DEFINED_01 = 350,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_02 = 351,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_03 = 352,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_04 = 353,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_05 = 354,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_06 = 355,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_07 = 356,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_08 = 357,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_09 = 358,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_10 = 359,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_11 = 360,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_12 = 361,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_13 = 362,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_14 = 363,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_15 = 364,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_16 = 365,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_17 = 366,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_18 = 367,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_19 = 368,	///< User-defined interaction property.
	INT_PROP_USER_DEFINED_20 = 369,	///< User-defined interaction property.
};

namespace MDBDescriptors
{
	/**
	 * \private
	 */
	using interDescr = std::map<EInteractionProperties, SCompoundTPDPropertyDescriptor>;

	// List of descriptors of temperature/pressure-dependent properties
	static interDescr defaultInteractionProperties
	{
		{ INTERFACE_TENSION , { "Interface tension" , L"N/m" , "" , ECorrelationTypes::CONSTANT , { 0 } } },
	};
}

namespace MDBDescriptors
{
	const unsigned FIRST_CONST_USER_PROP = CONST_PROP_USER_DEFINED_01; ///< Identifier of the first constant property.
	const unsigned FIRST_TPDEP_USER_PROP = TP_PROP_USER_DEFINED_01;	   ///< Identifier of the first dependent property.
	const unsigned FIRST_INTER_USER_PROP = INT_PROP_USER_DEFINED_01;   ///< Identifier of the first interaction property.

	/**
	 * \private
	 * \brief Descriptor of the material property.
	 */
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
