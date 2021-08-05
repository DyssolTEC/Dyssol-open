/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ModelsManager.h"
#include "DyssolTypes.h"

class CMultidimensionalGrid;
class CMaterialsDatabase;
class CH5Handler;

// TODO: rename to CUnit.

/* A container wrapping a unit's model in the flowsheet structure. Is responsible for loading/unloading units and solvers from dynamic libraries.
 * This is actually a Unit - an entity of the flowsheet. Each Unit is filled with a Model (CBaseUnit) that represents its structure and functionality. */
class CUnitContainer
{
	static const unsigned m_saveVersion{ 2 }; // Current version of the saving procedure.

	std::string m_name{ "Unit" };		// Name of the unit container - a unit name displayed in the flowsheet.
	std::string m_uniqueID;				// The unique identifier of the unit.

	CBaseUnit* m_model{ nullptr };		// Pointer to a contained model.

	CModelsManager& m_modelsManager;	// A holder of all accessible models.

	// TODO: rename to m_grid

	////////////////////////////////////////////////////////////////////////////////
	// References to flowsheet structural data and settings
	//

	// TODO: gather them in some global structure.
	const CMaterialsDatabase& m_materialsDB;			// Reference to a database of materials.
	const CMultidimensionalGrid& m_grid;				// Reference to the main distributions grid.
	const std::vector<SOverallDescriptor>& m_overall;	// Reference to overall properties.
	const std::vector<SPhaseDescriptor>& m_phases;		// Reference to phases.
	const SCacheSettings& m_cache;						// Reference to cache settings.
	const SToleranceSettings& m_tolerance;				// Reference to tolerance settings.
	const SThermodynamicsSettings& m_thermodynamics;	// Reference to thermodynamics settings.

public:
	// Basic constructor.
	CUnitContainer(const std::string& _id, CModelsManager& _modelsManager,
		const CMaterialsDatabase& _materialsDB, const CMultidimensionalGrid& _grid, const std::vector<SOverallDescriptor>& _overall,
		const std::vector<SPhaseDescriptor>& _phases, const SCacheSettings& _cache, const SToleranceSettings& _tolerance, const SThermodynamicsSettings& _thermodynamics);
	CUnitContainer(const CUnitContainer& _other)            = delete;
	CUnitContainer(CUnitContainer&& _other)                 = delete;
	CUnitContainer& operator=(const CUnitContainer& _other) = delete;
	CUnitContainer& operator=(CUnitContainer&& _other)      = delete;
	// Basic destructor.
	~CUnitContainer();

	// Returns a name of the unit.
	std::string GetName() const;
	// Sets a new name of the unit.
	void SetName(const std::string& _name);

	// Returns a unique key of the unit.
	std::string GetKey() const;
	// Sets a new unique key of the unit.
	void SetKey(const std::string& _id);

	// Sets new model with a specified unique ID. If the unit already contains a model, it is removed and a new instance is created.
	void SetModel(const std::string& _uniqueID);
	// Returns a pointer to a contained model.
	const CBaseUnit* GetModel() const;
	// Returns a pointer to a contained model.
	CBaseUnit* GetModel();

	// Initializes all solvers chosen in unit parameters by loading them from corresponding dynamic libraries. Returns an empty string on success, otherwise returns an error description.
	std::string InitializeExternalSolvers() const;
	// Unloads all solvers from unit parameters.
	void ClearExternalSolvers() const;

	// Saves unit to HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads unit from HDF5 file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads unit from HDF5 file. A compatibility version.
	void LoadFromFile_v1(CH5Handler& _h5File, const std::string& _path);
};

