/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DustFormationTester.h"
#include "PSDFunctions.h"
#include <numeric>
#include <algorithm>

void CDustFormationTester::SetBulkPorosity(double _porosity)
{
	if (eps == _porosity) return;
	eps = _porosity;
	eps = std::max(0., std::min(eps, 1.));
	consistent = IsConsistent();
	PrecalculateAll();
}

void CDustFormationTester::SetMoistureContent(double _moisture)
{
	Xa = std::max(0., std::min(_moisture, 1.));
}

void CDustFormationTester::SetMoistureContent90(double _moisture)
{
	X90 = std::max(0., std::min(_moisture, 1.));
}

void CDustFormationTester::SetGrid(const std::vector<double>& _grid)
{
	if (grid == _grid) return;
	grid = _grid;
	consistent = IsConsistent();
	PrecalculateAll();
}

void CDustFormationTester::SetPSD(const std::vector<double>& _PSD)
{
	if (PSD == _PSD) return;
	PSD = _PSD;
	consistent = IsConsistent();
	PrecalculateAll();
}

double CDustFormationTester::GetBulkPorosity() const
{
	return eps;
}

double CDustFormationTester::GetMoistureContent() const
{
	return Xa;
}

double CDustFormationTester::GetMoistureContent90() const
{
	return X90;
}

std::vector<double> CDustFormationTester::GetAll() const
{
	return { SSD20(), SSD50(), SSD80(), SSDLV(), SRDUL(), SRDMV(), SRDLL(), SRDWB(), SUNC() };
}

double CDustFormationTester::SSD20() const
{
	if (!consistent) return 0;
	const double EMAX = -14.320 + 30.950 * std::log10(KSD);
	const double E30  = -11.802 + 21.491 * std::log10(KSD);
	return EMAX + E30;
}

double CDustFormationTester::SSD50() const
{
	if (!consistent) return 0;
	const double EMAX = -19.19 + 29.14 * std::log10(KSD);
	const double E30  = -14.10 + 17.79 * std::log10(KSD);
	return EMAX + E30;
}

double CDustFormationTester::SSD80() const
{
	if (!consistent) return 0;
	const double EMAX = -20.00 + 29.30 * std::log10(KSD);
	const double E30  = -14.04 + 17.33 * std::log10(KSD);
	return EMAX + E30;
}

double CDustFormationTester::SSDLV() const
{
	if (!consistent) return 0;
	const double EMAX = -14.76 + 24.71 * std::log10(KSD);
	const double E30  =  -8.88 + 14.91 * std::log10(KSD);
	return EMAX + E30;
}

double CDustFormationTester::SRDUL() const
{
	if (!consistent) return 0;
	return std::pow(10, 3.2357 - 0.69102 * KRD);
}

double CDustFormationTester::SRDMV() const
{
	if (!consistent) return 0;
	return std::pow(10, 3.1083 - 0.66975 * KRD);
}

double CDustFormationTester::SRDLL() const
{
	if (!consistent) return 0;
	return std::pow(10, 2.918 - 0.62301 * KRD);
}

double CDustFormationTester::SRDWB() const
{
	if (!consistent) return 0;
	return std::pow(10, std::pow(2.21 * KRD * Xa / eps, -0.14)) * X90;
}

double CDustFormationTester::SUNC() const
{
	if (!consistent) return 0;
	return std::pow(10, 0.93058 * std::exp(-0.15031 * KUNC));
}

void CDustFormationTester::PrecalculateAll()
{
	if (!consistent) return;
	PrecalculateDistributions();
	PrecalculateKSD();
	PrecalculateKRD();
	PrecalculateKUNC();
}

void CDustFormationTester::PrecalculateDistributions()
{
	if (!consistent) return;
	Q3 = ConvertMassFractionsToQ3(PSD);
	Q2 = ConvertMassFractionsToQ2(grid, PSD);
	q3 = ConvertMassFractionsToq3(grid, PSD);
}

void CDustFormationTester::PrecalculateKSD()
{
	if (!consistent) return;
	const double d32 = GetSauterDiameter(grid, q3);
	const double x50 = GetDistributionValue(grid, Q2, 0.5);
	const double x10 = GetDistributionValue(grid, Q2, 0.1);
	KSD = 2. / 3. * eps / (1. - eps) * d32 * (x50 - x10) / x10 * 1e+6;
}

void CDustFormationTester::PrecalculateKRD()
{
	if (!consistent) return;
	const std::vector<double> q3Narrow = Narrowed_q3(2.9e-6);
	const double d32GP = GetSauterDiameter(grid, q3Narrow);
	const double d32 = GetSauterDiameter(grid, q3);
	const double x90 = GetDistributionValue(grid, Q3, 0.9);
	const double x50 = GetDistributionValue(grid, Q3, 0.5);
	const double x10 = GetDistributionValue(grid, Q3, 0.1);
	KRD = (d32GP / d32) / ((x90 - x10) / x50) / eps;
}

void CDustFormationTester::PrecalculateKUNC()
{
	if (!consistent) return;
	const std::vector<double> q3Narrow = Narrowed_q3(8.8e-6);
	const double d32GP = GetSauterDiameter(grid, q3Narrow);
	const double d32 = GetSauterDiameter(grid, q3);
	const double x90 = GetDistributionValue(grid, Q3, 0.9);
	const double x50 = GetDistributionValue(grid, Q3, 0.5);
	const double x10 = GetDistributionValue(grid, Q3, 0.1);
	KUNC = (d32GP / d32) / ((x90 - x10) / x50) / (2. / 3. * eps / (1. - eps) * d32) * 1e-6;
}

std::vector<double> CDustFormationTester::Narrowed_q3(double _min) const
{
	if (!consistent) return {};
	std::vector<double> q3New = q3;
	for (size_t i = 0; i < grid.size() - 1; ++i)
		if ((grid[i] + grid[i + 1]) / 2 <= _min)
			q3New[i] = 0;
		else
			break;
	NormalizeDensityDistribution(grid, q3New);
	return q3New;
}

bool CDustFormationTester::IsConsistent()
{
	if (PSD.empty()) return false;
	if (grid.size() != PSD.size() + 1) return false;
	if (std::accumulate(PSD.begin(), PSD.end(), 0.) == 0) return false;
	return true;
}
