/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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
};

// TODO: remove
#define UP_MIN (-std::numeric_limits<double>::max())
#define UP_MAX  (std::numeric_limits<double>::max())


/* Base class for unit parameters. */
class CBaseUnitParameter
{
	EUnitParameter m_type;                          ///< Type of unit parameter.
	std::string m_name;                             ///< Parameter name.
	std::string m_units;							///< Units of measurement.
	std::string m_description;                      ///< Description of the parameter.

public:
	CBaseUnitParameter();
	explicit CBaseUnitParameter(EUnitParameter _type);
	CBaseUnitParameter(EUnitParameter _type, std::string _name, std::string _units, std::string _description);
	virtual ~CBaseUnitParameter() = default;

	CBaseUnitParameter(const CBaseUnitParameter& _other)            = default;
	CBaseUnitParameter(CBaseUnitParameter&& _other)                 = default;
	CBaseUnitParameter& operator=(const CBaseUnitParameter& _other) = default;
	CBaseUnitParameter& operator=(CBaseUnitParameter&& _other)      = default;

	virtual void Clear() = 0;							     ///< Clears all data.

	EUnitParameter GetType() const;                          ///< Returns parameter type.
	std::string GetName() const;                             ///< Returns parameter name.
	std::string GetUnits() const;                            ///< Returns parameter units.
	std::string GetDescription() const;                      ///< Returns parameter description.

	void SetType(EUnitParameter _type);                      ///< Sets parameter type.
	void SetName(const std::string& _name);                  ///< Sets parameter name.
	void SetUnits(const std::string& _units);                ///< Sets parameter units.
	void SetDescription(const std::string& _description);    ///< Sets parameter description.

	virtual bool IsInBounds() const;	                     ///< Checks whether all values lay in allowed range.

	// Outputs user-specified values of the parameter to a stream.
	virtual std::ostream& ValueToStream(std::ostream& _s) = 0;
	// Reads user-specified values of the parameter from a stream.
	virtual std::istream& ValueFromStream(std::istream& _s) = 0;

	//virtual void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) = 0;
	//virtual void LoadFromFile(CH5Handler& _h5Loader, const std::string& _path) = 0;
};


// Class for constant single-value unit parameters.
template<typename T>
class CConstUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion{ 1 };

	T m_value{};									///< Const value.
	T m_min{};										///< Minimum allowed value.
	T m_max{};										///< Maximum allowed value.

public:
	CConstUnitParameter();
	CConstUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, T _value);

	void Clear() override;							///< Sets value to zero.

	T GetValue() const { return m_value; }			///< Returns constant unit parameter value.
	T GetMin() const{ return m_min; }				///< Returns minimum allowed value.
	T GetMax() const{ return m_max; }				///< Returns maximum allowed value.

	void SetValue(T _value) { m_value = _value; }	///< Sets constant unit parameter value.
	void SetMin(T _min){ m_min = _min; }			///< Sets minimum allowed value.
	void SetMax(T _max){ m_max = _max; }			///< Sets maximum allowed value.

	bool IsInBounds() const override;				///< Checks whether m_value lays in range [m_min; m_max].

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

private:
	EUnitParameter DeduceType() const;				///< Deduces type of the unit parameter depending on the template argument.
};

using CConstRealUnitParameter = CConstUnitParameter<double>;
using CConstIntUnitParameter  = CConstUnitParameter<int64_t>;
using CConstUIntUnitParameter = CConstUnitParameter<uint64_t>;

// Class for constant list unit parameters.
template<typename T>
class CListUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_saveVersion{ 0 };

	std::vector<T> m_values{};						// Const values.
	T m_min{};										// Minimum allowed value.
	T m_max{};										// Maximum allowed value.

public:
	CListUnitParameter();
	CListUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, std::vector<T> _values);

	// Removes all values.
	void Clear() override { m_values.clear(); }

	// Returns value at the given index of the list. Returns T{} if no value is defined for the given index.
	[[nodiscard]] T GetValue(size_t _index) const { if (_index >= m_values.size()) return T{}; return m_values[_index]; }
	// Add new value to the list.
	void AddValue(T _value) { m_values.push_back(_value); }
	// Sets new value at the given index of the list if it exists.
	void SetValue(size_t _index, T _value) { if (_index < m_values.size()) m_values[_index] = _value; }
	// Removes value at the given index of the list if it exists.
	void RemoveValue(size_t _index)	{ if (_index < m_values.size()) m_values.erase(m_values.begin() + _index); }
	// Returns all defined values.
	[[nodiscard]] std::vector<T> GetValues() const { return m_values; }

	// Returns minimum allowed value.
	[[nodiscard]] T GetMin() const { return m_min; }
	// Returns maximum allowed value.
	[[nodiscard]] T GetMax() const { return m_max; }

	// Sets new minimum allowed value.
	void SetMin(T _min) { m_min = _min; }
	// Sets new maximum allowed value.
	void SetMax(T _max) { m_max = _max; }

	// Returns the number of defined values.
	[[nodiscard]] size_t Size() const { return m_values.size(); }
	// Checks if any value is defined.
	[[nodiscard]] bool IsEmpty() const { return m_values.empty(); }
	// Checks if all values lay in range [min; max].
	[[nodiscard]] bool IsInBounds() const override { return std::all_of(m_values.begin(), m_values.end(), [&](const auto val) { return val >= m_min && val <= m_max; }); }

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

private:
	// Deduces type of the unit parameter depending on the template argument.
	[[nodiscard]] EUnitParameter DeduceType() const;
};

using CListRealUnitParameter = CListUnitParameter<double>;
using CListIntUnitParameter  = CListUnitParameter<int64_t>;
using CListUIntUnitParameter = CListUnitParameter<uint64_t>;

// Class for time-dependent unit parameters.
class CTDUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion{ 1 };

	CDependentValues m_values;                  ///< Time dependent values.
	double m_min{};								///< Minimum allowed value.
	double m_max{};								///< Maximum allowed value.

public:
	CTDUnitParameter();
	CTDUnitParameter(std::string _name, std::string _units, std::string _description, double _min, double _max, double _value);

	void Clear() override;                      ///< Removes all values.

	double GetMin() const;						///< Returns minimum allowed value.
	double GetMax() const;						///< Returns maximum allowed value.

	void SetMin(double _min);					///< Sets minimum allowed value.
	void SetMax(double _max);					///< Sets maximum allowed value.

	double GetValue(double _time) const;		///< Returns unit parameter value at given time point using interpolation if necessary.
	void SetValue(double _time, double _value);	///< Adds new unit parameter value at given time point or changes the value of existing one.
	void RemoveValue(double _time);             ///< Removes unit parameter value at given time point if it exists.

	std::vector<double> GetTimes() const;		///< Returns list of all defined time points.
	std::vector<double> GetValues() const;		///< Returns list of all defined values.
	const CDependentValues& GetTDData() const;  ///< Returns the time dependent data itself.

	size_t Size() const;	                    ///< Returns number of defined time points.
	bool IsEmpty() const;	                    ///< Checks whether any time point is defined.
	bool IsInBounds() const override;           ///< Checks whether all m_values lay in range [m_min; m_max].

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};


class CStringUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_value;                      ///< String parameter value.

public:
	CStringUnitParameter();
	CStringUnitParameter(std::string _name, std::string _description, std::string _value);

	void Clear() override;                    ///< Resets value.

	std::string GetValue() const;             ///< Returns string unit parameter value.
	void SetValue(const std::string& _value); ///< Sets string unit parameter value.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


class CCheckBoxUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	bool m_checked;					///< Check box parameter value: checked/unchecked.

public:
	CCheckBoxUnitParameter();
	CCheckBoxUnitParameter(std::string _name, std::string _description, bool _checked);

	void Clear() override;          ///< Resets value to unchecked.

	bool IsChecked() const;			///< Returns check box unit parameter value.
	void SetChecked(bool _checked); ///< Sets check box unit parameter value.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


class CSolverUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_key;                      ///< Solver's key.
	ESolverTypes m_solverType;              ///< Solver's type.
	CBaseSolver* m_solver{ nullptr };		///< Pointer to a selected solver.

public:
	CSolverUnitParameter();
	CSolverUnitParameter(std::string _name, std::string _description, ESolverTypes _type);

	void Clear() override;                  ///< Resets solver's key and type.

	std::string GetKey() const;             ///< Returns solver's key.
	ESolverTypes GetSolverType() const;     ///< Returns solver's type.
	CBaseSolver* GetSolver() const;			///< Returns pointer to a solver.

	void SetKey(const std::string& _key);   ///< Sets solver's key.
	void SetSolverType(ESolverTypes _type); ///< Sets solver's type.
	void SetSolver(CBaseSolver* _solver);	///< Sets pointer to a solver.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


class CComboUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::map<size_t, std::string> m_items;			///< List of possible items to select (value:name).
	size_t m_selected = -1;							///< Selected item.

public:
	CComboUnitParameter();
	CComboUnitParameter(std::string _name, std::string _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames);

	void Clear() override;							      ///< Resets selected key.

	size_t GetValue() const;						      ///< Returns currently selected item.
	void SetValue(size_t _item);					      ///< Sets new selected item.

	std::vector<size_t> GetItems() const;				  ///< Returns all items.
	std::vector<std::string> GetNames() const;			  ///< Returns all items' names.
	size_t GetItemByName(const std::string& _name) const; ///< Returns item by its name.

	bool HasItem(size_t _item) const;				      ///< Returns true if m_items contains _item.
	bool HasName(const std::string& _name) const;	      ///< Returns true if m_items contains item with _name.

	bool IsInBounds() const override;				      ///< Checks whether m_selected is one of the m_items.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


class CCompoundUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::string m_key;							///< Unique key of selected compound.

public:
	CCompoundUnitParameter();
	CCompoundUnitParameter(std::string _name, std::string _description);

	void Clear() override;						///< Resets compound's key.

	std::string GetCompound() const;			///< Returns key of currently selected compound.
	void SetCompound(const std::string& _key);	///< Sets new compound's key.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};

class CMDBCompoundUnitParameter : public CCompoundUnitParameter
{
	static const unsigned m_cnSaveVersion;

public:
	CMDBCompoundUnitParameter();
	CMDBCompoundUnitParameter(std::string _name, std::string _description);
};

class CReactionUnitParameter : public CBaseUnitParameter
{
	static const unsigned m_cnSaveVersion;

	std::vector<CChemicalReaction> m_reactions;					// Defined reactions.

public:
	CReactionUnitParameter();
	CReactionUnitParameter(std::string _name, std::string _description);

	void Clear() override;														// Clears all reactions.

	[[nodiscard]] std::vector<CChemicalReaction> GetReactions() const;			// Returns all defined chemical reactions.
	std::vector<CChemicalReaction*> GetReactionsPtr();							// Returns modifiable versions of defined chemical reactions.
	[[nodiscard]] const CChemicalReaction* GetReaction(size_t _index) const;	// Returns pointer to a selected reaction. If such reaction does not exist, returns nullptr.
	CChemicalReaction* GetReaction(size_t _index);								// Returns pointer to a selected reaction. If such reaction does not exist, returns nullptr.
	[[nodiscard]] size_t GetReactionsNumber() const;							// Returns the number of defined reactions.

	void AddReaction();															// Adds new empty reaction.
	void AddReaction(const CChemicalReaction& _reaction);						// Adds new reaction.
	void SetReactions(const std::vector<CChemicalReaction>& _reactions);		// Sets new reactions replacing existing.

	void RemoveReaction(size_t _index);											// Removes the selected reaction.

	// Outputs user-specified values of the parameter to a stream.
	std::ostream& ValueToStream(std::ostream& _s) override;
	// Reads user-specified values of the parameter from a stream.
	std::istream& ValueFromStream(std::istream& _s) override;

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const;
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);
};


/* Manager of unit parameters for each unit.
 * Each parameter may be a member of one or several groups, to allow showing / hiding of some parameters in GUI.
 * Block is defined by a single CComboUnitParameter and may have several options to choose.
 * Each option is a group of one or several parameters, which should be shown / hidden together, depending on the selection of corresponding CComboUnitParameter.
 * One parameter can belong to several groups and several blocks.
 * Block is stored as an index of a CComboUnitParameter. Group is stored as indices of parameters, which belong to this group.*/
class CUnitParametersManager
{
	static const unsigned m_cnSaveVersion{ 1 };

	using group_map_t = std::map<size_t, std::map<size_t, std::vector<size_t>>>; // map<iParameter, map<iBlock, vector<iGroups>>>

	std::vector<std::unique_ptr<CBaseUnitParameter>> m_parameters;  ///< All parameters.
	group_map_t m_groups;											///< List of group blocks and corresponding groups, to which parameters belong. Is used to determine activity of parameters.

public:
	// Returns number of specified unit parameters.
	size_t ParametersNumber() const;
	// Returns true if unit parameter with given name already exists.
	bool IsNameExist(const std::string& _name) const;

	// Adds new real constant unit parameter. If parameter with the given name already exists, does nothing.
	void AddConstRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value);
	// Adds new signed integer constant unit parameter. If parameter with the given name already exists, does nothing.
	void AddConstIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value);
	// Adds new unsigned integer constant unit parameter. If parameter with the given name already exists, does nothing.
	void AddConstUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value);
	// Adds new time-dependent unit parameter. If parameter with the given name already exists, does nothing.
	void AddTDParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value);
	// Adds new string unit parameter. If parameter with the given name already exists, does nothing.
	void AddStringParameter(const std::string& _name, const std::string& _description, const std::string& _value);
	// Adds new check box unit parameter. If parameter with the given name already exists, does nothing.
	void AddCheckBoxParameter(const std::string& _name, const std::string& _description, bool _value);
	// Adds new solver unit parameter. If parameter with the given name already exists, does nothing.
	void AddSolverParameter(const std::string& _name, const std::string& _description, ESolverTypes _type);
	// Adds new combo unit parameter. If parameter with the given name already exists, does nothing.
	void AddComboParameter(const std::string& _name, const std::string& _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames);
	// Adds new compound unit parameter. If parameter with the given name already exists, does nothing.
	void AddCompoundParameter(const std::string& _name, const std::string& _description);
	// Adds new MDB compound unit parameter. If parameter with the given name already exists, does nothing.
	void AddMDBCompoundParameter(const std::string& _name, const std::string& _description);
	// Adds new reaction unit parameter. If parameter with the given name already exists, does nothing.
	void AddReactionParameter(const std::string& _name, const std::string& _description);
	// Adds new real list unit parameter. If parameter with the given name already exists, does nothing.
	void AddListRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, const std::vector<double>& _values);
	// Adds new signed integer list unit parameter. If parameter with the given name already exists, does nothing.
	void AddListIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, const std::vector<int64_t>& _values);
	// Adds new unsigned integer list unit parameter. If parameter with the given name already exists, does nothing.
	void AddListUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, const std::vector<uint64_t>& _values);

	// Returns list of all defined parameters.
	std::vector<CBaseUnitParameter*> GetParameters() const;

	// Returns pointer to the unit parameter with the specified _index.
	const CBaseUnitParameter* GetParameter(size_t _index) const;
	// Returns const pointer to the unit parameter with the specified _index.
	CBaseUnitParameter* GetParameter(size_t _index);
	// Returns pointer to the unit parameter with the specified _name.
	const CBaseUnitParameter* GetParameter(const std::string& _name) const;
	// Returns const pointer to the unit parameter with the specified _name.
	CBaseUnitParameter* GetParameter(const std::string& _name);

	// Returns const pointer to the real constant unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CConstRealUnitParameter* GetConstRealParameter(size_t _index) const;
	// Returns pointer to the constant real unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CConstRealUnitParameter* GetConstRealParameter(size_t _index);
	// Returns const pointer to the signed integer constant unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CConstIntUnitParameter* GetConstIntParameter(size_t _index) const;
	// Returns pointer to the constant signed integer unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CConstIntUnitParameter* GetConstIntParameter(size_t _index);
	// Returns const pointer to the unsigned integer constant unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CConstUIntUnitParameter* GetConstUIntParameter(size_t _index) const;
	// Returns pointer to the constant unsigned integer unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CConstUIntUnitParameter* GetConstUIntParameter(size_t _index);
	// Returns const pointer to the time-dependent unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CTDUnitParameter* GetTDParameter(size_t _index) const;
	// Returns pointer to the time-dependent unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CTDUnitParameter* GetTDParameter(size_t _index);
	// Returns const pointer to the string unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CStringUnitParameter* GetStringParameter(size_t _index) const;
	// Returns pointer to the string unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CStringUnitParameter* GetStringParameter(size_t _index);
	// Returns const pointer to the check box unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CCheckBoxUnitParameter* GetCheckboxParameter(size_t _index) const;
	// Returns pointer to the check box unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CCheckBoxUnitParameter* GetCheckboxParameter(size_t _index);
	// Returns const pointer to the solver unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CSolverUnitParameter* GetSolverParameter(size_t _index) const;
	// Returns pointer to the solver unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CSolverUnitParameter* GetSolverParameter(size_t _index);
	// Returns const pointer to the combo unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CComboUnitParameter* GetComboParameter(size_t _index) const;
	// Returns pointer to the combo unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CComboUnitParameter* GetComboParameter(size_t _index);
	// Returns const pointer to the compound unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CCompoundUnitParameter* GetCompoundParameter(size_t _index) const;
	// Returns pointer to the compound unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CCompoundUnitParameter* GetCompoundParameter(size_t _index);
	// Returns const pointer to the MDB compound unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CMDBCompoundUnitParameter* GetMDBCompoundParameter(size_t _index) const;
	// Returns pointer to the MDB compound unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CMDBCompoundUnitParameter* GetMDBCompoundParameter(size_t _index);
	// Returns const pointer to the reaction unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CReactionUnitParameter* GetReactionParameter(size_t _index) const;
	// Returns pointer to the reaction unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CReactionUnitParameter* GetReactionParameter(size_t _index);
	// Returns const pointer to the real list unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CListRealUnitParameter* GetListRealParameter(size_t _index) const;
	// Returns pointer to the list real unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CListRealUnitParameter* GetListRealParameter(size_t _index);
	// Returns const pointer to the signed integer list unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CListIntUnitParameter* GetListIntParameter(size_t _index) const;
	// Returns pointer to the signed integer list unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CListIntUnitParameter* GetListIntParameter(size_t _index);
	// Returns const pointer to the unsigned integer list unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	const CListUIntUnitParameter* GetListUIntParameter(size_t _index) const;
	// Returns pointer to the unsigned integer list unit parameter with the specified _index. If such parameter does not exist, returns nullptr.
	CListUIntUnitParameter* GetListUIntParameter(size_t _index);

	// Returns const pointer to the real constant unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CConstRealUnitParameter* GetConstRealParameter(const std::string& _name) const;
	// Returns pointer to the constant real unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CConstRealUnitParameter* GetConstRealParameter(const std::string& _name);
	// Returns const pointer to the signed integer constant unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CConstIntUnitParameter* GetConstIntParameter(const std::string& _name) const;
	// Returns pointer to the constant signed integer unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CConstIntUnitParameter* GetConstIntParameter(const std::string& _name);
	// Returns const pointer to the unsigned integer constant unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CConstUIntUnitParameter* GetConstUIntParameter(const std::string& _name) const;
	// Returns pointer to the constant unsigned integer unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CConstUIntUnitParameter* GetConstUIntParameter(const std::string& _name);
	// Returns const pointer to the time-dependent unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CTDUnitParameter* GetTDParameter(const std::string& _name) const;
	// Returns pointer to the time-dependent unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CTDUnitParameter* GetTDParameter(const std::string& _name);
	// Returns const pointer to the string unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CStringUnitParameter* GetStringParameter(const std::string& _name) const;
	// Returns pointer to the string unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CStringUnitParameter* GetStringParameter(const std::string& _name);
	// Returns const pointer to the check box unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CCheckBoxUnitParameter* GetCheckboxParameter(const std::string& _name) const;
	// Returns pointer to the check box unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CCheckBoxUnitParameter* GetCheckboxParameter(const std::string& _name);
	// Returns const pointer to the solver unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CSolverUnitParameter* GetSolverParameter(const std::string& _name) const;
	// Returns pointer to the solver unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CSolverUnitParameter* GetSolverParameter(const std::string& _name);
	// Returns const pointer to the combo unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CComboUnitParameter* GetComboParameter(const std::string& _name) const;
	// Returns pointer to the combo unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CComboUnitParameter* GetComboParameter(const std::string& _name);
	// Returns const pointer to the compound unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CCompoundUnitParameter* GetCompoundParameter(const std::string& _name) const;
	// Returns pointer to the compound unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CCompoundUnitParameter* GetCompoundParameter(const std::string& _name);
	// Returns const pointer to the MDB compound unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CMDBCompoundUnitParameter* GetMDBCompoundParameter(const std::string& _name) const;
	// Returns pointer to the MDB compound unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CMDBCompoundUnitParameter* GetMDBCompoundParameter(const std::string& _name);
	// Returns const pointer to the reaction unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CReactionUnitParameter* GetReactionParameter(const std::string& _name) const;
	// Returns pointer to the reaction unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CReactionUnitParameter* GetReactionParameter(const std::string& _name);
	// Returns const pointer to the real list unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CListRealUnitParameter* GetListRealParameter(const std::string& _name) const;
	// Returns pointer to the list real unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CListRealUnitParameter* GetListRealParameter(const std::string& _name);
	// Returns const pointer to the signed integer list unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CListIntUnitParameter* GetListIntParameter(const std::string& _name) const;
	// Returns pointer to the list signed integer unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CListIntUnitParameter* GetListIntParameter(const std::string& _name);
	// Returns const pointer to the unsigned integer list unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	const CListUIntUnitParameter* GetListUIntParameter(const std::string& _name) const;
	// Returns pointer to the list unsigned integer unit parameter with the specified _name. If such parameter does not exist, returns nullptr.
	CListUIntUnitParameter* GetListUIntParameter(const std::string& _name);

	// Returns value of a constant real unit parameter with the specified _index. If such parameter does not exist or is not a constant real parameter, returns 0.
	double GetConstRealParameterValue(size_t _index) const;
	// Returns value of a constant signed integer unit parameter with the specified _index. If such parameter does not exist or is not a constant signed integer parameter, returns 0.
	int64_t GetConstIntParameterValue(size_t _index) const;
	// Returns value of a constant unsigned integer unit parameter with the specified _index. If such parameter does not exist or is not a constant unsigned integer parameter, returns 0.
	uint64_t GetConstUIntParameterValue(size_t _index) const;
	// Returns value of a TD unit parameter with the specified _index and _time. If such parameter does not exist or is not a TD parameter, returns 0.
	double GetTDParameterValue(size_t _index, double _time) const;
	// Returns value of a string unit parameter with the specified _index. If such parameter does not exist or is not a string parameter, returns "".
	std::string GetStringParameterValue(size_t _index) const;
	// Returns value of a check box unit parameter with the specified _index. If such parameter does not exist or is not a check box parameter, returns false.
	bool GetCheckboxParameterValue(size_t _index) const;
	// Returns value of a solver unit parameter with the specified _index. If such parameter does not exist or is not a solver parameter, returns "".
	std::string GetSolverParameterValue(size_t _index) const;
	// Returns value of a combo unit parameter with the specified _index. If such parameter does not exist or is not a combo parameter, returns -1.
	size_t GetComboParameterValue(size_t _index) const;
	// Returns value of a compound unit parameter with the specified _index. If such parameter does not exist or is not a compound parameter, returns "".
	std::string GetCompoundParameterValue(size_t _index) const;
	// Returns value of a MDB compound unit parameter with the specified _index. If such parameter does not exist or is not a MDB compound parameter, returns "".
	std::string GetMDBCompoundParameterValue(size_t _index) const;
	// Returns value of a reaction unit parameter with the specified _index. If such parameter does not exist or is not a reaction parameter, returns empty vector.
	std::vector<CChemicalReaction> GetReactionParameterValue(size_t _index) const;
	// Returns value of a list real unit parameter with the specified _index. If such parameter does not exist or is not a list real parameter, returns 0.
	double GetListRealParameterValue(size_t _index, size_t _vauleIndex) const;
	// Returns value of a list signed integer unit parameter with the specified _index. If such parameter does not exist or is not a list signed integer parameter, returns 0.
	int64_t GetListIntParameterValue(size_t _index, size_t _vauleIndex) const;
	// Returns value of a list unsigned integer unit parameter with the specified _index. If such parameter does not exist or is not a list unsigned integer parameter, returns 0.
	uint64_t GetListUIntParameterValue(size_t _index, size_t _vauleIndex) const;

	// Returns value of a constant real unit parameter with the specified _name. If such parameter does not exist or is not a constant real parameter, returns 0.
	double GetConstRealParameterValue(const std::string& _name) const;
	// Returns value of a constant signed integer unit parameter with the specified _name. If such parameter does not exist or is not a constant signed integer parameter, returns 0.
	int64_t GetConstIntParameterValue(const std::string& _name) const;
	// Returns value of a constant unsigned integer unit parameter with the specified _name. If such parameter does not exist or is not a constant unsigned integer parameter, returns 0.
	uint64_t GetConstUIntParameterValue(const std::string& _name) const;
	// Returns value of a TD unit parameter with the specified _name and _time. If such parameter does not exist or is not a TD parameter, returns 0.
	double GetTDParameterValue(const std::string& _name, double _time) const;
	// Returns value of a string unit parameter with the specified _name. If such parameter does not exist or is not a string parameter, returns "".
	std::string GetStringParameterValue(const std::string& _name) const;
	// Returns value of a check box unit parameter with the specified _name. If such parameter does not exist or is not a check box parameter, returns "".
	bool GetCheckboxParameterValue(const std::string& _name) const;
	// Returns value of a solver unit parameter with the specified _name. If such parameter does not exist or is not a solver parameter, returns "".
	std::string GetSolverParameterValue(const std::string& _name) const;
	// Returns value of a combo unit parameter with the specified _name. If such parameter does not exist or is not a combo parameter, returns -1.
	size_t GetComboParameterValue(const std::string& _name) const;
	// Returns value of a compound unit parameter with the specified _name. If such parameter does not exist or is not a compound parameter, returns "".
	std::string GetCompoundParameterValue(const std::string& _name) const;
	// Returns value of a MDB compound unit parameter with the specified _name. If such parameter does not exist or is not a MDB compound parameter, returns "".
	std::string GetMDBCompoundParameterValue(const std::string& _name) const;
	// Returns value of a reaction unit parameter with the specified _name. If such parameter does not exist or is not a reaction parameter, returns "".
	std::vector<CChemicalReaction> GetReactionParameterValue(const std::string& _name) const;
	// Returns value of a list real unit parameter with the specified _name. If such parameter does not exist or is not a list real parameter, returns 0.
	double GetListRealParameterValue(const std::string& _name, size_t _index) const;
	// Returns value of a list signed integer unit parameter with the specified _name. If such parameter does not exist or is not a list signed integer parameter, returns 0.
	int64_t GetListIntParameterValue(const std::string& _name, size_t _index) const;
	// Returns value of a list unsigned integer unit parameter with the specified _name. If such parameter does not exist or is not a list unsigned integer parameter, returns 0.
	uint64_t GetListUIntParameterValue(const std::string& _name, size_t _index) const;

	// Returns const pointers to all specified reaction unit parameters.
	std::vector<const CReactionUnitParameter*> GetAllReactionParameters() const;
	// Returns pointers to all specified reaction unit parameters.
	std::vector<CReactionUnitParameter*> GetAllReactionParameters();
	// Returns const pointers to all specified compound and MDB compound unit parameters.
	std::vector<const CCompoundUnitParameter*> GetAllCompoundParameters() const;
	// Returns pointers to all specified compound and MDB compound unit parameters.
	std::vector<CCompoundUnitParameter*> GetAllCompoundParameters();

	// Returns const pointers to all specified solver unit parameters.
	std::vector<const CSolverUnitParameter*> GetAllSolverParameters() const;
	// Returns pointers to all specified solver unit parameters.
	std::vector<CSolverUnitParameter*> GetAllSolverParameters();
	// Returns a sorted list of time points form given interval defined in all unit parameters.
	std::vector<double> GetAllTimePoints(double _tBeg, double _tEnd) const;

	// Adds the list of _parameters by their indices to existing _group of existing _block. If _block, _group or some of parameters do not exist, does nothing.
	void AddParametersToGroup(size_t _block, size_t _group, const std::vector<size_t>& _parameters);
	// Adds the list of _parameters by their names to existing _group of existing _block. If _block, _group or some of parameters do not exist, does nothing.
	void AddParametersToGroup(const std::string& _block, const std::string& _group, const std::vector<std::string>& _parameters);
	// Returns true if unit parameter with the specified _index is selected in at least one group of any block, or if it is a not grouped parameter.
	bool IsParameterActive(size_t _index) const;
	// Returns true if this parameter is selected in at least one group of any block, or if it is a not grouped parameter.
	bool IsParameterActive(const CBaseUnitParameter& _parameter) const;

	// Save all parameters to HDF5 file.
	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path);
	// Load all parameters from HDF5 file.
	void LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path);

private:
	// Makes parameter with index _parameter a member of the corresponding _block and _group.
	void AddToGroup(size_t _parameter, size_t _block, size_t _group);

	// Returns index of a unit parameter with the given name.
	size_t Name2Index(const std::string& _name) const;
	// Returns indices of unit parameter with the given names.
	std::vector<size_t> Name2Index(const std::vector<std::string>& _names) const;
};
