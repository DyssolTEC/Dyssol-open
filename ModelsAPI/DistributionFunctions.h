/* Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <cstdint>
#include <array>
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

	COUNT_	        ///< Number of distribution functions.
};

namespace details
{
	struct SFunctionParam
	{
		std::string_view name;
		double initValue;
	};

	struct SFunctionDescriptor
	{
		std::string_view name;
		std::vector<SFunctionParam> params;
	};

	const SFunctionDescriptor& GetFunctionDescriptor(EDistributionFunction _type);
}

/**
 * Returns the given probability density function.
 *
 * \details
 * For EDistributionFunction::NORMAL:
 *   \f$y(x) = \frac{1}{\sqrt{2\pi\sigma^{2}}}e^{-\frac{(x-\mu)^{2}}{2\sigma^{2}}}\f$, with \f$\sigma \neq 0\f; returns empty vector otherwise.
 *
 * For EDistributionFunction::LOGNORMAL:
 *   \f$y(x) = \frac{1}{x\sigma\sqrt{2\pi}}e^{-\frac{(\ln x-\mu)^{2}}{2\sigma^{2}}}\f,
 *   with \f$\sigma > 0\f and \f$x > 0\f; returns empty vector otherwise.
 *
 * For EDistributionFunction::RRSB:
 *   \f$y(x) = \frac{k}{\lambda}\left(\frac{x}{\lambda}\right)^{k-1}e^{-\left(\frac{x}{\lambda}\right)^{k}}\f,
 *   with \f$\lambda \neq 0\f, \f$k > 0\f and \f$x > 0\f; returns empty vector otherwise.
 *
 * For EDistributionFunction::GGS:
 *   \f$y(x) = \frac{m}{x_{max}}\left(\frac{x}{x_{max}}\right)^{m-1}\f,
 *   with \f$x_{max} > 0\f, \f$m > 0\f and \f$0 \le x \le x_{max}\f; returns empty vector otherwise.
 *
 * For EDistributionFunction::MANUAL:
 *   returns a vector of zeroes.
 *
 * \param _type  Type of the distribution function.
 * \param _x     Points for which the distribution has to be generated.
 * \param _param1  First parameter of the distribution function:
 *                 - NORMAL, LOGNORMAL: mean (\f$\mu\f; any real)
 *                 - RRSB: characteristic size (\f$\lambda\f)
 *                 - GGS: maximum size (\f$x_{max}\f)
 * \param _param2  Second parameter of the distribution function:
 *                 - NORMAL, LOGNORMAL: standard deviation (\f$\sigma\f)
 *                 - RRSB: distribution modulus (\f$k\f)
 *                 - GGS: distribution modulus (\f$m\f)
 * \return         Distribution.
 */
std::vector<double> CreateDistribution(EDistributionFunction _type, const std::vector<double>& _x, double _param1, double _param2);