/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "UnitParameters.h"

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
	void AddConstRealParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value);
	/**
	 * \private
	 * \brief Adds new signed integer constant unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddConstIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value);
	/**
	 * \private
	 * \brief Adds new unsigned integer constant unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddConstUIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value);
	/**
	 * \private
	 * \brief Adds new dependent unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddDependentParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value, const std::string& _paramName, const std::wstring& _paramUnits, double _paramMin, double _paramMax, double _paramValue);
	/**
	 * \private
	 * \brief Adds new time-dependent unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddTDParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value);
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
	void AddListRealParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, const std::vector<double>& _values);
	/**
	 * \private
	 * \brief Adds new signed integer list unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddListIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, int64_t _min, int64_t _max, const std::vector<int64_t>& _values);
	/**
	 * \private
	 * \brief Adds new unsigned integer list unit parameter.
	 * \details If parameter with the given name already exists, does nothing.
	 */
	void AddListUIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, uint64_t _min, uint64_t _max, const std::vector<uint64_t>& _values);

	/**
	 * \brief Returns list of all defined unit parameters.
	 * \return List of all defined unit parameters.
	 */
	[[nodiscard]] std::vector<CBaseUnitParameter*> GetParameters() const;
	/**
	 * \private
	 * \brief Returns list of all defined unit parameters of the given type.
	 * \return List of unit parameters.
	 */
	template <EUnitParameter... Args, std::enable_if_t<(sizeof...(Args) > 0), bool> = true>
	[[nodiscard]] std::vector<CBaseUnitParameter*> GetParameters() const
	{
		std::vector<CBaseUnitParameter*> result;
		auto loop = [this](std::vector<CBaseUnitParameter*>& _result, EUnitParameter _param)
			{
				for (size_t i = 0; i < ParametersNumber(); ++i)
				{
					const CBaseUnitParameter* unitParameter = GetParameter(i);
					if (unitParameter && unitParameter->GetType() == _param)
						_result.emplace_back(const_cast<CBaseUnitParameter*>(unitParameter));
				}
			};

		(loop(result, Args), ...);

		return result;
	}
	/**
	 * \private
	 * \brief Returns list of all defined unit parameters of the given type.
	 * \return List of unit parameters.
	 */
	template<EUnitParameter... Args>
	[[nodiscard]] std::vector<CBaseUnitParameter*> GetParameters(ParameterCollection<Args...>) const
	{
		std::vector<CBaseUnitParameter*> result;
		(([&result, this]
			{
				auto res = GetParameters<Args>();
				result.insert(result.end(), res.begin(), res.end());
			}()), ...);
		return result;
	}

	/**
	 * \brief Returns list of all active unit parameters.
	 * \return List of all active unit parameters.
	 */
	[[nodiscard]] std::vector<CBaseUnitParameter*> GetActiveParameters() const;

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
	 * \brief Clears all parameter groups.
	 */
	void ClearGroups();

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
