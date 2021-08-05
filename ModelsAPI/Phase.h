/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MDMatrix.h"
#include "MultidimensionalGrid.h"
#include "TimeDependentValue.h"

class CMultidimensionalGrid;

class CPhase
{
	static const uint16_t m_saveVersion{ 0 }; // Version of the saving procedure.

	CMultidimensionalGrid m_grid;			// Defined distribution grid.

	// TODO: remove it from here, leave only in flowsheet
	std::string m_name { "Phase" };			// Name.
	EPhase m_state{ EPhase::UNDEFINED };	// Aggregation state.
	CTimeDependentValue m_fractions;		// Mass fraction of this phase at each time point.
	CMDMatrix m_distribution;				// Multidimensional distributed parameters.

public:
	CPhase(EPhase _state, std::string _name, CMultidimensionalGrid _grid, const SCacheSettings& _cache);

	// Returns name of the stream.
	std::string GetName() const;
	// Sets new name of the stream.
	void SetName(const std::string& _name);

	// Returns aggregation state of the phase.
	EPhase GetState() const;

	// Adds a new temp point _time if it doesn't already exist.
	// TODO: maybe remove.
	void AddTimePoint(double _time);
	// Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	void CopyTimePoint(double _timeDst, double _timeSrc);
	// Removes all existing time points in the specified interval, including or excluding boundaries.
	void RemoveTimePoints(double _timeBeg, double _timeEnd, bool _inclusive = true);
	// Removes all existing time points.
	void RemoveAllTimePoints();

	// Adds new compound distribution to the phase.
	void AddCompound(const std::string& _compoundKey);
	// Removes the distribution of the corresponding compound from the phase.
	void RemoveCompound(const std::string& _compoundKey);

	// Returns mass fraction of the phase at the given time point.
	double GetFraction(double _time) const;
	// Sets mass fraction of the phase at the given time point.
	void SetFraction(double _time, double _value);

	// Returns mass fraction of the compound in the phase at the given time point.
	double GetCompoundFraction(double _time, size_t _iCompound) const;
	// Sets mass fraction of the compound in the phase at the given time point.
	void SetCompoundFraction(double _time, size_t _iCompound, double _value);

	// Returns mass fractions of all defined compounds in the phase at the given time point.
	std::vector<double> GetCompoundsDistribution(double _time) const;
	// Sets mass fractions of all defined compounds in the phase at the given time point.
	void SetCompoundsDistribution(double _time, const std::vector<double>& _value);

	// Copies data from another phase at the given time point.
	void CopyFrom(double _time, const CPhase& _source);
	// Copies data to the given time point from another time point of the source phase.
	void CopyFrom(double _timeDst, const CPhase& _source, double _timeSrc);
	// Copies data from another phase at the given time interval.
	void CopyFrom(double _timeBeg, double _timeEnd, const CPhase& _source);

	// Performs nearest-neighbor extrapolation of all data.
	void Extrapolate(double _timeExtra, double _time);
	// Performs linear extrapolation of all data.
	void Extrapolate(double _timeExtra, double _time1, double _time2);
	// Performs cubic spline extrapolation of all data.
	void Extrapolate(double _timeExtra, double _time1, double _time2, double _time3);

	// Returns a pointer to a multidimensional distribution.
	CMDMatrix* MDDistr();
	// Returns a const pointer to a multidimensional distribution.
	const CMDMatrix* MDDistr() const;

	// Returns a pointer to phase fractions.
	CTimeDependentValue* Fractions();
	// Returns a const pointer to phase fractions.
	const CTimeDependentValue* Fractions() const;

	// Sets new caching parameters.
	void SetCacheSettings(const SCacheSettings& _cache);

	// Sets grids of distributed parameters.
	void SetGrid(const CMultidimensionalGrid& _grid);

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

private:
	// Copies data from source distribution to destination converting the data to destination grids.
	static void CopyDistributionsWithConvert(double _srcTime, double _dstTime, const CPhase& _srcPhase, CPhase& _dstPhase);
};
