/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MDMatrix.h"
#include "TimeDependentValue.h"

class CDistributionsGrid;

class CPhase
{
	std::string m_name{ "Phase" };			// Name.
	EPhase m_state{ EPhase::UNDEFINED };	// Aggregation state.
	CTimeDependentValue m_fractions;		// Mass fraction for each time point.
	CMDMatrix m_distribution;				// Multidimensional distributed parameters.

	std::vector<std::string> m_compounds;	// Keys of chemical compounds described in this phase.
	const CDistributionsGrid& m_grid;		// Pointer to a distribution grid.

public:
	CPhase(EPhase _state, const CDistributionsGrid& _grid, std::vector<std::string> _compounds, const SCacheSettings& _cache);

	// Returns name of the stream.
	std::string GetName() const;
	// Sets new name of the stream.
	void SetName(const std::string& _name);

	// RTeturns aggregation state of the phase.
	EPhase GetState() const;

	// Adds a new temp point _time if it doesn't already exist.
	// TODO: maybe remove.
	void AddTimePoint(double _time);
	// Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	void CopyTimePoint(double _timeDst, double _timeSrc);
	// Removes all existing time points in the specified interval.
	void RemoveTimePoints(double _timeBeg, double _timeEnd);

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
	void SetCompoundFraction(double _time, size_t _iCompound, double _fraction);

	// Sets new caching parameters.
	void SetCacheParameters(const SCacheSettings& _cache);

	// Returns a pointer to a multidimensional distribution.
	CMDMatrix* MDDistr();
	// Returns a const pointer to a multidimensional distribution.
	const CMDMatrix* MDDistr() const;

private:
	// Checks whether the specified compound is defined in the stream.
	bool HasCompound(const std::string& _compoundKey) const;
	// Returns index of the compound.
	size_t CompoundIndex(const std::string& _compoundKey) const;
};


