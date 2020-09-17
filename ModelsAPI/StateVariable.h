/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <string>
#include <vector>

// Description of time-dependent state variables.
class CStateVariable
{
	inline static const double m_eps{ 16 * std::numeric_limits<double>::epsilon() };

public:
	std::string m_name;					// Name of the state variable.
	double m_value;						// Current value of a time-dependent state variable.

	// TODO: remove variable
	bool m_trackHistory;				// Whether to track and store the whole history of time-dependent changes.
	const double m_initValue;			// The value specified by the user as the initial value.
	std::vector<STDValue> m_data;		// Stored the history of time-dependent changes.

private:
	double m_valueStored;				// Memory for temporary storage of the state variable for cyclic recalculations.

public:
	CStateVariable(std::string _name, double _initValue, bool _trackHistory = true);

	void Initialize();							// Initializes the state variable before starting the simulation.
	void Clear();								// Clears state variable.

	std::string GetName() const;				// Returns name of the state variable.
	void SetName(const std::string& _name);		// Sets name of the state variable.

	double GetValue() const;					// Returns current value of the state variable
	void SetValue(double _value);				// Sets new value of the state variable.
	void SetValue(double _time, double _value);	// Sets new value of the state variable and saves it in the history of changes.

	bool IsTrackHistory() const;				// Returns whether the history of time-dependent changes is tracked.
	void SetTrackHistory(bool _trackHistory);	// Sets whether to track the history of time-dependent changes.

	void SaveState();							// Stores current value of the state variable.
	void LoadState();							// Restores previously stored value of the state variable.

	bool HasHistory() const;					// Checks whether the state variable contains a stored history of time-dependent changes.
	std::vector<STDValue> GetHistory() const;	// Returns the stored history of time-dependent changes.

private:
	void AddToHistory(double _time, double _value);	// Adds the given value to the history and removes all data after the given time.
};

