/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/**
\file DyssolDefines.h
\details This definition is needed for Doxygen to properly generate documentation.
*/

#pragma once

#include <cstdint>
#include <cstddef>

// ========== Initial values

// Simulator
#define DEFAULT_MAX_ITERATIONS_NUMBER		500
#define DEFAULT_SIMULATION_TIME				60
#define DEFAULT_INIT_TIME_WINDOW			1
#define DEFAULT_MIN_TIME_WINDOW				1e-9
#define DEFAULT_MAX_TIME_WINDOW				1000000
#define DEFAULT_ITERS_UPPER_LIMIT			7
#define DEFAULT_ITERS_LOWER_LIMIT			3
#define DEFAULT_ITERS_1ST_UPPER_LIMIT		20
#define DEFAULT_WINDOW_MAGNIFICATION_RATIO	1.2
#define	DEFAULT_WEGSTEIN_ACCEL_PARAM		-0.5
#define DEFAULT_RELAXATION_PARAM			1

// Cache
#define DEFAULT_CACHE_FLAG_STREAMS		true
#define DEFAULT_CACHE_FLAG_HOLDUPS		false
#define DEFAULT_CACHE_FLAG_INTERNAL		false
#define DEFAULT_CACHE_WINDOW			100

// Initial tolerances
#define DEFAULT_A_TOL	1e-6
#define DEFAULT_R_TOL	1e-3

// Initial minimal fraction
#define DEFAULT_MIN_FRACTION	0

// Enthalpy calculator
#define DEFAULT_ENTHALPY_MIN_T     173
#define DEFAULT_ENTHALPY_MAX_T     1273
#define DEFAULT_ENTHALPY_INTERVALS 100


// ========== Convergence methods
enum class EConvergenceMethod : uint32_t
{
	DIRECT_SUBSTITUTION	= 0,
	WEGSTEIN			= 1,
	STEFFENSEN			= 2
};

// ========== Extrapolation methods
enum class EExtrapolationMethod : uint32_t
{
	LINEAR	= 0,
	SPLINE	= 1,
	NEAREST	= 2
};

//======== SOLID DISTRIBUTIONS DATABASE [0; 50] ===============
#define DISTRIBUTIONS_NUMBER 15

// TODO: make enum class
/**
 * \brief Types of distributed parameters of the solid phase.
 */
enum EDistrTypes : uint32_t
{
	DISTR_COMPOUNDS		  = 0,  ///< Distribution by compounds.
	DISTR_SIZE			  = 1,	///< Distribution by particle size.
	DISTR_PART_POROSITY	  = 2,	///< Distribution by porosity.
	DISTR_FORM_FACTOR	  = 3,	///< Distribution by form factor.
	DISTR_COLOR			  = 4,	///< Distribution by color.
	DISTR_MOISTURE		  = 5,	///< Distribution by moisture.
	DISTR_USER_DEFINED_01 = 20,	///< User-defined distribution.
	DISTR_USER_DEFINED_02 = 21,	///< User-defined distribution.
	DISTR_USER_DEFINED_03 = 22,	///< User-defined distribution.
	DISTR_USER_DEFINED_04 = 23,	///< User-defined distribution.
	DISTR_USER_DEFINED_05 = 24,	///< User-defined distribution.
	DISTR_USER_DEFINED_06 = 25,	///< User-defined distribution.
	DISTR_USER_DEFINED_07 = 26,	///< User-defined distribution.
	DISTR_USER_DEFINED_08 = 27,	///< User-defined distribution.
	DISTR_USER_DEFINED_09 = 28,	///< User-defined distribution.
	DISTR_USER_DEFINED_10 = 29,	///< User-defined distribution.
	DISTR_UNDEFINED		  = 31,	///< Type is undefined.
};

#define DISTR_NAMES { "Compounds", "Size", "Particle porosity", "Form factor", "Color", "Moisture", "Distribution 1", "Distribution 2", "Distribution 3", "Distribution 4", "Distribution 5", "Distribution 6", "Distribution 7", "Distribution 8", "Distribution 9", "Distribution 10" }
#define DISTR_TYPES { DISTR_COMPOUNDS,  DISTR_SIZE, DISTR_PART_POROSITY, DISTR_FORM_FACTOR, DISTR_COLOR, DISTR_MOISTURE, DISTR_USER_DEFINED_01, DISTR_USER_DEFINED_02, DISTR_USER_DEFINED_03, DISTR_USER_DEFINED_04, DISTR_USER_DEFINED_05, DISTR_USER_DEFINED_06, DISTR_USER_DEFINED_07, DISTR_USER_DEFINED_08, DISTR_USER_DEFINED_09, DISTR_USER_DEFINED_10}
inline int GetDistributionTypeIndex(EDistrTypes _nType)
{
	EDistrTypes vTypes[] = DISTR_TYPES;
	for (unsigned i = 0; i < DISTRIBUTIONS_NUMBER; ++i)
		if (_nType == vTypes[i])
			return i;
	return -1;
}


// ========== m_StreamMTP indexes
#define MTP_MASS		0
#define MTP_TEMPERATURE	1
#define MTP_PRESSURE	2

/**
 * \brief Types of grid entries of distributed parameters of the solid phase.
 */
enum class EGridEntry : uint32_t
{
	GRID_NUMERIC	= 0,  ///< Numeric grid.
	GRID_SYMBOLIC	= 2,  ///< Symbolic grid.
	GRID_UNDEFINED  = 15, ///< Type is undefined.
};

// ========== TYPES OF GRID FUNCTIONAL DISTRIBUTIONS
enum class EGridFunction : uint32_t
{
	GRID_FUN_MANUAL	         = 0,
	GRID_FUN_EQUIDISTANT	 = 1,
	GRID_FUN_GEOMETRIC_S2L	 = 2,
	GRID_FUN_LOGARITHMIC_S2L = 3,
	GRID_FUN_GEOMETRIC_L2S	 = 4,
	GRID_FUN_LOGARITHMIC_L2S = 5,
	GRID_FUN_UNDEFINED		 = 15,
};

// ========== TYPES OF SOLVING STRATEGIES FOR NLSOLVER
enum class ENLSolverStrategy : uint32_t
{
	Newton, Linesearch, Picard, Fixedpoint
};

// ========== PSD TYPES
enum EPSDTypes
{
	PSD_q3       = 0,
	PSD_Q3       = 1,
	PSD_q0       = 2,
	PSD_Q0       = 3,
	PSD_MassFrac = 4,
	PSD_Number   = 5,
	PSD_q2       = 6,
	PSD_Q2       = 7
};

// ========== For lookup table creation
enum class EDependencyTypes
{
	DEPENDENCE_UNKNOWN = 0,
	DEPENDENCE_TEMP = 1,

	DEPENDENCE_PRES = 2
};

// ========== Non-constant single-phase mixture properties and overall properties [300..399]

// TODO: remove them
//#define ENTHALPY				308 // overall
#define FLOW					320	// overall (stream)
#define MASS					320	// overall (holdup)
#define FRACTION				321
#define PHASE_FRACTION			327
#define PRESSURE				338	// overall
#define TEMPERATURE				340	// overall
#define TOTAL_FLOW				342	// overall (stream)
#define TOTAL_MASS				342	// overall (holdup)

// ========== Universal constants

#define AVOGADRO_CONSTANT					6.022141994747e+23
#define BOLTZMANN_CONSTANT					1.38065032424e-23
#define IDEAL_GAS_STATE_REFERENCE_PRESSURE	101325
#define MOLAR_GAS_CONSTANT					8.314459848
#define SPEED_OF_LIGHT_IN_VACUUM			2.9979245811e+8
#define STANDARD_ACCELERATION_OF_GRAVITY	9.80665
#define STANDARD_CONDITION_T				298.15	               ///< Standard condition temperature [K].
#define STANDARD_CONDITION_P				101325	               ///< Standard condition pressure [Pa].
#define STEFAN_BOLTZMANN_CONSTANT			5.670374419e-8		   // W/m^2.K^4
#define MATH_PI								3.14159265358979323846

// ========== Value basis

enum eValueBasises
{
	BASIS_MASS, BASIS_MOLL
};


//////////////////////////////////////////////////////////////////////////
/// Common enumerators
//////////////////////////////////////////////////////////////////////////
enum class EDistrFunction : unsigned
{
	Manual = 0,
	Normal = 1,
	RRSB = 2,
	GGS = 3,
	LogNormal = 4
};

enum class EPSDGridType : unsigned
{
	DIAMETER = 0,
	VOLUME = 1
};

/**
 * \brief Identifiers of phase types.
 */
enum class EPhase : uint32_t
{
	UNDEFINED = 0, ///< Type is undefined.
	SOLID     = 1, ///< Solid phase.
	LIQUID    = 2, ///< Liquid phase.
	VAPOR     = 3, ///< Gas phase.
	GAS       = 3, ///< Gas phase.
};

// Identifiers of time-dependent overall parameters.
enum class EOverall : uint32_t
{
	// TODO: rename when corresponding defines are removed
	OVERALL_MASS,
	OVERALL_TEMPERATURE,
	OVERALL_PRESSURE,
	OVERALL_USER_DEFINED_01 = 201,
	OVERALL_USER_DEFINED_02 = 202,
	OVERALL_USER_DEFINED_03 = 203,
	OVERALL_USER_DEFINED_04 = 204,
	OVERALL_USER_DEFINED_05 = 205,
	OVERALL_USER_DEFINED_06 = 206,
	OVERALL_USER_DEFINED_07 = 207,
	OVERALL_USER_DEFINED_08 = 208,
	OVERALL_USER_DEFINED_09 = 209,
	OVERALL_USER_DEFINED_10 = 210,
};

/**
 * \brief Types of unit ports.
 */
enum class EUnitPort : uint32_t
{
	INPUT     = 0, ///< Input port.
	OUTPUT    = 1, ///< Output port.
	UNDEFINED = 2  ///< Undefined.
};


////////////////////////////////////////////////////////////////////////////////
/// Deprecated types

// Identifiers of phase types
enum EPhaseTypes : unsigned
{
	SOA_SOLID,
	SOA_LIQUID,
	SOA_VAPOR,
	SOA_LIQUID2,
	SOA_UNDEFINED
};

enum EPortType
{
	INPUT_PORT = 0,
	OUTPUT_PORT = 1,
	UNDEFINED_PORT = 2
};
