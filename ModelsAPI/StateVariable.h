/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <string>
#include <vector>

class CH5Handler;

////////////////////////////////////////////////////////////////////////////////
// CStateVariable
//

// Description of time-dependent state variables.
class CStateVariable
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	inline static const double m_eps{ 16 * std::numeric_limits<double>::epsilon() };

	std::string m_name;					// Name of the state variable.
	double m_value;						// Current value of a time-dependent state variable.
	double m_valueStored;				// Memory for temporary storage of the state variable for cyclic recalculations.

	std::vector<STDValue> m_history;	// Stored history of time-dependent changes.

public:
	CStateVariable(std::string _name, double _initValue);

	void Clear();								// Clears state variable.

	std::string GetName() const;				// Returns name of the state variable.
	void SetName(const std::string& _name);		// Sets name of the state variable.

	double GetValue() const;					// Returns current value of the state variable
	void SetValue(double _value);				// Sets new value of the state variable.
	void SetValue(double _time, double _value);	// Sets new value of the state variable and saves it in the history of changes.

	void SaveState();							// Stores current value of the state variable.
	void LoadState();							// Restores previously stored value of the state variable.

	bool HasHistory() const;								// Checks whether the state variable contains a stored history of time-dependent changes.
	std::vector<STDValue> GetHistory() const;				// Returns the stored history of time-dependent changes.
	void SetHistory(const std::vector<STDValue>& _history);	// Sets the history of time-dependent changes.

	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;	// Saves data to file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);		// Loads data from file.
private:
	void AddToHistory(double _time, double _value);	// Adds the given value to the history and removes all data after the given time.
};

////////////////////////////////////////////////////////////////////////////////
// CStateVariablesManager
//

class CStateVariablesManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CStateVariable>> m_stateVariables;	// Defined state variables.

public:
	// Adds a new time-dependent state variable and returns a pointer to it. If a state variable with this name already exists, does nothing and returns nullptr.
	CStateVariable* AddStateVariable(const std::string& _name, double _initValue);
	// Returns a state variable with the specified name.
	const CStateVariable* GetStateVariable(const std::string& _name) const;
	// Returns a state variable with the specified name.
	CStateVariable* GetStateVariable(const std::string& _name);

	// Returns const pointers to all defined state variables.
	std::vector<const CStateVariable*> GetAllStateVariables() const;
	// Returns const pointers to all defined state variables.
	std::vector<CStateVariable*> GetAllStateVariables();

	// Returns const pointers to all defined state variables that track history of changes.
	std::vector<const CStateVariable*> GetAllStateVariablesWithHistory() const;
	// Returns const pointers to all defined state variables that track history of changes.
	std::vector<CStateVariable*> GetAllStateVariablesWithHistory();

	// Returns a number of defined state variables.
	size_t GetStateVariablesNumber() const;

	// Removes all defined state variables.
	void ClearData();
	// Removes all data.
	void Clear();

	// Stores the current state of all state variables.
	void SaveState();
	// Restores previously stored state of all state variables.
	void LoadState();

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
};