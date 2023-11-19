/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <string>
#include <vector>
#include <limits>
#include <memory>

class CH5Handler;

////////////////////////////////////////////////////////////////////////////////
// CStateVariable
//

/**
 * \brief Description of time-dependent state variables of the unit.
 * \details Used either to track internal time-dependent variables of the unit or to output it to the user.
 */
class CStateVariable
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	inline static const double m_eps{ 16 * std::numeric_limits<double>::epsilon() };

	std::string m_name;					// Name of the state variable.
	double m_value;						// Current value of a time-dependent state variable.
	double m_valueStored;				// Memory for temporary storage of the state variable for cyclic recalculations.

	std::vector<STDValue> m_history;	// Stored history of time-dependent changes.

public:
	/**
	 * \private
	 * \param _name Name of the state variable.
	 * \param _initValue Initial value of the state variable.
	 */
	CStateVariable(std::string _name, double _initValue);

	/**
	 * \private
	 * \brief Clears state variable.
	 */
	void Clear();

	/**
	 * \brief Returns name of the state variable.
	 * \return Name of the state variable.
	 */
	std::string GetName() const;
	/**
	 * \private
	 * \brief Sets name of the state variable.
	 * \param _name Name of the state variable.
	 */
	void SetName(const std::string& _name);

	/**
	 * \brief Returns current value of the state variable.
	 * \return Current value of the state variable.
	 */
	double GetValue() const;
	/**
	 * \private
	 * \brief Sets new value of the state variable.
	 * \param _value New value.
	 */
	void SetValue(double _value);
	/**
	 * \private
	 * \brief Sets new value of the state variable and saves it in the history of changes.
	 * \param _time Time point.
	 * \param _value New value.
	 */
	void SetValue(double _time, double _value);

	/**
	 * \private
	 * \brief Stores current value of the state variable.
	 */
	void SaveState();
	/**
	 * \private
	 * \brief Restores previously stored value of the state variable.
	 */
	void LoadState();

	/**
	 * \brief Checks whether the state variable contains a stored history of time-dependent changes.
	 * \return Whether the state variable stores history.
	 */
	bool HasHistory() const;
	/**
	 * \brief Returns the stored history of time-dependent changes.
	 * \return History of time-dependent changes.
	 */
	std::vector<STDValue> GetHistory() const;
	/**
	 * \brief Returns a value for a given time point from the stored history.
	 * \details Interpolates the value if it is required.
	 * \param _time Time point.
	 * \return Value at the given time point.
	 */
	double GetHistoryValue(double _time) const;
	/**
	 * \private
	 * \brief Sets the history of time-dependent changes.
	 * \param _history New history.
	 */
	void SetHistory(const std::vector<STDValue>& _history);

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
private:
	void AddToHistory(double _time, double _value);	// Adds the given value to the history and removes all data after the given time.
};

////////////////////////////////////////////////////////////////////////////////
// CStateVariablesManager
//

/**
* \brief Manager of state variables.
*/
class CStateVariablesManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CStateVariable>> m_stateVariables;	// Defined state variables.

public:
	/**
	 * \private
	 * \brief Copies user-defined data from \p _stateVariables.
	 * \details Copies information about state variables. Assumes the corresponding state variables structure is the same.
	 * \param _stateVariables Reference to state variables manager.
	 */
	void CopyUserData(const CStateVariablesManager& _stateVariables) const;

	/**
	 * \private
	 * \brief Adds a new time-dependent state variable and returns a pointer to it.
	 * \details If a state variable with this name already exists, does nothing and returns nullptr.
	 * \param _name Name of the state variable.
	 * \param _initValue Initial value of the state variable.
	 * \return Pointer to added state variable.
	 */
	CStateVariable* AddStateVariable(const std::string& _name, double _initValue);
	/**
	 * \brief Returns a state variable with the specified index.
	 * \param _index Index of the state variable.
	 * \return Const pointer to state variable.
	 */
	const CStateVariable* GetStateVariable(size_t _index) const;
	/**
	 * \brief Returns a state variable with the specified index.
	 * \param _index Index of the state variable.
	 * \return Pointer to state variable.
	 */
	CStateVariable* GetStateVariable(size_t _index);
	/**
	 * \brief Returns a state variable with the specified name.
	 * \param _name Name of the state variable.
	 * \return Const pointer to state variable.
	 */
	const CStateVariable* GetStateVariable(const std::string& _name) const;
	/**
	 * \brief Returns a state variable with the specified name.
	 * \param _name Name of the state variable.
	 * \return Pointer to state variable.
	 */
	CStateVariable* GetStateVariable(const std::string& _name);

	/**
	 * \brief Returns const pointers to all defined state variables.
	 * \return Const pointers to all state variables.
	 */
	std::vector<const CStateVariable*> GetAllStateVariables() const;
	/**
	 * \brief Returns const pointers to all defined state variables.
	 * \return Pointers to all state variables.
	 */
	std::vector<CStateVariable*> GetAllStateVariables();

	/**
	 * \private
	 * \brief Returns const pointers to all defined state variables that track history of changes.
	 * \return Const pointers to all state variables with history.
	 */
	std::vector<const CStateVariable*> GetAllStateVariablesWithHistory() const;
	/**
	 * \private
	 * \brief Returns const pointers to all defined state variables that track history of changes.
	 * \return Pointers to all state variables with history.
	 */
	std::vector<CStateVariable*> GetAllStateVariablesWithHistory();

	/**
	 * \brief Returns number of defined state variables.
	 * \return Number of state variables.
	 */
	size_t GetStateVariablesNumber() const;

	/**
	 * \private
	 * \brief Clears all data in all state variables.
	 */
	void ClearData();
	/**
	 * \private
	 * \brief Removes all defined state variables.
	 */
	void Clear();

	/**
	 * \private
	 * \brief Stores the current state of all state variables.
	 */
	void SaveState();
	/**
	 * \private
	 * \brief Restores previously stored state of all state variables.
	 */
	void LoadState();

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file. A compatibility version.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
};