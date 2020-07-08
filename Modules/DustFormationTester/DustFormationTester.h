/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>

class CDustFormationTester
{
	double eps{0.5};			// Bulk porosity of the material.
	double Xa{0.03};			// Moisture content of material sample.
	double X90{0.08};			// Moisture content of material at 90% relative humidity.
	std::vector<double> grid;	// Size classes of the PSD.
	std::vector<double> PSD;	// Particle size distributions in terms of mass fractions of the material.
	bool consistent{};			// Is set to true when all data is set properly and results can be obtained.

	/// Precalculated values.
	std::vector<double> Q3;		// Precalculated mass related cumulative PSD.
	std::vector<double> Q2;		// Precalculated surface area related cumulative PSD.
	std::vector<double> q3;		// Precalculated mass related density PSD.
	double KSD{};				// Precalculated specific calculation factor for single drop tester.
	double KRD{};				// Precalculated specific calculation factor for rotating drum tester.
	double KUNC{};				// Precalculated specific calculation factor for UNC Dustiness tester.

public:
	/// Setters.
	void SetBulkPorosity(double _porosity);			// Sets new bulk porosity of the material.
	void SetMoistureContent(double _moisture);	    // Sets new moisture content of sample.
	void SetMoistureContent90(double _moisture);	// Sets new moisture content of the material at 90% relative humidity.
	void SetGrid(const std::vector<double>& _grid);	// Sets new size grid.
	void SetPSD(const std::vector<double>& _PSD);	// Sets new particle size distributions in terms of mass fractions.

	/// Getters.
	double GetBulkPorosity() const;		 // Returns defined bulk porosity of the material.
	double GetMoistureContent() const;	 // Returns defined moisture content of sample.
	double GetMoistureContent90() const; // Returns defined moisture content of the material at 90% relative humidity.

	std::vector<double> GetAll() const;	// Returns all calculated values: {SSD20, SSD50, SSD80, SSDLV, SRDUL, SRDMV, SRDLL, SUNC}

	/// Single drop tester.
	double SSD20() const; // Predicted dust release number for single drop tester and 20% relative air humidity.
	double SSD50() const; // Predicted dust release number for single drop tester and 50% relative air humidity.
	double SSD80() const; // Predicted dust release number for single drop tester and 80% relative air humidity.
	double SSDLV() const; // Predicted dust release number for single drop tester and low vacuum.

	/// Rotating drum tester.
	double SRDUL() const; // Predicted dust release number for rotating drum tester, upper limit.
	double SRDMV() const; // Predicted dust release number for rotating drum tester, average value.
	double SRDLL() const; // Predicted dust release number for rotating drum tester, lower limit.
	double SRDWB() const; // Predicted dust release number for rotating drum tester, wet bulk material.

	/// UNC Dustiness tester.
	double SUNC() const; // Predicted dust release number for UNC Dustiness tester.

private:
	void PrecalculateAll();				// Precalculates all required values.
	void PrecalculateDistributions();	// Precalculates distributions.
	void PrecalculateKSD();				// Specific calculation factor for single drop tester.
	void PrecalculateKRD();				// Specific calculation factor for rotating drum tester.
	void PrecalculateKUNC();			// Specific calculation factor for UNC Dustiness tester.

	std::vector<double> Narrowed_q3(double _min) const; // Returns q3 with particles all larger as _min.

	bool IsConsistent();	// Checks if all data is set properly and results can be obtained.
};

