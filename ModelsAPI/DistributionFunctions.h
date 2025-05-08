/* Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

/**
 * Types of distribution functions.
 */
enum class EDistributionFunction : uint8_t
{
	MANUAL = 0,		///< Manual distribution.
	NORMAL = 1,		///< Normal distribution.
	RRSB = 2,		///< Rosin-Rammler-Sperling-Bennett distribution.
	GGS = 3,		///< Gates-Gaudin-Schuhmann distribution.
	LOG_NORMAL = 4,	///< Log-normal distribution.

	/// \cond <- to hide in the Doxygen output
	COUNT_	        ///< Number of distribution functions.
	/// \endcond
};

/**
 * \cond <- to hide in the Doxygen output
 * \brief Implementation details for distribution functions.
 */
namespace DistributionFunction
{
	struct SFunctionParam
	{
		std::string_view name;	///< Name of the function parameter.
		double initValue;		///< Initial value of the function parameter.
	};

	struct SFunctionDescriptor
	{
		std::string_view name;				///< Name of the function.
		std::vector<SFunctionParam> params; ///< Parameters of the function.
	};

	const SFunctionDescriptor& GetFunctionDescriptor(EDistributionFunction _type);
	std::vector<std::string> GetFunctionParameterNames(EDistributionFunction _type);
}
/**
 * \endcond
 */

/**
 * Returns the given probability density function.
 * \details Available functions:
 * - EDistributionFunction::NORMAL:
 *   \f$y(x) = \frac{1}{\sqrt{2\pi\sigma^{2}}}e^{-\frac{(x-\mu)^{2}}{2\sigma^{2}}}\f$,
 *   
 *   with \f$\sigma \neq 0\f$,
 *   \f$\mu\f$ - mean value,
 *   \f$\sigma\f$ - standard deviation.
 * - EDistributionFunction::LOG_NORMAL:
 *   \f$y(x) = \frac{1}{x\sigma\sqrt{2\pi}}e^{-\frac{(\ln x-\mu)^{2}}{2\sigma^{2}}}\f$,
 *   
 *   with \f$\sigma > 0\f$ and \f$x > 0\f$,
 *   \f$\mu\f$ - mean value,
 *   \f$\sigma\f$ - standard deviation.
 * - EDistributionFunction::RRSB:
 *   \f$y(x) = \frac{k}{\lambda}\left(\frac{x}{\lambda}\right)^{k-1}e^{-\left(\frac{x}{\lambda}\right)^{k}}\f$,
 *   
 *   with \f$\lambda \neq 0\f$, \f$k > 0\f$ and \f$x > 0\f$,
 *   \f$\lambda\f$ - characteristic size,
 *   \f$k\f$ - distribution modulus.
 * - EDistributionFunction::GGS:
 *   \f$y(x) = \frac{m}{x_{max}}\left(\frac{x}{x_{max}}\right)^{m-1}\f$,
 *   
 *   with \f$x_{max} > 0\f$, \f$m > 0\f$ and \f$0 \le x \le x_{max}\f$,
 *   \f$x_{max}\f$ - maximum size,
 *   \f$m\f$ - distribution modulus.
 * - EDistributionFunction::MANUAL:
 *   returns a vector of zeroes.
 *
 * Returns an empty vector if the constraints on function parameters are not satisfied.
 * \param _type    Type of the distribution function.
 * \param _x       Points for which the distribution has to be generated.
 * \param _param1  First parameter of the distribution function:
 *                 - EDistributionFunction::NORMAL: mean \f$\mu\f$
 *                 - EDistributionFunction::LOG_NORMAL: mean \f$\mu\f$
 *                 - EDistributionFunction::RRSB: characteristic size \f$\lambda\f$
 *                 - EDistributionFunction::GGS: maximum size \f$x_{max}\f$
 * \param _param2  Second parameter of the distribution function:
 *                 - EDistributionFunction::NORMAL: standard deviation \f$\sigma\f$
 *                 - EDistributionFunction::LOG_NORMAL: standard deviation \f$\sigma\f$
 *                 - EDistributionFunction::RRSB: distribution modulus \f$k\f$
 *                 - EDistributionFunction::GGS: distribution modulus \f$m\f$
 * \return         Distribution.
 */
std::vector<double> CreateDistribution(EDistributionFunction _type, const std::vector<double>& _x, double _param1, double _param2);