/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ModelsManager.h"
#include "DynamicUnit.h"
#include "FileSystem.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#ifdef _MSC_VER
#else
#include <dlfcn.h>
#endif

size_t CModelsManager::DirsNumber() const
{
	return m_dirsList.size();
}

bool CModelsManager::AddDir(const std::filesystem::path& _path, bool _active)
{
	const auto& it = std::find_if(m_dirsList.begin(), m_dirsList.end(), [&](const SModelDir& entry) { return entry.path == _path; });
	if (it != m_dirsList.end()) return false;	// this path has been already added
	m_dirsList.push_back(SModelDir{ _path , StringFunctions::GenerateUniqueKey("", AllDirsKeys()), _active, false });
	UpdateAvailableModels();
	return true;
}

bool CModelsManager::RemoveDir(size_t _index)
{
	if (_index >= m_dirsList.size()) return false;
	m_dirsList.erase(m_dirsList.begin() + _index);
	UpdateAvailableModels();
	return true;
}

bool CModelsManager::UpDir(size_t _index)
{
	if (_index >= m_dirsList.size() || _index == 0) return false;
	std::iter_swap(m_dirsList.begin() + _index, m_dirsList.begin() + _index - 1);
	UpdateAvailableModels();
	return true;
}

bool CModelsManager::DownDir(size_t _index)
{
	if (_index >= m_dirsList.size() || _index == m_dirsList.size() - 1) return false;
	std::iter_swap(m_dirsList.begin() + _index, m_dirsList.begin() + _index + 1);
	UpdateAvailableModels();
	return true;
}

std::filesystem::path CModelsManager::GetDirPath(size_t _index) const
{
	return _index < m_dirsList.size() ? m_dirsList[_index].path : "";
}

std::vector<std::filesystem::path> CModelsManager::GetAllActiveDirPaths() const
{
	std::vector<std::filesystem::path> res;
	for (const auto& p : m_dirsList)
		if (p.active)
			res.push_back(p.path);
	return res;
}

bool CModelsManager::GetDirActivity(size_t _index) const
{
	return _index < m_dirsList.size() ? m_dirsList[_index].active : false;
}

void CModelsManager::SetDirActivity(size_t _index, bool _active)
{
	if (_index >= m_dirsList.size()) return;
	m_dirsList[_index].checked = m_dirsList[_index].active || !_active;
	m_dirsList[_index].active = _active;
	UpdateAvailableModels();
}

void CModelsManager::Clear()
{
	m_dirsList.clear();
	m_availableUnits.clear();
	m_availableSolvers.clear();
}

std::vector<SUnitDescriptor> CModelsManager::GetAvailableUnits() const
{
	return m_availableUnits;
}

std::vector<SSolverDescriptor> CModelsManager::GetAvailableSolvers() const
{
	return m_availableSolvers;
}

SSolverDescriptor CModelsManager::GetSolverDescriptor(const std::wstring& _fileName) const
{
	const std::wstring sLibName = FileSystem::FileName(_fileName);
	const auto res = std::find_if(m_availableSolvers.begin(), m_availableSolvers.end(), [&](const SSolverDescriptor& s) { return s.fileLocation.filename() == sLibName; });
	if (res == m_availableSolvers.end()) return {};
	return *res;
}

std::wstring CModelsManager::GetSolverLibName(const std::string& _key) const
{
	const auto res = std::find_if(m_availableSolvers.begin(), m_availableSolvers.end(), [&](const SSolverDescriptor& s) { return s.uniqueID == _key; });
	if (res == m_availableSolvers.end()) return {};
	return res->fileLocation.filename().wstring();
}

CBaseUnit* CModelsManager::InstantiateUnit(const std::string& _key)
{
	for (const auto& u : m_availableUnits)	// go through all available units
		if (u.uniqueID == _key)			// find the required descriptor
		{
			// load library
			const DYSSOL_LIBRARY_INSTANCE hLibrary = LoadDyssolLibrary(u.fileLocation);
			if (!hLibrary) return nullptr;
			// get constructor function
			const CreateUnit2 createUnitFunc = reinterpret_cast<CreateUnit2>(LoadDyssolLibraryConstructor(hLibrary, DYSSOL_CREATE_MODEL_FUN_NAME));
			if (!createUnitFunc)
			{
				CloseDyssolLibrary(hLibrary);
				continue; // seek further
			}
			// instantiate unit
			CBaseUnit* pUnit = createUnitFunc();
			if (!pUnit)
			{
				CloseDyssolLibrary(hLibrary);
				continue; // seek further
			}
			pUnit->CreateBasicInfo();
			// save created unit and its library
			m_loadedUnits[pUnit] = hLibrary;
			// return instantiated unit
			return pUnit;
		}
	return nullptr;
}

CBaseSolver* CModelsManager::InstantiateSolver(const std::string& _key)
{
	for (const auto& s : m_availableSolvers)	// go through all available solvers
		if (s.uniqueID == _key)				// find the required descriptor
		{
			// load library
			const DYSSOL_LIBRARY_INSTANCE hLibrary = LoadDyssolLibrary(s.fileLocation);
			if (!hLibrary) return nullptr;
			// get constructor function
			const CreateExternalSolver createSolverFunc = reinterpret_cast<CreateExternalSolver>(LoadDyssolLibraryConstructor(hLibrary, std::vector<std::string>{CREATE_SOLVER_FUN_NAMES}[static_cast<unsigned>(s.solverType)]));
			if (!createSolverFunc)
			{
				CloseDyssolLibrary(hLibrary);
				continue; // seek further
			}
			// instantiate solver
			CBaseSolver* pSolver = createSolverFunc();
			if (!pSolver)
			{
				CloseDyssolLibrary(hLibrary);
				continue; // seek further
			}
			pSolver->CreateBasicInfo();
			// save created solver and its library
			m_loadedSolvers[pSolver] = hLibrary;
			// return instantiated solver
			return pSolver;
		}
	return nullptr;
}

void CModelsManager::FreeUnit(CBaseUnit* _unit)
{
	if (!_unit) return;
	// test if such unit exists
	if (m_loadedUnits.find(_unit) == m_loadedUnits.end()) return;
	// copy entry
	const DYSSOL_LIBRARY_INSTANCE hLibrary = m_loadedUnits[_unit];
	// remove it from the list
	m_loadedUnits.erase(_unit);
	// delete unit
	delete _unit;
	_unit = nullptr;
	// close the library
	CloseDyssolLibrary(hLibrary);
}

void CModelsManager::FreeSolver(CBaseSolver* _solver)
{
	if (!_solver) return;
	// test if such solver exists
	if (m_loadedSolvers.find(_solver) == m_loadedSolvers.end()) return;
	// copy entry
	const DYSSOL_LIBRARY_INSTANCE hLibrary = m_loadedSolvers[_solver];
	// remove it from the list
	m_loadedSolvers.erase(_solver);
	// delete unit
	delete _solver;
	_solver = nullptr;
	// close the library
	CloseDyssolLibrary(hLibrary);
}

std::vector<std::string> CModelsManager::AllDirsKeys() const
{
	std::vector<std::string> res;
	for (const auto& dir : m_dirsList)
		res.push_back(dir.key);
	return res;
}

void CModelsManager::UpdateAvailableModels()
{
	// remove models laying in removed dirs
	const auto IsToDelete = [&](const SModelDescriptor& _model) -> bool // checks whether to delete model
	{
		for (const auto& dir : m_dirsList)
			if (dir.active && dir.key == _model.dirKey)
				return false;
		return true;
	};
	VectorDelete(m_availableUnits, IsToDelete);
	VectorDelete(m_availableSolvers, IsToDelete);

	// add models from added dirs
	for (auto& dir : m_dirsList)
		if (dir.active && !dir.checked)
		{
			// get all models from directory
			auto models = GetModelsList(dir.path);
			// set directory key to all found models
			for (auto& unit : models.first)
				unit.dirKey = dir.key;
			for (auto& unit : models.second)
				unit.dirKey = dir.key;
			// add models to lists
			m_availableUnits.insert(m_availableUnits.end(), models.first.begin(), models.first.end());
			m_availableSolvers.insert(m_availableSolvers.end(), models.second.begin(), models.second.end());
			// set this directory as already checked
			dir.checked = true;
		}

	// sort models according to dirs order
	for (size_t i = 0; i < m_dirsList.size(); ++i)
	{
		for (auto& unit : m_availableUnits)
			if (unit.dirKey == m_dirsList[i].key)
				unit.position = i;
		for (auto& solver : m_availableSolvers)
			if (solver.dirKey == m_dirsList[i].key)
				solver.position = i;
	}
	std::sort(m_availableUnits.begin(), m_availableUnits.end());
	std::sort(m_availableSolvers.begin(), m_availableSolvers.end());
}

std::pair<std::vector<SUnitDescriptor>, std::vector<SSolverDescriptor>> CModelsManager::GetModelsList(const std::filesystem::path& _dir)
{
	// try to treat _dir as absolute path
	const std::filesystem::path absPath = std::filesystem::absolute(_dir);
	auto models = GetAllModelsInDir(absPath);
	if (models.first.empty() && models.second.empty())
	{
		// try to treat _dir as relative path
		const std::filesystem::path relPath = std::filesystem::absolute(FileSystem::ExecutableDirPath() + L"/" + _dir.wstring());
		models = GetAllModelsInDir(relPath);
	}
	return models;
}

std::pair<std::vector<SUnitDescriptor>, std::vector<SSolverDescriptor>> CModelsManager::GetAllModelsInDir(const std::filesystem::path& _dir)
{
	std::vector<SUnitDescriptor> resUnits;
	std::vector<SSolverDescriptor> resSolvers;
	for (const auto& f : FileSystem::FilesList(_dir, StrConst::MM_LibraryFileExtension))
		if(const DYSSOL_LIBRARY_INSTANCE lib = LoadDyssolLibrary(f))
		{
			if(const SUnitDescriptor unit = TryGetUnitDescriptor(f, lib))             // try to load unit from library
				resUnits.push_back(unit);
			else if (const SSolverDescriptor solver = TryGetSolverDescriptor(f, lib)) // try to load solver from library
				resSolvers.push_back(solver);
			CloseDyssolLibrary(lib);
		}
	return std::make_pair(resUnits, resSolvers);
}

SUnitDescriptor CModelsManager::TryGetUnitDescriptor(const std::filesystem::path& _pathToUnit, DYSSOL_LIBRARY_INSTANCE _library)
{
	// try to get constructor
	const CreateUnit2 createUnitFunc = reinterpret_cast<CreateUnit2>(LoadDyssolLibraryConstructor(_library, DYSSOL_CREATE_MODEL_FUN_NAME));
	if (!createUnitFunc)
		return {};

	// try to create unit
	CBaseUnit* pUnit;
	try {
		pUnit = createUnitFunc();
	}
	catch (const std::logic_error& e) {
		std::cerr << e.what() << std::endl;
		return {};
	}

	// do not allow code in constructor, as in previous versions
	try {
		if (!pUnit->GetUnitName().empty() || !pUnit->GetUniqueID().empty() || !pUnit->GetAuthorName().empty() || pUnit->GetVersion() != 0)
			return {};
	}
	catch (const std::logic_error& e) {
		std::cerr << e.what() << std::endl;
		return {};
	}

	pUnit->CreateBasicInfo();

	SUnitDescriptor unitDescriptor;

	// obtain descriptor information
	try {
		unitDescriptor.uniqueID = pUnit->GetUniqueID();
		unitDescriptor.name = pUnit->GetUnitName();
		unitDescriptor.author = pUnit->GetAuthorName();
		unitDescriptor.version = pUnit->GetVersion();
		unitDescriptor.isDynamic = dynamic_cast<CDynamicUnit*>(pUnit);
		unitDescriptor.fileLocation = StringFunctions::UnifyPath(_pathToUnit);
	}
	catch (...) {
		unitDescriptor = {};
	}

	delete pUnit;
	return unitDescriptor;
}

SSolverDescriptor CModelsManager::TryGetSolverDescriptor(const std::filesystem::path& _pathToSolver, DYSSOL_LIBRARY_INSTANCE _library)
{
	SSolverDescriptor solverDescriptor;
	CBaseSolver* pSolver = nullptr;
	bool bFound = false;

	// go through all solver types
	for (size_t i = 1; i <= SOLVERS_TYPES_NUMBER && !bFound; ++i)
	{
		// try to get constructor
		const CreateExternalSolver createSolverFunc = reinterpret_cast<CreateExternalSolver>(LoadDyssolLibraryConstructor(_library, std::vector<std::string>{CREATE_SOLVER_FUN_NAMES}[i]));
		if (!createSolverFunc)
			continue;

		// try to create unit
		try {
			pSolver = createSolverFunc();
		}
		catch (const std::logic_error&) {
			continue;
		}

		// do not allow code in constructor, as in previous versions
		try {
			if (!pSolver->GetName().empty() || !pSolver->GetUniqueID().empty() || !pSolver->GetAuthorName().empty() || pSolver->GetVersion() != 0)
				continue;
		}
		catch (const std::logic_error&) {
			continue;
		}

		pSolver->CreateBasicInfo();

		// obtain descriptor information
		try {
			solverDescriptor.uniqueID = pSolver->GetUniqueID();
			solverDescriptor.name = pSolver->GetName();
			solverDescriptor.author = pSolver->GetAuthorName();
			solverDescriptor.version = pSolver->GetVersion();
			solverDescriptor.solverType = pSolver->GetType();
			solverDescriptor.fileLocation = StringFunctions::UnifyPath(_pathToSolver);
			bFound = true;
		}
		catch (...) {
			solverDescriptor = {};
		}
	}

	delete pSolver;
	return solverDescriptor;
}

DYSSOL_LIBRARY_INSTANCE CModelsManager::LoadDyssolLibrary(const std::filesystem::path& _libPath)
{
#ifdef _MSC_VER
	return LoadLibrary(_libPath.c_str());
#else
	return dlopen(_libPath.c_str(), RTLD_LAZY);
#endif
}

DYSSOL_CREATE_FUNCTION_TYPE CModelsManager::LoadDyssolLibraryConstructor(DYSSOL_LIBRARY_INSTANCE _lib, const std::string& _funName)
{
#ifdef _MSC_VER
	return GetProcAddress(_lib, _funName.c_str());
#else
	return dlsym(_lib, _funName.c_str());
#endif
}

void CModelsManager::CloseDyssolLibrary(DYSSOL_LIBRARY_INSTANCE _lib)
{
#ifdef _MSC_VER
	::FreeLibrary(_lib);
#else
	dlclose(_lib);
#endif
}
