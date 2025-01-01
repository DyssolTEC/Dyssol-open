/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitContainer.h"
#include "BaseUnit.h"
#include "DyssolStringConstants.h"

CUnitContainer::CUnitContainer(const std::string& _id, CModelsManager* _modelsManager,
	const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid* _grid, const std::vector<SOverallDescriptor>* _overall,
	const std::vector<SPhaseDescriptor>* _phases, const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics) :
	m_uniqueID{ _id.empty() ? StringFunctions::GenerateRandomKey() : _id },
	m_modelsManager{ _modelsManager },
	m_materialsDB{ _materialsDB },
	m_grid{ _grid },
	m_overall{ _overall },
	m_phases{ _phases },
	m_cache{ _cache },
	m_tolerance{ _tolerance },
	m_thermodynamics{ _thermodynamics }
{
}

CUnitContainer::CUnitContainer(const CUnitContainer& _other)
	: m_name{ _other.m_name }
	, m_uniqueID{ _other.m_uniqueID }
	, m_modelsManager{ _other.m_modelsManager }
	, m_materialsDB{ _other.m_materialsDB }
	, m_grid{ _other.m_grid }
	, m_overall{ _other.m_overall }
	, m_phases{ _other.m_phases }
	, m_cache{ _other.m_cache }
	, m_tolerance{ _other.m_tolerance }
	, m_thermodynamics{ _other.m_thermodynamics }
{
	if (_other.m_model)
	{
		SetModel(_other.m_model->GetUniqueID());
		if (m_model)
			*m_model = *_other.m_model;
	}
}

CUnitContainer::CUnitContainer(CUnitContainer&& _other) noexcept
{
	swap(*this, _other);
}

CUnitContainer& CUnitContainer::operator=(CUnitContainer _other)
{
	swap(*this, _other);
	return *this;
}

CUnitContainer& CUnitContainer::operator=(CUnitContainer&& _other) noexcept
{
	CUnitContainer tmp{ std::move(_other) };
	swap(tmp, _other);
	return *this;
}

CUnitContainer::~CUnitContainer()
{
	ClearExternalSolvers();
	m_modelsManager->FreeUnit(m_model);
}

void swap(CUnitContainer& _first, CUnitContainer& _second) noexcept
{
	using std::swap;
	swap(_first.m_name          , _second.m_name);
	swap(_first.m_uniqueID      , _second.m_uniqueID);
	swap(_first.m_model         , _second.m_model);
	swap(_first.m_modelsManager , _second.m_modelsManager);
	swap(_first.m_materialsDB   , _second.m_materialsDB);
	swap(_first.m_grid          , _second.m_grid);
	swap(_first.m_overall       , _second.m_overall);
	swap(_first.m_phases        , _second.m_phases);
	swap(_first.m_cache         , _second.m_cache);
	swap(_first.m_tolerance     , _second.m_tolerance);
	swap(_first.m_thermodynamics, _second.m_thermodynamics);
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
	if (m_model && m_model->GetUniqueID() == _uniqueID) return;
	m_modelsManager->FreeUnit(m_model);
	m_model = m_modelsManager->InstantiateUnit(_uniqueID);
	if (m_model)
	{
		m_model->ConfigureUnitStructures(m_materialsDB, *m_grid, m_overall, m_phases, m_cache, m_tolerance, m_thermodynamics);
		// TODO: catch exceptions from Add-functions
		m_model->DoCreateStructure();
	}
}

const CBaseUnit* CUnitContainer::GetModel() const
{
	return m_model;
}

CBaseUnit* CUnitContainer::GetModel()
{
	return m_model;
}

void CUnitContainer::SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;
	if (!m_model) return;
	m_model->SetMaterialsDatabase(m_materialsDB);
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

		CBaseSolver* solver = m_modelsManager->InstantiateSolver(parameter->GetKey());
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
		m_modelsManager->FreeSolver(parameter->GetSolver());
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
