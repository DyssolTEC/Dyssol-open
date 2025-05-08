/* Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

struct SDistributionFunctionDescriptor
{
	std::string name;						///< Name of the function.
	std::vector<std::string> paramNames;	///< Names of the function's parameters.
	std::vector<double> paramInitValues{};	///< Initial values of the function's parameters
};

/**
 * 
 * Creates and returns a descriptor of the given distribution function.
 * \param _function Distribution function.
 * \return Descriptor of the distribution function.
 */
SDistributionFunctionDescriptor DistributionFunctionDescriptor(EDistributionFunction _function);

/**
 * Returns normal distribution as the probability density function.
 * \details Calculated as:
 * \f$y(x) = \frac{1}{\sqrt{2\pi\sigma^{2}}}e^{-\frac{{(x-\mu)}^{2}}{2\sigma^{2}}}\f$.
 * \f$\sigma \neq 0\f$.
 * Returns empty vector on not allowed parameters.
 * \param _x Points for which the distribution has to be generated.
 * \param _mu Mean value.
 * \param _sigma Standard deviation.
 * \return Distribution.
 */
std::vector<double> CreateDistributionNormal(const std::vector<double>& _x, double _mu, double _sigma);

/**
 * Returns log-normal distribution as the probability density function.
 * \details Calculated as:
 * \f$y(x) = \frac{1}{x\sigma\sqrt{2\pi}}e^{-\frac{{(\ln{x}-\mu)}^{2}}{2\sigma^{2}}}\f$.
 * \f$\sigma > 0\f$.
 * \f$x > 0\f$.
 * Returns empty vector on not allowed parameters.
 * \param _x Points for which the distribution has to be generated.
 * \param _mu Mean value.
 * \param _sigma Standard deviation.
 * \return Distribution.
 */
std::vector<double> CreateDistributionLogNormal(const std::vector<double>& _x, double _mu, double _sigma);

/**
 * Returns Rosin-Rammler-Sperling-Bennett distribution as the probability density function.
 * \details Calculated as:
 * \f$y(x) = \frac{k}{\lambda}\left( \frac{x}{\lambda} \right)^{k-1}e^{-\left( \frac{x}{\lambda}\right)^k}\f$.
 * \f$\lambda \neq 0\f$.
 * \f$k > 0\f$.
 * \f$x > 0\f$.
 * Returns empty vector on not allowed parameters.
 * \param _x Points for which the distribution has to be generated.
 * \param _x63 Characteristic size.
 * \param _n Distribution modulus.
 * \return Distribution.
 */
std::vector<double> CreateDistributionRRSB(const std::vector<double>& _x, double _x63, double _n);

/**
 * Returns Gates-Gaudin-Schumann distributed probability density function.
 * \details Calculated as:
 * \f$y(x) = \frac{m}{x_{max}}\left( \frac{x}{x_{max}} \right)^{m-1}\f$.
 * \f$x_{max} > 0\f$.
 * \f$m > 0\f$.
 * \f$ 0 \le x \le x_{max}\f$.
 * Returns empty vector on not allowed parameters.
 * \param _x Points for which the distribution has to be generated.
 * \param _xmax Maximum size.
 * \param _m Distribution modulus.
 * \return Distribution.
 */
std::vector<double> CreateDistributionGGS(const std::vector<double>& _x, double _xmax, double _m);

/**
 * Returns the given probability density function.
 * \details If EDistributionFunction::MANUAL is chosen, returns vector of zeroes.
 * \param _type Type of the distribution function.
 * \param _x Points for which the distribution has to be generated.
 * \param _param1 Parameter of the distribution function, depending on the chosen distribution type.
 * \param _param2 Parameter of the distribution function, depending on the chosen distribution type.
 * \return Distribution.
 */
std::vector<double> CreateDistribution(EDistributionFunction _type, const std::vector<double>& _x, double _param1, double _param2);
