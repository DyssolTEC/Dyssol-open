/* Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DistributionFunctions.h"

#include <algorithm>
#include <cassert>
#include <cmath>
 // TODO: use std::numbers::pi (c++20) instead of M_PI, remove this include
#define _USE_MATH_DEFINES
#include <math.h>

namespace
{
	std::vector<double> CreateDistributionNormal(const std::vector<double>& _x, double _mu, double _sigma)
	{
		std::vector<double> res;
		if (_sigma == 0.0) return res;
		res.resize(_x.size());
		const double a = 1 / (_sigma * std::sqrt(2 * M_PI));
		for (size_t i = 0; i < _x.size(); ++i)
			res[i] = a * std::exp(-std::pow(_x[i] - _mu, 2) / (2 * _sigma * _sigma));
		return res;
	}

	std::vector<double> CreateDistributionLogNormal(const std::vector<double>& _x, double _mu, double _sigma)
		{
			std::vector<double> res;
			if (_sigma <= 0.0) return res;
			if (std::any_of(_x.begin(), _x.end(), [](double _v) { return _v <= 0.0; })) return res;
			res.resize(_x.size());
			const double a = 1 / (_sigma * std::sqrt(2 * M_PI));
			for (size_t i = 0; i < _x.size(); ++i)
				res[i] = a / _x[i] * std::exp(-std::pow(std::log(_x[i]) - _mu, 2) / (2 * _sigma * _sigma));
			return res;
		}

	std::vector<double> CreateDistributionRRSB(const std::vector<double>& _x, double _x63, double _n)
		{
			std::vector<double> res;
			if (_x63 == 0.0) return res;
			if (_n <= 0.0) return res;
			if (std::any_of(_x.begin(), _x.end(), [](double _v) { return _v <= 0.0; })) return res;
			res.resize(_x.size());
			for (size_t i = 0; i < _x.size(); ++i)
				res[i] = _n / _x63 * std::pow(_x[i] / _x63, _n - 1) * std::exp(-std::pow(_x[i] / _x63, _n));
			return res;
		}

	std::vector<double> CreateDistributionGGS(const std::vector<double>& _x, double _xmax, double _m)
		{
			std::vector<double> res;
			if (_m <= 0.0) return res;
			if (_xmax <= 0.0) return res;
			if (std::any_of(_x.begin(), _x.end(), [](double _v) { return _v < 0.0; })) return res;
			if (std::any_of(_x.begin(), _x.end(), [_xmax](double _v) { return _v > _xmax; })) return res;
			res.resize(_x.size());
			for (size_t i = 0; i < _x.size(); ++i)
				res[i] = _m / _xmax * std::pow(_x[i] / _xmax, _m - 1);
			return res;
		}
}

namespace details
{
	const SFunctionDescriptor& GetFunctionDescriptor(EDistributionFunction _type)
	{
		static const std::array<SFunctionDescriptor, size_t(EDistributionFunction::COUNT_)> functions
		{ {
			{ "Manual", {} },
			{ "Normal", {
				{ "Mean", 0.001 },
				{ "Standard deviation", 0.0001 }
			}},
			{ "Log-normal", {
				{ "Log of mean", 0.001 },
				{ "Log of scale", 0.01 }
			}},
			{ "RRSB", {
				{ "x63", 0.001 },
				{ "Modulus", 3.0 }
			}},
			{ "GGS", {
				{ "Max size", 0.001 },
				{ "Modulus", 3.0 }
			}}
		} };

		assert(_type != EDistributionFunction::COUNT_);
		return functions[size_t(_type)];
	}
}

std::vector<double> CreateDistribution(EDistributionFunction _type, const std::vector<double>& _x, double _param1, double _param2)
{
	switch (_type)
	{
	case EDistributionFunction::MANUAL:		return std::vector(_x.size(), 0.0);
	case EDistributionFunction::NORMAL:		return ::CreateDistributionNormal(_x, _param1, _param2);
	case EDistributionFunction::LOG_NORMAL:	return ::CreateDistributionLogNormal(_x, _param1, _param2);
	case EDistributionFunction::RRSB:		return ::CreateDistributionRRSB(_x, _param1, _param2);
	case EDistributionFunction::GGS:		return ::CreateDistributionGGS(_x, _param1, _param2);
	case EDistributionFunction::COUNT_:		break;
	}

	assert(false);
	return {};
}
