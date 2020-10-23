/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitContainer.h"
#include "BaseUnit.h"
#include "DyssolStringConstants.h"

CUnitContainer::CUnitContainer(const std::string& _id, CModelsManager& _modelsManager,
	const CMaterialsDatabase& _materialsDB, const CDistributionsGrid& _grid, const std::vector<std::string>& _compounds, const std::vector<SOverallDescriptor>& _overall,
	const std::vector<SPhaseDescriptor>& _phases, const SCacheSettings& _cache, const SToleranceSettings& _tolerance) :
	m_uniqueID{ _id.empty() ? StringFunctions::GenerateRandomKey() : _id },
	m_modelsManager{ _modelsManager },
	m_materialsDB{ _materialsDB },
	m_grid{ _grid },
	m_compounds{ _compounds },
	m_overall{ _overall },
	m_phases{ _phases },
	m_cache{ _cache },
	m_tolerance{ _tolerance }
{
}

CUnitContainer::~CUnitContainer()
{
	ClearExternalSolvers();
	m_modelsManager.FreeUnit(m_model);
}

std::string CUnitContainer::GetName() const
{
	return m_name;
}

void CUnitContainer::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CUnitContainer::GetKey() const
{
	return m_uniqueID;
}

void CUnitContainer::SetKey(const std::string& _id)
{
	m_uniqueID = _id;
}

void CUnitContainer::SetModel(const std::string& _uniqueID)
{
	m_modelsManager.FreeUnit(m_model);
	m_model = m_modelsManager.InstantiateUnit(_uniqueID);
	if (m_model)
		m_model->SetPointers(&m_materialsDB, &m_grid, &m_compounds, &m_overall, &m_phases, &m_cache, &m_tolerance);
}

const CBaseUnit* CUnitContainer::GetModel() const
{
	return m_model;
}

CBaseUnit* CUnitContainer::GetModel()
{
	return m_model;
}

std::string CUnitContainer::InitializeExternalSolvers() const
{
	ClearExternalSolvers();

	for (auto& parameter : m_model->GetUnitParametersManager().GetAllSolverParameters())
	{
		if (parameter->GetKey().empty())
		{
			ClearExternalSolvers();
			return StrConst::Flow_ErrSolverKeyEmpty(parameter->GetName(), m_name);
		}

		CBaseSolver* solver = m_modelsManager.InstantiateSolver(parameter->GetKey());
		if (!solver)
		{
			ClearExternalSolvers();
			return StrConst::Flow_ErrCannotLoadSolverLib(parameter->GetName(), m_name);
		}

		parameter->SetSolver(solver);
	}

	return {};
}

void CUnitContainer::ClearExternalSolvers() const
{
	if (!m_model) return;
	for (auto& parameter : m_model->GetUnitParametersManager().GetAllSolverParameters())
		m_modelsManager.FreeSolver(parameter->GetSolver());
}

void CUnitContainer::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::BCont_H5UnitName, m_name);
	_h5File.WriteData(_path, StrConst::BCont_H5UnitKey,  m_uniqueID);

	if (m_model)
	{
		// save a copy of unique key of the model here, to be able to directly load a proper model afterwards
		_h5File.WriteData(_path, StrConst::BCont_H5ModelKey, m_model->GetUniqueID());
		m_model->SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BCont_H5GroupModel));
	}
}

void CUnitContainer::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// version of save procedure
	const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);
	if (version <= 1)
	{
		LoadFromFile_v1(_h5File, _path);
		return;
	}

	_h5File.ReadData(_path, StrConst::BCont_H5UnitName, m_name);
	_h5File.ReadData(_path, StrConst::BCont_H5UnitKey,  m_uniqueID);

	std::string modelKey;
	_h5File.ReadData(_path, StrConst::BCont_H5ModelKey, modelKey);
	SetModel(modelKey);

	if (m_model)
		m_model->LoadFromFile(_h5File, _path + "/" + StrConst::BCont_H5GroupModel);
}

void CUnitContainer::LoadFromFile_v1(CH5Handler& _h5File, const std::string& _path)
{
	const std::string H5UnitName   = "ModelName";
	const std::string H5UnitKey    = "ModelKey";
	const std::string H5ModelKey   = "UnitKey";
	const std::string H5GroupModel = "Unit";

	_h5File.ReadData(_path, H5UnitName, m_name);
	_h5File.ReadData(_path, H5UnitKey,  m_uniqueID);

	std::string modelKey;
	_h5File.ReadData(_path, H5ModelKey, modelKey);
	SetModel(modelKey);

	if (m_model)
		m_model->LoadFromFile(_h5File, _path + "/" + H5GroupModel);
}