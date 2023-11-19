/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DependentValues.h"
#include "BaseSolver.h"
#include "H5Handler.h"
#include "ChemicalReaction.h"

enum class EUnitParameter
{
	UNKNOWN               = 0,
	TIME_DEPENDENT        = 1,
	CONSTANT              = 2,
	STRING                = 3,
	CHECKBOX              = 4,
	SOLVER                = 5,
	COMBO                 = 6,
	GROUP                 = 7,
	COMPOUND              = 8,
	CONSTANT_DOUBLE       = 9,
	CONSTANT_INT64        = 10,
	CONSTANT_UINT64       = 11,
	REACTION			  = 12,
	LIST_DOUBLE           = 13,
	LIST_INT64            = 14,
	LIST_UINT64           = 15,
	MDB_COMPOUND          = 16,
	PARAM_DEPENDENT       = 17,
};

// TODO: remove
#define UP_MIN (-std::numeric_limits<double>::max())
#define UP_MAX  (std::numeric_limits<double>::max())


/**
 * \brief Base class for unit parameters.
 */
class CBaseUnitParameter
{
	EUnitParameter m_type;     ///< Type of unit parameter.
	std::string m_name;        ///< Parameter name.
	std::string m_units;       ///< Units of measurement.
	std::string m_description; ///< Description of the parameter.

public:
	/**
	 * \private
	 */
	CBaseUnitParameter();
	/**
	 * \private
	 */
	explicit CBaseUnitParameter(EUnitParameter _type);
	/**
	 * \private
	 */
	CBaseUnitParameter(EUnitParameter _type, std::string _name, std::string _units, std::string _description);
	/**
	 * \private
	 */
	virtual ~CBaseUnitParameter() = default;

	/**
	 * \private
	 */
	CBaseUnitParameter(const CBaseUnitParameter& _other)            = default;
	/**
	 * \private
	 */
	CBaseUnitParameter(CBaseUnitParameter&& _other)                 = default;
	/**
	 * \private
	 */
	CBaseUnitParameter& operator=(const CBaseUnitParameter& _other) = default;
	/**
	 * \private
	 */
	CBaseUnitParameter& operator=(CBaseUnitParameter&& _other)      = default;

	/**
	 * \private
	 * \brief Clears all data.
	 */
	virtual void Clear() = 0;

	/**
	 * \brief Returns unit parameter type.
	 * \return Unit parameter type
	 */
	EUnitParameter GetType() const;
	/**
	* \brief Returns unit parameter name.
	* \return Parameter name.
	*/
	std::string GetName() const;
	/**
	 * \brief Returns parameter units.
	 * \return Parameter units.
	 */
	std::string GetUnits() const;
	/**
	 * \brief Returns parameter description.
	 * \return Parameter description.
	 */
	std::string GetDescription() const;

	/**
	 * \private
	 * \brief Sets parameter type.
	 * \param _type Parameter type.
	 */
	void SetType(EUnitParameter _type);
	/**
	 * \private
	 * \brief Sets parameter name.
	 * \param _name Parameter name.
	 */
	void SetName(const std::string& _name);
	/**
	 * \private
	 * \brief Sets parameter units.
	 * \param _units Parameter units.
	 */
	void SetUnits(const std::string& _units);
	/**
	 * \private
	 * \brief Sets parameter description.
	 * \param _description Parameter description.
	 */
	void SetDescription(const std::string& _description);

	/**
	 * \brief Checks whether all values lay in allowed range.
	 * \return Whether all values lay in allowed range.
	 */
	virtual bool IsInBounds() const;

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	virtual std::ostream& ValueToStream(std::ostream& _s) = 0;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	virtual std::istream& ValueFromStream(std::istream& _s) = 0;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	//virtual void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) = 0;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	//virtual void LoadFromFile(CH5Handler& _h5Loader, const std::string& _path) = 0;
};


/**
 * \brief Basic class for constant unit parameters.
 */
template<typename T>
class CConstUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion{ 1 };

	T m_value{}; ///< Const value.
	T m_min{};   ///< Minimum allowed value.
	T m_max{};   ///< Maximum allowed value.

public:
	/**
	 * \private
	 */
	CConstUnitParameter();
	/**
	 * \private
	 */
	CConstUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, T _value);

	/**
	 * \private
	 * \brief Sets value to zero.
	 */
	void Clear() override;

	/**
	 * \brief Returns constant unit parameter value.
	 * \return Constant unit parameter value.
	 */
	T GetValue() const { return m_value; }
	/**
	 * \brief Returns minimum allowed value.
	 * \return Minimum allowed value.
	 */
	T GetMin() const{ return m_min; }
	/**
	 * \brief Returns maximum allowed value.
	 * \return Maximum allowed value.
	 */
	T GetMax() const{ return m_max; }

	/**
	 * \private
	 * \brief Sets constant unit parameter value.
	 * \param _value Constant unit parameter value.
	 */
	void SetValue(T _value) { m_value = _value; }
	/**
	 * \private
	 * \brief Sets minimum allowed value.
	 * \param _min Minimum allowed value.
	 */
	void SetMin(T _min){ m_min = _min; }
	/**
	 * \private
	 * \brief Sets maximum allowed value.
	 * \param _max Maximum allowed value.
	 */
	void SetMax(T _max){ m_max = _max; }

	/**
	 * \brief Checks whether m_value lays in range [m_min; m_max].
	 * \return Whether m_value lays in the allowed range.
	 */
	bool IsInBounds() const override;

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

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
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};

/**
 * \brief Specialization of the class CConstUnitParameter for real constant unit parameters.
 */
using CConstRealUnitParameter = CConstUnitParameter<double>;
/**
 * \brief Specialization of the class CConstUnitParameter for signed integer constant unit parameters.
 */
using CConstIntUnitParameter = CConstUnitParameter<int64_t>;
/**
 * \brief Specialization of the class CConstUnitParameter for unsigned integer constant unit parameters.
 */
using CConstUIntUnitParameter = CConstUnitParameter<uint64_t>;


/**
 * \brief Base class for constant list unit parameters.
 */
template<typename T>
class CListUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_saveVersion{ 0 };

	std::vector<T> m_values{}; ///< Const values.
	T m_min{};                 ///< Minimum allowed value.
	T m_max{};                 ///< Maximum allowed value.

public:
	/**
	 * \private
	 */
	CListUnitParameter();
	/**
	 * \private
	 */
	CListUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, std::vector<T> _values);

	/**
	 * \private
	 * \brief Clears all data.
	 */
	void Clear() override { m_values.clear(); }

	/**
	 * \brief Returns value at the given index of the list.
	 * \details Returns T{} if no value is defined for the given index.
	 * \param _index Index of the value.
	 * \return Value at the given index.
	 */
	[[nodiscard]] T GetValue(size_t _index) const { if (_index >= m_values.size()) return T{}; return m_values[_index]; }
	/**
	 * \private
	 * \brief Adds new value to the list.
	 * \param _value New value.
	 */
	void AddValue(T _value) { m_values.push_back(_value); }
	/**
	 * \private
	 * \brief Sets new value at the given index of the list if it exists.
	 * \details If the value at the given index does not exist, nothing is done.
	 * \param _index Index of the value.
	 * \param _value New value.
	 */
	void SetValue(size_t _index, T _value) { if (_index < m_values.size()) m_values[_index] = _value; }
	/**
	 * \private
	 * \brief Removes value at the given index of the list if it exists.
	 * \details If the value at the given index does not exist, nothing is done.
	 * \param _index Index of the value.
	 */
	void RemoveValue(size_t _index)	{ if (_index < m_values.size()) m_values.erase(m_values.begin() + _index); }
	/**
	 * \brief Returns all defined values.
	 * \return All defined values.
	 */
	[[nodiscard]] std::vector<T> GetValues() const { return m_values; }
	/**
	 * \private
	 * \brief Sets new list of values.
	 * \param _values New list of values.
	 */
	void SetValues(const std::vector<T>& _values) { m_values = _values; }

	/**
	 * \brief Returns minimum allowed value.
	 * \return Minimum allowed value.
	 */
	[[nodiscard]] T GetMin() const { return m_min; }
	/**
	 * \brief Returns maximum allowed value.
	 * \return Maximum allowed value.
	 */
	[[nodiscard]] T GetMax() const { return m_max; }

	/**
	 * \private
	 * \brief Sets minimum allowed value.
	 * \param _min Minimum allowed value.
	 */
	void SetMin(T _min) { m_min = _min; }
	/**
	 * \private
	 * \brief Sets maximum allowed value.
	 * \param _max Maximum allowed value.
	 */
	void SetMax(T _max) { m_max = _max; }

	/**
	 * \brief Returns number of defined values.
	 * \return Number of defined values.
	 */
	[[nodiscard]] size_t Size() const { return m_values.size(); }
	/**
	 * \brief Checks if any value is defined in the list.
	 * \return Whether any value is defined.
	 */
	[[nodiscard]] bool IsEmpty() const { return m_values.empty(); }
	/**
	 * \brief Checks if all values lay in range [min; max].
	 * \return Whether all value lay in the allowed interval.
	 */
	[[nodiscard]] bool IsInBounds() const override { return std::all_of(m_values.begin(), m_values.end(), [&](const auto val) { return val >= m_min && val <= m_max; }); }

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

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
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};

/**
 * \brief Specialization of the class CListUnitParameter for real constant unit parameters.
 */
using CListRealUnitParameter = CListUnitParameter<double>;
/**
 * \brief Specialization of the class CListUnitParameter for signed integer constant unit parameters.
 */
using CListIntUnitParameter  = CListUnitParameter<int64_t>;
/**
 * \brief Specialization of the class CListUnitParameter for unsigned integer constant unit parameters.
 */
using CListUIntUnitParameter = CListUnitParameter<uint64_t>;


/**
 * \brief Class for dependent unit parameters.
 */
class CDependentUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion{ 2 };

	CDependentValues m_data;  ///< Dependent values.
	double m_valueMin{};      ///< Minimum allowed value.
	double m_valueMax{};      ///< Maximum allowed value.
	double m_paramMin{};      ///< Minimum allowed parameter.
	double m_paramMax{};      ///< Maximum allowed parameter.
	std::string m_paramName;  ///< Parameter name.
	std::string m_paramUnits; ///< Parameter units.

public:
	/**
	 * \private
	 */
	CDependentUnitParameter();
	/**
	 * \private
	 */
	CDependentUnitParameter(std::string _valueName, double _valueInit, std::string _valueUnits, std::string _paramName, double _paramInit, std::string _paramUnits, std::string _description, double _valueMin, double _valueMax, double _paramMin, double _paramMax);

	/**
	 * \private
	 * \brief Removes all values
	 */
	void Clear() override;

	/**
	 * \brief Returns dependent parameter name.
	 * \return Dependent parameter name.
	 */
	std::string GetParamName() const;
	/**
	 * \private
	 * \brief Sets dependent parameter name.
	 * \param _paramName Dependent parameter name.
	 */
	void SetParamName(const std::string& _paramName);

	/**
	 * \brief Returns dependent parameter units.
	 * \return Dependent parameter units.
	 */
	std::string GetParamUnits() const;
	/**
	 * \private
	 * \brief Sets dependent parameter units.
	 * \param _paramUnits Dependent parameter units.
	 */
	void SetParamUnits(const std::string& _paramUnits);

	/**
	 * \brief Returns minimum allowed value.
	 * \return Minimum allowed value.
	 */
	double GetValueMin() const;
	/**
	 * \brief Returns maximum allowed value.
	 * \return Maximum allowed value.
	 */
	double GetValueMax() const;

	/**
	 * \private
	 * \brief Sets minimum allowed value.
	 * \param _valueMin Minimum allowed value.
	 */
	void SetValueMin(double _valueMin);
	/**
	 * \private
	 * \brief Sets maximum allowed value.
	 * \param _valueMax Maximum allowed value.
	 */
	void SetValueMax(double _valueMax);

	/**
	 * \brief Returns minimum allowed dependent parameter.
	 * \return Minimum allowed dependent parameter.
	 */
	double GetParamMin() const;
	/**
	 * \brief Returns maximum allowed dependent parameter.
	 * \return Maximum allowed dependent parameter.
	 */
	double GetParamMax() const;

	/**
	 * \private
	 * \brief Sets minimum allowed dependent parameter.
	 * \param _paramMin Minimum allowed dependent parameter.
	 */
	void SetParamMin(double _paramMin);
	/**
	 * \private
	 * \brief Sets maximum allowed dependent parameter.
	 * \param _paramMax Maximum allowed dependent parameter.
	 */
	void SetParamMax(double _paramMax);

	/**
	 * \brief Returns unit parameter value at given dependent parameter.
	 * \details Applies data interpolation if necessary.
	 * \param _param Dependent parameter.
	 * \return Value at current dependent parameter.
	 */
	double GetValue(double _param) const;
	/**
	 * \private
	 * \brief Sets new unit parameter value at given dependent parameter.
	 * \details If the given dependent parameter already exists, changes its value. If the given dependent parameter does not yet exists, adds a new parameter-value pair.
	 * \param _param Dependent parameter.
	 * \param _value Value at current dependent parameter.
	 */
	void SetValue(double _param, double _value);
	/**
	 * \private
	 * \brief Removes unit parameter value at given dependent parameter.
	 * \details If the value at the given parameter does not exist, nothing is done.
	 * \param _param Dependent parameter.
	 */
	void RemoveValue(double _param);

	/**
	 * \brief Returns list of all defined dependent parameters.
	 * \return List of all defined dependent parameters.
	 */
	std::vector<double> GetParams() const;
	/**
	 * \brief Returns list of all defined values.
	 * \return List of all defined values.
	 */
	std::vector<double> GetValues() const;
	/**
	 * \private
	 * \brief Sets new lists of dependent parameters and values.
	 * \param _params List of dependent parameters.
	 * \param _values List of values.
	 */
	void SetValues(const std::vector<double>& _params, const std::vector<double>& _values);
	/**
	 * \private
	 * \brief Returns the dependent data itself.
	 * \return Reference to dependent data.
	 */
	const CDependentValues& GetDependentData() const;

	/**
	 * \brief Returns number of defined dependent values.
	 * \return Number of defined dependent values.
	 */
	size_t Size() const;
	/**
	 * \brief Checks whether any dependent value is defined.
	 * \return Whether any dependent value is defined.
	 */
	bool IsEmpty() const;
	/**
	 * \brief Checks whether all values lay in range [m_valueMin; m_valueMax] and parameters lay in range [m_paramMin; m_paramMax].
	 * \return Whether all values and all parameters lay in the allowed interval.
	 */
	bool IsInBounds() const override;
	/**
	 * \brief Checks if a specific parameter is included in data without interpolation.
	 * \param _param Parameter.
	 * \return Dependent parameter.
	 */
	bool HasParam(double _param) const;

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

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
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};

/**
 * \brief Class for time-dependent unit parameters.
 */
class CTDUnitParameter : public CDependentUnitParameter
{
	static const unsigned m_cnSaveVersion{ 2 };

public:
	/**
	 * \private
	 */
	CTDUnitParameter();
	/**
	 * \private
	 */
	CTDUnitParameter(std::string _name, std::string _units, std::string _description, double _min, double _max, double _value);

	/**
	* \brief Returns list of all defined time points.
	* \details Refer to function CDependentUnitParameter::GetParams() const.
	* \return List of all defined time points.
	*/
	std::vector<double> GetTimes() const;

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
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};


/**
 * \brief Class for string unit parameters.
 */
class CStringUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_value; ///< String parameter value.

public:
	/**
	 * \private
	 */
	CStringUnitParameter();
	/**
	 * \private
	 */
	CStringUnitParameter(std::string _name, std::string _description, std::string _value);

	/**
	 * \private
	 * \brief Resets value.
	 */
	void Clear() override;

	/**
	 * \brief Returns string unit parameter value.
	 * \return String unit parameter value.
	 */
	std::string GetValue() const;
	/**
	 * \private
	 * \brief Sets string unit parameter value.
	 * \param _value String unit parameter value.
	 */
	void SetValue(const std::string& _value);

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/**
 * \brief Class for check box unit parameters.
 */
class CCheckBoxUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	bool m_checked; ///< Check box parameter value: checked/unchecked.

public:
	/**
	 * \private
	 */
	CCheckBoxUnitParameter();
	/**
	 * \private
	 */
	CCheckBoxUnitParameter(std::string _name, std::string _description, bool _checked);

	/**
	 * \private
	 * \brief Resets value to unchecked.
	 */
	void Clear() override;

	/**
	 * \brief Returns check box unit parameter value.
	 * \return Whether checkbox is checked.
	 */
	bool IsChecked() const;
	/**
	 * \private
	 * \brief Sets check box unit parameter value.
	 * \param _checked Flag.
	 */
	void SetChecked(bool _checked);

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/**
 * \brief Class for solver unit parameters.
 */
class CSolverUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_key;                ///< Solver's key.
	ESolverTypes m_solverType;        ///< Solver's type.
	CBaseSolver* m_solver{ nullptr }; ///< Pointer to a selected solver.

public:
	/**
	 * \private
	 */
	CSolverUnitParameter();
	/**
	 * \private
	 */
	CSolverUnitParameter(std::string _name, std::string _description, ESolverTypes _type);

	/**
	 * \private
	 * \brief Resets solver's key and type.
	 */
	void Clear() override;

	/**
	 * \brief Returns solver's key.
	 * \return Solver's key.
	 */
	std::string GetKey() const;
	/**
	 * \brief Returns solver's type.
	 * \return Solver's type.
	 */
	ESolverTypes GetSolverType() const;
	/**
	 * \brief Returns pointer to a solver.
	 * \return Pointer to a solver.
	 */
	CBaseSolver* GetSolver() const;

	/**
	 * \private
	 * \brief Sets solver's key.
	 * \param _key Solver's key.
	 */
	void SetKey(const std::string& _key);
	/**
	 * \private
	 * \brief Sets solver's type.
	 * \param _type Solver's type.
	 */
	void SetSolverType(ESolverTypes _type);
	/**
	 * \private
	 * \brief Sets pointer to a solver.
	 * \param _solver Pointer to a solver.
	 */
	void SetSolver(CBaseSolver* _solver);

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/**
 * \brief Class for combobox unit parameters.
 */
class CComboUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::map<size_t, std::string> m_items; ///< List of possible items to select (value:name).
	size_t m_selected = -1;                ///< Selected item.

public:
	/**
	 * \private
	 */
	CComboUnitParameter();
	/**
	 * \private
	 */
	CComboUnitParameter(std::string _name, std::string _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames);

	/**
	 * \private
	 * \brief Resets selected key.
	 */
	void Clear() override;

	/**
	 * \brief Returns currently selected item.
	 * \return Currently selected item.
	 */
	size_t GetValue() const;
	/**
	 * \private
	 * \brief Sets new selected item.
	 * \param _item New selected item.
	 */
	void SetValue(size_t _item);

	/**
	 * \brief Returns all items.
	 * \return All items.
	 */
	std::vector<size_t> GetItems() const;
	/**
	 * \brief Returns all items' names.
	 * \return All items' names.
	 */
	std::vector<std::string> GetNames() const;
	/**
	 * \brief Returns item by its name.
	 * \param _name Name of the item.
	 * \return Item value.
	 */
	size_t GetItemByName(const std::string& _name) const;
	/**
	 * \brief Returns name of the item.
	 * \param _item Item value.
	 * \return Name of the item.
	 */
	std::string GetNameByItem(size_t _item) const;

	/**
	 * \brief Returns true if the combobox contains the given item.
	 * \param _item Item value.
	 * \return Whether combobox contains the given item.
	 */
	bool HasItem(size_t _item) const;
	/**
	 * \brief Returns true if the combobox contains an item with the given name.
	 * \param _name Name of the item.
	 * \return Whether combobox contains an item with the given name.
	 */
	bool HasName(const std::string& _name) const;
	/**
	 * \brief Checks whether the selected item is one of the allowed items.
	 * \return Whether the selected item is in allowed bounds.
	 */
	bool IsInBounds() const override;

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/**
 * \brief Class for material compound unit parameters.
 */
class CCompoundUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_key; ///< Unique key of selected compound.

public:
	/**
	 * \private
	 */
	CCompoundUnitParameter();
	/**
	 * \private
	 */
	CCompoundUnitParameter(std::string _name, std::string _description);

	/**
	 * \private
	 * \brief Resets compound's key.
	 */
	void Clear() override;

	/**
	 * \brief Returns key of currently selected compound.
	 * \return Key of currently selected compound.
	 */
	std::string GetCompound() const;
	/**
	 * \private
	 * \brief Sets new compound's key.
	 * \param _key Compound's key.
	 */
	void SetCompound(const std::string& _key);

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};

/**
 * \brief Class for material compound from MDB unit parameters.
 */
class CMDBCompoundUnitParameter : public CCompoundUnitParameter
{
	static const unsigned m_cnSaveVersion;

public:
	/**
	 * \private
	 */
	CMDBCompoundUnitParameter();
	/**
	 * \private
	 */
	CMDBCompoundUnitParameter(std::string _name, std::string _description);
};


/**
 * \brief Class for chemical reaction unit parameters.
 */
class CReactionUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::vector<CChemicalReaction> m_reactions; ///< Defined reactions.

public:
	/**
	 * \private
	 */
	CReactionUnitParameter();
	/**
	 * \private
	 */
	CReactionUnitParameter(std::string _name, std::string _description);

	/**
	 * \private
	 * \brief Clears all reactions.
	 */
	void Clear() override;

	/**
	 * \brief Returns all defined chemical reactions.
	 * \return All defined chemical reactions.
	 */
	[[nodiscard]] std::vector<CChemicalReaction> GetReactions() const;
	/**
	 * \brief Returns pointers to all defined chemical reactions.
	 * \return Pointers to all defined chemical reactions.
	 */
	std::vector<CChemicalReaction*> GetReactionsPtr();
	/**
	 * \brief Returns const pointer to reaction with the given index.
	 * \details If such reaction does not exist, returns nullptr.
	 * \param _index Index of the reaction.
	 * \return Const pointer to reaction.
	 */
	[[nodiscard]] const CChemicalReaction* GetReaction(size_t _index) const;
	/**
	 * \brief Returns pointer to reaction with the given index.
	 * \details If such reaction does not exist, returns nullptr.
	 * \param _index Index of the reaction.
	 * \return Pointer to reaction.
	 */
	CChemicalReaction* GetReaction(size_t _index);
	/**
	 * \brief Returns the number of defined reactions.
	 * \return Number of defined reactions.
	 */
	[[nodiscard]] size_t GetReactionsNumber() const;

	/**
	 * \private
	 * \brief Adds new empty reaction.
	 */
	void AddReaction();
	/**
	 * \private
	 * \brief Adds new reaction.
	 * \param _reaction Reaction.
	 */
	void AddReaction(const CChemicalReaction& _reaction);
	/**
	 * \private
	 * \brief Sets new reactions replacing existing.
	 * \param _reactions New reactions.
	 */
	void SetReactions(const std::vector<CChemicalReaction>& _reactions);

	/**
	 * \private
	 * \brief Removes the selected reaction.
	 * \param _index Index of the reaction.
	 */
	void RemoveReaction(size_t _index);

	/**
	 * \private
	 * \brief Outputs user-specified values of the parameter to a stream.
	 * \param _s Output stream.
	 * \return Output stream.
	 */
	std::ostream& ValueToStream(std::ostream& _s) override;
	/**
	 * \private
	 * \brief Reads user-specified values of the parameter from a stream.
	 * \param _s Input stream.
	 * \return Input stream.
	 */
	std::istream& ValueFromStream(std::istream& _s) override;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/**
 * \brief Manager of unit parameters for each unit.
 * \details Each parameter may be a member of one or several groups, to allow showing / hiding of some parameters in GUI.
 * Block is defined by a single CComboUnitParameter and may have several options to choose.
 * Each option is a group of one or several parameters, which should be shown / hidden together, depending on the selection of corresponding CComboUnitParameter.
 * One parameter can belong to several groups and several blocks.
 * Block is stored as an index of a CComboUnitParameter. Group is stored as indices of parameters, which belong to this group.
 */
class CUnitParametersManager
{
	static const unsigned m_cnSaveVersion{ 1 };

	using group_map_t = std::map<size_t, std::map<size_t, std::vector<size_t>>>; ///< map<iParameter, map<iBlock, vector<iGroups>>>

	std::vector<std::unique_ptr<CBaseUnitParameter>> m_parameters; ///< All parameters.
	group_map_t m_groups;                                          ///< List of group blocks and corresponding groups, to which parameters belong. Is used to determine activity of parameters.

public:
	/**
	 * \private
	 * \brief Copies user-defined data from _other.
	 * \details Copies information about selected parameters. Assumes the corresponding parameters structure is the same.
	 * \param _other Reference to source unit parameters manager.
	 */
	void CopyUserData(const CUnitParametersManager& _other);

	/**
	 * \brief Returns number of specified unit parameters.
	 * \return Number of parameters defined in the unit.
	 */
	size_t ParametersNumber() const;
	/**
	 * \brief Checks whether unit parameter with given name exists.
	 * \param _name Name of unit parameter.
	 * \return Whether unit parameter with given name exists.
	 */
	bool IsNameExist(const std::string& _name) const;

	/**
	 * \private
	 * \brief Adds new real constant unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddConstRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value);
	/**
	 * \private
	 * \brief Adds new signed integer constant unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddConstIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value);
	/**
	 * \private
	 * \brief Adds new unsigned integer constant unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddConstUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value);
	/**
	 * \private
	 * \brief Adds new dependent unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddDependentParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value, const std::string& _paramName, const std::string& _paramUnits, double _paramMin, double _paramMax, double _paramValue);
	/**
	 * \private
	 * \brief Adds new time-dependent unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddTDParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value);
	/**
	 * \private
	 * \brief Adds new string unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddStringParameter(const std::string& _name, const std::string& _description, const std::string& _value);
	/**
	 * \private
	 * \brief Adds new check box unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddCheckBoxParameter(const std::string& _name, const std::string& _description, bool _value);
	/**
	 * \private
	 * \brief Adds new solver unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddSolverParameter(const std::string& _name, const std::string& _description, ESolverTypes _type);
	/**
	 * \private
	 * \brief Adds new combo unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddComboParameter(const std::string& _name, const std::string& _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames);
	/**
	 * \private
	 * \brief Adds new compound unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddCompoundParameter(const std::string& _name, const std::string& _description);
	/**
	 * \private
	 * \brief Adds new MDB compound unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddMDBCompoundParameter(const std::string& _name, const std::string& _description);
	/**
	 * \private
	 * \brief Adds new reaction unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddReactionParameter(const std::string& _name, const std::string& _description);
	/**
	 * \private
	 * \brief Adds new real list unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddListRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, const std::vector<double>& _values);
	/**
	 * \private
	 * \brief Adds new signed integer list unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddListIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, const std::vector<int64_t>& _values);
	/**
	 * \private
	 * \brief Adds new unsigned integer list unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddListUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, const std::vector<uint64_t>& _values);

	/**
	 * \brief Returns list of all defined unit parameters.
	 * \return List of all defined unit parameters.
	 */
	std::vector<CBaseUnitParameter*> GetParameters() const;

	/**
	 * \brief Returns const pointer to the unit parameter with the specified index.
	 * \param _index Index of the unit parameter.
	 * \return Const pointer to the unit parameter.
	 */
	const CBaseUnitParameter* GetParameter(size_t _index) const;
	/**
	 * \brief Returns const pointer to the unit parameter with the specified index.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CBaseUnitParameter* GetParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the unit parameter with the specified name.
	 * \param _name Name of the unit parameter.
	 * \return Const pointer to the unit parameter.
	 */
	const CBaseUnitParameter* GetParameter(const std::string& _name) const;
	/**
	 * \brief Returns const pointer to the unit parameter with the specified name.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CBaseUnitParameter* GetParameter(const std::string& _name);

	/**
	 * \brief Returns const pointer to the real constant unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstRealUnitParameter* GetConstRealParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the constant real unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstRealUnitParameter* GetConstRealParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the signed integer constant unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstIntUnitParameter* GetConstIntParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the constant signed integer unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstIntUnitParameter* GetConstIntParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the unsigned integer constant unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstUIntUnitParameter* GetConstUIntParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the constant unsigned integer unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstUIntUnitParameter* GetConstUIntParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the dependent unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CDependentUnitParameter* GetDependentParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the dependent unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CDependentUnitParameter* GetDependentParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the time-dependent unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CTDUnitParameter* GetTDParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the time-dependent unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CTDUnitParameter* GetTDParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the string unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CStringUnitParameter* GetStringParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the string unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CStringUnitParameter* GetStringParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the check box unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CCheckBoxUnitParameter* GetCheckboxParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the check box unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CCheckBoxUnitParameter* GetCheckboxParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the solver unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CSolverUnitParameter* GetSolverParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the solver unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CSolverUnitParameter* GetSolverParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the combo unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CComboUnitParameter* GetComboParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the combo unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CComboUnitParameter* GetComboParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the compound unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CCompoundUnitParameter* GetCompoundParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the compound unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CCompoundUnitParameter* GetCompoundParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the MDB compound unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CMDBCompoundUnitParameter* GetMDBCompoundParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the MDB compound unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CMDBCompoundUnitParameter* GetMDBCompoundParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the reaction unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CReactionUnitParameter* GetReactionParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the reaction unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CReactionUnitParameter* GetReactionParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the real list unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListRealUnitParameter* GetListRealParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the list real unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListRealUnitParameter* GetListRealParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the signed integer list unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListIntUnitParameter* GetListIntParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the signed integer list unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListIntUnitParameter* GetListIntParameter(size_t _index);
	/**
	 * \brief Returns const pointer to the unsigned integer list unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListUIntUnitParameter* GetListUIntParameter(size_t _index) const;
	/**
	 * \brief Returns pointer to the unsigned integer list unit parameter with the specified index.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListUIntUnitParameter* GetListUIntParameter(size_t _index);

	/**
	 * \brief Returns const pointer to the real constant unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstRealUnitParameter* GetConstRealParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the constant real unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstRealUnitParameter* GetConstRealParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the signed integer constant unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstIntUnitParameter* GetConstIntParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the constant signed integer unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstIntUnitParameter* GetConstIntParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the unsigned integer constant unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CConstUIntUnitParameter* GetConstUIntParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the constant unsigned integer unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CConstUIntUnitParameter* GetConstUIntParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the dependent unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CDependentUnitParameter* GetDependentParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the time-dependent unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CDependentUnitParameter* GetDependentParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the time-dependent unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CTDUnitParameter* GetTDParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the time-dependent unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CTDUnitParameter* GetTDParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the string unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CStringUnitParameter* GetStringParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the string unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CStringUnitParameter* GetStringParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the check box unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CCheckBoxUnitParameter* GetCheckboxParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the check box unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CCheckBoxUnitParameter* GetCheckboxParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the solver unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CSolverUnitParameter* GetSolverParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the solver unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CSolverUnitParameter* GetSolverParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the combo unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CComboUnitParameter* GetComboParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the combo unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CComboUnitParameter* GetComboParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the compound unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CCompoundUnitParameter* GetCompoundParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the compound unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CCompoundUnitParameter* GetCompoundParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the MDB compound unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CMDBCompoundUnitParameter* GetMDBCompoundParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the MDB compound unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CMDBCompoundUnitParameter* GetMDBCompoundParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the reaction unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CReactionUnitParameter* GetReactionParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the reaction unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CReactionUnitParameter* GetReactionParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the real list unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListRealUnitParameter* GetListRealParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the list real unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListRealUnitParameter* GetListRealParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the signed integer list unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListIntUnitParameter* GetListIntParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the list signed integer unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListIntUnitParameter* GetListIntParameter(const std::string& _name);
	/**
	 * \brief Returns const pointer to the unsigned integer list unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	const CListUIntUnitParameter* GetListUIntParameter(const std::string& _name) const;
	/**
	 * \brief Returns pointer to the list unsigned integer unit parameter with the specified _name.
	 * \details If such parameter does not exist, returns nullptr.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	CListUIntUnitParameter* GetListUIntParameter(const std::string& _name);

	/**
	 * \brief Returns value of a constant real unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a constant real parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	double GetConstRealParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a constant signed integer unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a constant signed integer parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	int64_t GetConstIntParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a constant unsigned integer unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a constant unsigned integer parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	uint64_t GetConstUIntParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a dependent unit parameter with the specified _index and _param.
	 * \details If such parameter does not exist or is not a dependent parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \param _param Dependent parameter.
	 * \return Pointer to the unit parameter.
	 */
	double GetDependentParameterValue(size_t _index, double _param) const;
	/**
	 * \brief Returns value of a TD unit parameter with the specified _index and _time.
	 * \details If such parameter does not exist or is not a TD parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \param _time Time point.
	 * \return Pointer to the unit parameter.
	 */
	double GetTDParameterValue(size_t _index, double _time) const;
	/**
	 * \brief Returns value of a string unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a string parameter, returns "".
		 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetStringParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a check box unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a check box parameter, returns false.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	bool GetCheckboxParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a solver unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a solver parameter, returns "".
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetSolverParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a combo unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a combo parameter, returns -1.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	size_t GetComboParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a compound unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a compound parameter, returns "".
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetCompoundParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a MDB compound unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a MDB compound parameter, returns "".
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetMDBCompoundParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a reaction unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a reaction parameter, returns empty vector.
	 * \param _index Index of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::vector<CChemicalReaction> GetReactionParameterValue(size_t _index) const;
	/**
	 * \brief Returns value of a list real unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a list real parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \param _valueIndex Index of the value.
	 * \return Pointer to the unit parameter.
	 */
	double GetListRealParameterValue(size_t _index, size_t _valueIndex) const;
	/**
	 * \brief Returns value of a list signed integer unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a list signed integer parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \param _valueIndex Index of the value.
	 * \return Pointer to the unit parameter.
	 */
	int64_t GetListIntParameterValue(size_t _index, size_t _valueIndex) const;
	/**
	 * \brief Returns value of a list unsigned integer unit parameter with the specified _index.
	 * \details If such parameter does not exist or is not a list unsigned integer parameter, returns 0.
	 * \param _index Index of the unit parameter.
	 * \param _valueIndex Index of the value.
	 * \return Pointer to the unit parameter.
	 */
	uint64_t GetListUIntParameterValue(size_t _index, size_t _valueIndex) const;

	/**
	 * \brief Returns value of a constant real unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a constant real parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	double GetConstRealParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a constant signed integer unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a constant signed integer parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	int64_t GetConstIntParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a constant unsigned integer unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a constant unsigned integer parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	uint64_t GetConstUIntParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a dependent unit parameter with the specified _name and _param.
	 * \details If such parameter does not exist or is not a dependent parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \param _param Dependent parameter.
	 * \return Pointer to the unit parameter.
	 */
	double GetDependentParameterValue(const std::string& _name, double _param) const;
	/**
	 * \brief Returns value of a TD unit parameter with the specified _name and _time.
	 * \details If such parameter does not exist or is not a TD parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \param _time Time point.
	 * \return Pointer to the unit parameter.
	 */
	double GetTDParameterValue(const std::string& _name, double _time) const;
	/**
	 * \brief Returns value of a string unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a string parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetStringParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a check box unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a check box parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	bool GetCheckboxParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a solver unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a solver parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetSolverParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a combo unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a combo parameter, returns -1.
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	size_t GetComboParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a compound unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a compound parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetCompoundParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a MDB compound unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a MDB compound parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::string GetMDBCompoundParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a reaction unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a reaction parameter, returns "".
	 * \param _name Name of the unit parameter.
	 * \return Pointer to the unit parameter.
	 */
	std::vector<CChemicalReaction> GetReactionParameterValue(const std::string& _name) const;
	/**
	 * \brief Returns value of a list real unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a list real parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \param _index Index of the value in the list.
	 * \return Pointer to the unit parameter.
	 */
	double GetListRealParameterValue(const std::string& _name, size_t _index) const;
	/**
	 * \brief Returns value of a list signed integer unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a list signed integer parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \param _index Index of the value in the list.
	 * \return Pointer to the unit parameter.
	 */
	int64_t GetListIntParameterValue(const std::string& _name, size_t _index) const;
	/**
	 * \brief Returns value of a list unsigned integer unit parameter with the specified _name.
	 * \details If such parameter does not exist or is not a list unsigned integer parameter, returns 0.
	 * \param _name Name of the unit parameter.
	 * \param _index Index of the value in the list.
	 * \return Pointer to the unit parameter.
	 */
	uint64_t GetListUIntParameterValue(const std::string& _name, size_t _index) const;

	/**
	 * \private
	 * \brief Returns const pointers to all specified reaction unit parameters.
	 * \return Const pointers to all reaction unit parameters.
	 */
	std::vector<const CReactionUnitParameter*> GetAllReactionParameters() const;
	/**
	 * \private
	 * \brief Returns pointers to all specified reaction unit parameters.
	 * \return Pointers to all reaction unit parameters.
	 */
	std::vector<CReactionUnitParameter*> GetAllReactionParameters();
	/**
	 * \private
	 * \brief Returns const pointers to all specified compound and MDB compound unit parameters.
	 * \return Const pointers to all compound and MDB compound unit parameters.
	 */
	std::vector<const CCompoundUnitParameter*> GetAllCompoundParameters() const;
	/**
	 * \private
	 * \brief Returns pointers to all specified compound and MDB compound unit parameters.
	 * \return Pointers to all compound and MDB compound unit parameters.
	 */
	std::vector<CCompoundUnitParameter*> GetAllCompoundParameters();

	/**
	 * \private
	 * \brief Returns const pointers to all specified solver unit parameters.
	 * \return Const pointers to all specified solver unit parameters.
	 */
	std::vector<const CSolverUnitParameter*> GetAllSolverParameters() const;
	/**
	 * \private
	 * \brief Returns pointers to all specified solver unit parameters.
	 * \return Pointers to all specified solver unit parameters.
	 */
	std::vector<CSolverUnitParameter*> GetAllSolverParameters();
	/**
	 * \brief Returns a sorted list of time points defined in all unit parameters.
	 * \return Sorted vector of time points.
	 */
	[[nodiscard]] std::vector<double> GetAllTimePoints() const;
	/**
	 * \brief Returns a sorted list of time points form the given interval defined in all unit parameters.
	 * \param _tBeg Begin of the time interval.
	 * \param _tEnd End of the time interval.
	 * \return Sorted vector of time points.
	 */
	[[nodiscard]] std::vector<double> GetAllTimePoints(double _tBeg, double _tEnd) const;

	/**
	 * \brief Adds the list of parameters by their indices to existing group of existing block.
	 * \details If block, group or some of parameters do not exist, does nothing.
	 * \param _block Index of a block of unit parameters.
	 * \param _group Index of a group of unit parameters shown/hidden together.
	 * \param _parameters Indices of unit parameters to add to the group.
	 */
	void AddParametersToGroup(size_t _block, size_t _group, const std::vector<size_t>& _parameters);
	/**
	 * \brief Adds the list of _parameters by their names to existing _group of existing _block.
	 * \details If block, group or some of parameters do not exist, does nothing.
	 * \param _block Name of a block of unit parameters.
	 * \param _group Name of a group of unit parameters shown/hidden together.
	 * \param _parameters Names of unit parameters to add to the group.
	 */
	void AddParametersToGroup(const std::string& _block, const std::string& _group, const std::vector<std::string>& _parameters);
	/**
	 * \brief Returns true if unit parameter with the specified index is selected in at least one group of any block, or if it is a not grouped parameter.
	 * \param _index Index of the unit parameter.
	 * \return Whether unit parameter is selected in one of the groups.
	 */
	bool IsParameterActive(size_t _index) const;
	/**
	 * \brief Returns true if this parameter is selected in at least one group of any block, or if it is a not grouped parameter.
	 * \param _parameter Reference to unit parameter.
	 * \return Whether unit parameter is selected in one of the groups.
	 */
	bool IsParameterActive(const CBaseUnitParameter& _parameter) const;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5Saver Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5Loader Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);

private:
	/**
	 * \brief Makes parameter with index _parameter a member of the corresponding _block and _group.
	 */
	void AddToGroup(size_t _parameter, size_t _block, size_t _group);

	/**
	 * \brief Returns index of a unit parameter with the given name.
	 */
	size_t Name2Index(const std::string& _name) const;
	/**
	 * \brief Returns indices of unit parameter with the given names.
	 */
	std::vector<size_t> Name2Index(const std::vector<std::string>& _names) const;
};
