/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseSolver.h"
#include "DyssolFilesystem.h"
#include <map>
#include <vector>

#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>

typedef HINSTANCE DYSSOL_LIBRARY_INSTANCE;
typedef FARPROC DYSSOL_CREATE_FUNCTION_TYPE;
#else
typedef void* DYSSOL_LIBRARY_INSTANCE;
typedef void* DYSSOL_CREATE_FUNCTION_TYPE;
#endif

class CBaseUnit;

/// Structure to load, instantiate, free and unload models (units or solvers) from DLL (SO) files.
struct SModelDescriptor
{
	std::filesystem::path fileLocation; // Location where the model is stored.
	std::string uniqueID;				// Unique ID of this model.
	std::string name;					// Model's name.
	std::string author;					// Name of model's author.
	std::string dirKey;					// Unique key of the dir, where it stored.
	size_t version{};					// Model's version.
	size_t position{};					// Needed to sort models according to the list of dirs.

	bool operator<(const SModelDescriptor& _other) const
	{
		if (position == _other.position)
			return name < _other.name;
		return position < _other.position;
	}

	explicit operator bool() const
	{
		return !fileLocation.empty() || !uniqueID.empty();
	}
};

/// Structure to load units from DLL.
struct SUnitDescriptor : SModelDescriptor
{
	bool isDynamic{}; // Determines whether unit is dynamic or steady-state.
};

/// Structure to load solvers from DLL.
struct SSolverDescriptor : SModelDescriptor
{
	ESolverTypes solverType{};	// Type of the solver.
};

/// Manages directories with libraries.
class CModelsManager
{
	struct SModelDir
	{
		std::filesystem::path path;		// The path itself. Can be relative or absolute.
		std::filesystem::path pathFull;	// Absolute version of all paths.
		std::string key;				// Unique key of the dir.
		bool active{ false };			// Whether this path selected to be considered.
		bool checked{ false };			// Whether libraries from this path are already in the list of available models.
		SModelDir(std::filesystem::path _path, std::string _key, bool _active);
	};

	std::vector<SModelDir> m_dirsList;                 // Directories to look for libraries with models.
	std::vector<SUnitDescriptor> m_availableUnits;	   // List of available units.
	std::vector<SSolverDescriptor> m_availableSolvers; // List of available solvers.

	std::map<CBaseUnit*, DYSSOL_LIBRARY_INSTANCE> m_loadedUnits;		 // List of loaded units with their libraries. Used for proper resource management.
	std::map<CBaseSolver*, DYSSOL_LIBRARY_INSTANCE> m_loadedSolvers; // List of loaded solvers with their libraries. Used for proper resource management.

public:
	// Returns number of defined paths to look for models.
	size_t DirsNumber() const;
	// Add new path to look for models. Returns true on success.
	bool AddDir(const std::filesystem::path& _path, bool _active = true);
	// Removes the specified path with models. Returns true on success.
	bool RemoveDir(size_t _index);
	// Moves path upwards in the list. Returns true on success.
	bool UpDir(size_t _index);
	// Moves path downwards in the list. Returns true on success.
	bool DownDir(size_t _index);
	// Returns path.
	std::filesystem::path GetDirPath(size_t _index) const;
	// Returns all active paths.
	std::vector<std::filesystem::path> GetAllActiveDirPaths() const;
	// Returns all active paths as absolute paths.
	std::vector<std::filesystem::path> GetAllActiveDirFullPaths() const;
	// Returns activity of the specified path.
	bool GetDirActivity(size_t _index) const;
	// Sets activity of the specified path.
	void SetDirActivity(size_t _index, bool _active);
	// Removes all paths and models.
	void Clear();

	// Returns a list of descriptors for all available units.
	std::vector<SUnitDescriptor> GetAvailableUnits() const;
	// Returns a list of descriptors for all available solvers.
	std::vector<SSolverDescriptor> GetAvailableSolvers() const;
	// Returns descriptor of the solver by its file name. Returns empty descriptor if the specified solver is not available in manager.
	SSolverDescriptor GetSolverDescriptor(const std::wstring& _fileName) const;
	// Returns the name of the library, where the solver is stored.
	std::wstring GetSolverLibName(const std::string& _key) const;

	// Instantiates unit with provided unique key and returns a pointer to it. Returns nullptr if such unit has not been found.
	CBaseUnit* InstantiateUnit(const std::string& _key);
	// Instantiates solver with provided unique key and returns a pointer to it. Returns nullptr if such solver has not been found.
	CBaseSolver* InstantiateSolver(const std::string& _key);

	// Frees resources for the specified unit and closes a corresponding library.
	void FreeUnit(CBaseUnit* _unit);
	// Frees resources for the specified solver and closes a corresponding library.
	void FreeSolver(CBaseSolver* _solver);

private:
	// Returns a vector of unique keys of all defined dirs.
	std::vector<std::string> AllDirsKeys() const;

	// Updates models, available in active paths.
	void UpdateAvailableModels();

	// Returns a list of models available in the specified directory, treating it as relative or absolute path.
	static std::pair<std::vector<SUnitDescriptor>, std::vector<SSolverDescriptor>> GetModelsList(const std::filesystem::path& _dir);
	// Returns a list of models available in the specified directory.
	static std::pair<std::vector<SUnitDescriptor>, std::vector<SSolverDescriptor>> GetAllModelsInDir(const std::filesystem::path& _dir);

	// Tries to load unit from _library. If the model cannot be loaded, returns a structure with empty strings.
	static SUnitDescriptor TryGetUnitDescriptor(const std::filesystem::path& _pathToUnit, DYSSOL_LIBRARY_INSTANCE _library);
	// Tries to load solver from _library. If the solver cannot be loaded, returns a structure with empty strings.
	static SSolverDescriptor TryGetSolverDescriptor(const std::filesystem::path& _pathToSolver, DYSSOL_LIBRARY_INSTANCE _library);

	// Loads library from a file with the specified path.
	static DYSSOL_LIBRARY_INSTANCE LoadDyssolLibrary(const std::filesystem::path& _libPath);
	// Returns address of the specified function from provided library.
	static DYSSOL_CREATE_FUNCTION_TYPE LoadDyssolLibraryConstructor(DYSSOL_LIBRARY_INSTANCE _lib, const std::string& _funName);
	// Closes specified library.
	static void CloseDyssolLibrary(DYSSOL_LIBRARY_INSTANCE _lib);
};

