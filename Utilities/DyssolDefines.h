/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/**
\file DyssolDefines.h
\details This definition is needed for Doxygen to properly generate documentation.
*/

#pragma once

#include <cstdint>
#include <iterator>

// ========== Initial values

// Simulator
constexpr double   DEFAULT_SIMULATION_TIME_BEGIN      = 0;       ///< Default value.
constexpr double   DEFAULT_SIMULATION_TIME            = 60;      ///< Default value.
constexpr double   DEFAULT_INIT_TIME_WINDOW           = 1;       ///< Default value.
constexpr double   DEFAULT_MIN_TIME_WINDOW            = 1e-9;    ///< Default value.
constexpr double   DEFAULT_MAX_TIME_WINDOW            = 1000000; ///< Default value.
constexpr double   DEFAULT_WINDOW_MAGNIFICATION_RATIO = 1.2;     ///< Default value.
constexpr double   DEFAULT_WEGSTEIN_ACCEL_PARAM       = -0.5;    ///< Default value.
constexpr double   DEFAULT_RELAXATION_PARAM           = 1;       ///< Default value.
constexpr uint32_t DEFAULT_MAX_ITERATIONS_NUMBER      = 500;     ///< Default value.
constexpr uint32_t DEFAULT_ITERS_UPPER_LIMIT          = 7;       ///< Default value.
constexpr uint32_t DEFAULT_ITERS_LOWER_LIMIT          = 3;       ///< Default value.
constexpr uint32_t DEFAULT_ITERS_1ST_UPPER_LIMIT      = 20;      ///< Default value.

// Initial tolerances
constexpr double DEFAULT_A_TOL = 1e-6; ///< Default value.
constexpr double DEFAULT_R_TOL = 1e-3; ///< Default value.

// Cache
constexpr bool     DEFAULT_CACHE_FLAG_STREAMS  = true;  ///< Default value.
constexpr bool     DEFAULT_CACHE_FLAG_HOLDUPS  = false; ///< Default value.
constexpr bool     DEFAULT_CACHE_FLAG_INTERNAL = false; ///< Default value.
constexpr uint32_t DEFAULT_CACHE_WINDOW      = 100;     ///< Default value.

// Initial minimal fraction
constexpr double DEFAULT_MIN_FRACTION = 0; ///< Default value.

// Enthalpy calculator
constexpr double   DEFAULT_ENTHALPY_MIN_T     = 173;  ///< Default value.
constexpr double   DEFAULT_ENTHALPY_MAX_T     = 1273; ///< Default value.
constexpr uint32_t DEFAULT_ENTHALPY_INTERVALS = 100;  ///< Default value.


/**
 * Convergence methods.
 */
enum class EConvergenceMethod : uint32_t
{
	DIRECT_SUBSTITUTION	= 0,
	WEGSTEIN			= 1,
	STEFFENSEN			= 2
};

/**
 * Extrapolation methods.
 */
enum class EExtrapolationMethod : uint32_t
{
	LINEAR	= 0,
	SPLINE	= 1,
	NEAREST	= 2
};

//======== SOLID DISTRIBUTIONS DATABASE [0; 50] ===============

// TODO: make enum class
/**
 * \brief Types of distributed properties of the solid phase.
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
	DISTR_UNDEFINED		  = 31,	///< Distribution type is undefined.
};

/**
 * \brief Names of all distributed properties.
 * \details Must be in the same order as DISTR_TYPES.
 */
inline static constexpr const char* const DISTR_NAMES[] = {	"Compounds", "Size", "Particle porosity", "Form factor",
															"Color", "Moisture", "Distribution 1", "Distribution 2",
															"Distribution 3", "Distribution 4", "Distribution 5",
															"Distribution 6", "Distribution 7", "Distribution 8",
															"Distribution 9", "Distribution 10" };
/**
 * \brief Types of all distributed properties.
 * \details Must be in the same order as DISTR_NAMES.
 */
inline static constexpr EDistrTypes DISTR_TYPES[] = { DISTR_COMPOUNDS,  DISTR_SIZE, DISTR_PART_POROSITY, DISTR_FORM_FACTOR,
											DISTR_COLOR, DISTR_MOISTURE, DISTR_USER_DEFINED_01, DISTR_USER_DEFINED_02,
											DISTR_USER_DEFINED_03, DISTR_USER_DEFINED_04, DISTR_USER_DEFINED_05,
											DISTR_USER_DEFINED_06, DISTR_USER_DEFINED_07, DISTR_USER_DEFINED_08,
											DISTR_USER_DEFINED_09, DISTR_USER_DEFINED_10 };

static_assert(std::size(DISTR_NAMES) == std::size(DISTR_TYPES));

/**
 * \brief Calculates index of the distributed property from its type.
 * \details Indices from DISTR_TYPES.
 */
inline int GetDistributionTypeIndex(EDistrTypes _nType)
{
	for (unsigned i = 0; i < std::size(DISTR_TYPES); ++i)
		if (_nType == DISTR_TYPES[i])
			return i;
	return -1;
}

/**
 * \brief Types of grid entries for distributed properties of the solid phase.
 */
enum class EGridEntry : uint32_t
{
	GRID_NUMERIC	= 0,  ///< Numeric grid.
	GRID_SYMBOLIC	= 2,  ///< Symbolic grid.
	GRID_UNDEFINED  = 15, ///< Grid type is undefined.
};

/**
 * Types of grid functional distributions.
 */
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

/**
 * Types of solving strategies for NLSolver.
 */
enum class ENLSolverStrategy : uint32_t
{
	Newton, Linesearch, Picard, Fixedpoint
};

/**
* \brief Identifiers of PSD types.
* \details
* \note
* - \f$i\f$ - index of size classes
* - \f$j\f$ - index of compounds
* - \f$k\f$ - index of porosities
* - \f$d_i\f$ - particle diameter of class \f$i\f$
* - \f$\Delta d_i\f$ - size of the class \f$i\f$
* - \f$m_i\f$ - mass of particles of class \f$i\f$
* - \f$M_{tot}\f$ - total mass of particles
* - \f$N_i\f$ - number of particles of class \f$i\f$
* - \f$N_{i,j}\f$ - number of particles of compound \f$j\f$ of size class \f$i\f$
* - \f$N_{tot}\f$ - total number of particles
* - \f$w_i\f$ - mass fraction of particles of class \f$i\f$
* - \f$w_{i,j}\f$ - mass fraction of particles of compound \f$j\f$ of size class \f$i\f$
* - \f$w_{i,j,k}\f$ - mass fraction of particles of compound \f$j\f$ of size class \f$i\f$ and porosity \f$k\f$
* - \f$\rho_{j}\f$ - density of compound \f$j\f$
* - \f$\varepsilon_{k}\f$ - porosity of class \f$k\f$
* - \f$q_0\f$ - number-related density distribution
* - \f$Q_0\f$ - number-related cumulative distribution
* - \f$q_2\f$ - surface-related density distribution
* - \f$Q_2\f$ - surface-related cumulative distribution
* - \f$q_3\f$ - mass-related density distribution
* - \f$Q_3\f$ - mass-related cumulative distribution
*/
enum EPSDTypes
{
	PSD_q3       = 0, ///< Mass-related density distribution: \f$q_{3,i} = \frac{w_i}{\Delta d_i}\f$.
	PSD_Q3       = 1, ///< Mass-related cumulative distribution: \f$Q_{3,0} = w_0\f$, \f$Q_{3,i} = Q_{3,i-1} + w_i\f$.
	PSD_q0       = 2, ///< Number-related density distribution: \f$q_{0,i} = \frac{N_i}{N_{tot} \Delta d_i}\f$.
	PSD_Q0       = 3, ///< Number-related cumulative distribution: \f$Q_{0,i} = Q_{0,i-1} + q_{0,i} \Delta d_i\f$.
	PSD_MassFrac = 4, ///< Size distribution in the form of mass fractions with the total sum of 1.
	PSD_Number   = 5, ///< Number-related distribution of particles, depends on several conditions:
					  ///< -# If only one compound is specified: \f$N_i = \frac{m_i}{\rho \frac{\pi}{6} d_i^3}\f$.
					  ///< -# For several compounds: \f$N_i = \sum_j \frac{M_{tot} w_{i,j}}{\frac{\pi d_i^3}{6} \rho_j}\f$.
					  ///< -# If distribution by particle porosity defined: \f$N_i = \sum_j N_{i,j}\f$, with \f$N_{i, j} = \sum_k \frac{M_{tot} w_{i,j,k}}{\frac{\pi d_i^3}{6} \rho_j (1 - \varepsilon_k)}\f$.
	PSD_q2       = 6, ///< Surface-area-related density distribution: \f$q_{2,i} = \frac{Q_{2,i} - Q_{2,i-1}}{\Delta d_i}\f$.
	PSD_Q2       = 7  ///< Surface-area-related cumulative distribution: \f$Q_{2,i} = \frac{\sum_{j=0}^i N_j \pi d_j^2}{\sum_j N_j \pi d_j^2}\f$.
};

/**
 * For lookup table creation.
 */
enum class EDependencyTypes
{
	DEPENDENCE_UNKNOWN = 0,
	DEPENDENCE_TEMP = 1,

	DEPENDENCE_PRES = 2
};

// ========== Universal constants

constexpr double AVOGADRO_CONSTANT                = 6.022141994747e+23;     ///< Avogadro constant [1/mol].
constexpr double BOLTZMANN_CONSTANT               = 1.38065032424e-23;      ///< Boltzmann constant [J/K].
constexpr double MOLAR_GAS_CONSTANT               = 8.314459848;            ///< Molar gas constant [J/mol/K].
constexpr double SPEED_OF_LIGHT_IN_VACUUM         = 2.9979245811e+8;        ///< Speed of light in vacuum [m/s].
constexpr double STANDARD_ACCELERATION_OF_GRAVITY = 9.80665;                ///< Standard acceleration of gravity [m/s<sup>2</sup>].
constexpr double STANDARD_CONDITION_T             = 298.15;                 ///< Standard condition temperature [K].
constexpr double STANDARD_CONDITION_P             = 101325;                 ///< Standard condition pressure [Pa].
constexpr double STEFAN_BOLTZMANN_CONSTANT        = 5.670374419e-8;         ///< Stefan-Boltzmann constant [W/m<sup>2</sup>/K<sup>4</sup>].
constexpr double MATH_PI                          = 3.14159265358979323846; ///< \f$\pi\f$ constant.


//////////////////////////////////////////////////////////////////////////
/// Common enumerators
//////////////////////////////////////////////////////////////////////////

/**
 * \brief Identifiers of grid unit types.
 */
enum class EPSDGridType : unsigned
{
	DIAMETER = 0, ///< Diameter-based size-grid [m].
	VOLUME = 1	  ///< Volume-based size-grid [m<sup>3</sup>].
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
	GAS       = VAPOR, ///< Gas phase.
};

/**
 * \brief Identifiers of time-dependent overall properties.
 */
enum class EOverall : uint32_t
{
	OVERALL_MASS,					///< Overall mass or mass flow.
	OVERALL_TEMPERATURE,			///< Overall temperature.
	OVERALL_PRESSURE,				///< Overall pressure.
	OVERALL_USER_DEFINED_01 = 201,	///< User-defined overall property.
	OVERALL_USER_DEFINED_02 = 202,	///< User-defined overall property.
	OVERALL_USER_DEFINED_03 = 203,	///< User-defined overall property.
	OVERALL_USER_DEFINED_04 = 204,	///< User-defined overall property.
	OVERALL_USER_DEFINED_05 = 205,	///< User-defined overall property.
	OVERALL_USER_DEFINED_06 = 206,	///< User-defined overall property.
	OVERALL_USER_DEFINED_07 = 207,	///< User-defined overall property.
	OVERALL_USER_DEFINED_08 = 208,	///< User-defined overall property.
	OVERALL_USER_DEFINED_09 = 209,	///< User-defined overall property.
	OVERALL_USER_DEFINED_10 = 210,	///< User-defined overall property.
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
