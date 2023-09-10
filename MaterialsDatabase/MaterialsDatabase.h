/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Compound.h"
#include "Interaction.h"
#include "DyssolFilesystem.h"

// Description of parameters of all compounds.
class CMaterialsDatabase
{
	enum class ETXTFileKeys : unsigned
	{
		PROPERTY_TYPE                        = 10,
		PROPERTY_KEY                         = 11,
		PROPERTY_NAME                        = 12,
		PROPERTY_UNITS                       = 13,
		PROPERTY_INIT_VALUE                  = 14,
		PROPERTY_DESCRIPTION                 = 15,
		COMPOUND_KEY                         = 21,
		COMPOUND_NAME                        = 22,
		COMPOUND_DESCRIPTION                 = 23,
		COMPOUND_CONST_PROPERTY              = 26,
		COMPOUND_TPD_PROPERTY                = 27,
		INTERACTION_KEYS                     = 31,
		INTERACTION_PROPERTY                 = 32,
		CONST_PROPERTY_DESCRIPTION           = 41,
		TPD_PROPERTY_DESCRIPTION             = 42,
		CORRELATION_DESCRIPTION              = 43,
		INTERACTION_TPD_PROPERTY_DESCRIPTION = 44,
		INTERACTION_CORRELATION_DESCRIPTION  = 45
	};
	enum class ETXTFileKeysOld : unsigned
	{
		MDB_PROPERTY      = 10,	// Start of property entity
		MDB_PROPERTY_SOA  = 11,
		MDB_COMPOUND      = 20,	// Start of compound entity
		COMPOUND_KEY      = 21,
		COMPOUND_NAME     = 22,
		COMPOUND_PROPERTY = 26
	};

	MDBDescriptors::constDescr activeConstProperties;
	MDBDescriptors::tpdepDescr activeTPDepProperties;
	MDBDescriptors::interDescr activeInterProperties;

	std::filesystem::path m_sFileName;			// Current file where the database is stored.
	std::vector<CCompound> m_vCompounds;				// List of defined compounds.
	std::vector<CInteraction> m_vInteractions;	// List of defined interactions between each pair of defined compounds.

public:
	CMaterialsDatabase();

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with properties

	// Currently available constant properties.
	MDBDescriptors::constDescr ActiveConstProperties() const;
	// Currently available TP-dependent properties.
	MDBDescriptors::tpdepDescr ActiveTPDepProperties() const;
	// Currently available interaction properties.
	MDBDescriptors::interDescr ActiveInterProperties() const;

	// Adds new property to active properties. If this property already exists, updates the data.
	void AddProperty(const MDBDescriptors::SPropertyDescriptor& _descriptor);
	// Removes the specified property from active properties.
	void RemoveProperty(const MDBDescriptors::EPropertyType& _type, unsigned _key);

	// Determines, whether a property with the specified key is present in materials database.
	bool IsPropertyDefined(unsigned _key) const;

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with file

	// Returns the name of the current database file.
	std::filesystem::path GetFileName() const;

	// Creates new database by removing information about compounds and file name.
	void Clear();

	// Saves database to a text file with specified name. If the name is not specified, data will be written to the default file. Returns true on success.
	bool SaveToFile(const std::filesystem::path& _fileName = "");
	// Loads database from a text file with specified name. If the name is not specified, data will be loaded from the default file. Returns true on success.
	bool LoadFromFile(const std::filesystem::path& _fileName = "");
	// Loads database from the file. Loads file with old syntax for versions before v0.7. Returns true on success.
	bool LoadFromFileV0(std::ifstream& _file);
	// Loads database from the file. Loads file with old syntax for versions before v0.9.1. Returns true on success.
	bool LoadFromFileV2(std::ifstream& _file);
	// Loads database from the file.
	bool LoadFromFileV3(std::ifstream& _file);


	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with compounds

	// Returns number of defined compounds.
	size_t CompoundsNumber() const;

	// Creates new compound and returns pointer to it.  If no key is specified or the key already exists in the database, new unique key is generated.
	CCompound* AddCompound(const std::string& _sCompoundUniqueKey = "");
	// Creates new compound as a copy of specified one. Returns pointer to created compound.
	CCompound* AddCompound(const CCompound& _compound);
	// Removes compound with specified index from the database.
	void RemoveCompound(size_t _iCompound);
	// Removes compound with specified key from the database.
	void RemoveCompound(const std::string& _sCompoundUniqueKey);
	// Moves selected compound upwards in the list of compounds.
	void ShiftCompoundUp(size_t _iCompound);
	// Moves selected compound upwards in the list of compounds.
	void ShiftCompoundUp(const std::string& _sCompoundUniqueKey);
	// Moves selected compound downwards in the list of compounds.
	void ShiftCompoundDown(size_t _iCompound);
	// Moves selected compound downwards in the list of compounds.
	void ShiftCompoundDown(const std::string& _sCompoundUniqueKey);

	// Returns index of a compound with specified key. Returns -1 if such compound has not been defined.
	size_t GetCompoundIndex(const std::string& _sCompoundUniqueKey) const;

	// Returns pointer to a compound with specified index. Returns nullptr if such compound has not been defined.
	CCompound* GetCompound(size_t _iCompound);
	// Returns const pointer to a compound with specified index. Returns nullptr if such compound has not been defined.
	const CCompound* GetCompound(size_t _iCompound) const;
	// Returns pointer to a compound with specified key. Returns nullptr if such compound has not been defined.
	CCompound* GetCompound(const std::string& _sCompoundUniqueKey);
	// Returns const pointer to a compound with specified key. Returns nullptr if such compound has not been defined.
	const CCompound* GetCompound(const std::string& _sCompoundUniqueKey) const;
	// Returns pointer to a compound with specified name. Returns nullptr if such compound has not been defined.
	CCompound* GetCompoundByName(const std::string& _sCompoundName);
	// Returns const pointer to a compound with specified name. Returns nullptr if such compound has not been defined.
	const CCompound* GetCompoundByName(const std::string& _sCompoundName) const;

	// Returns names of compounds with the specified unique keys.
	std::vector<std::string> GetCompoundsNames(const std::vector<std::string>& _keys) const;
	// Returns names of all defined compounds.
	[[nodiscard]] std::vector<std::string> GetCompoundsNames() const;
	// Returns the list of compounds keys that have been defined in the database.
	[[nodiscard]] std::vector<std::string> GetCompoundsKeys() const;

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with properties values

	// Returns value of a constant property for specified compound. Returns 0 if such property doesn't exist.
	double GetConstPropertyValue(const std::string& _sCompoundUniqueKey, ECompoundConstProperties _nConstPropType) const;
	// Returns value of a temperature/pressure-dependent property by specified temperature [K] and pressure [Pa] for specified compound. Returns 0 if such property doesn't exist.
	double GetTPPropertyValue(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType, double _dT, double _dP) const;
	// Returns value of an interaction property by specified temperature [K] and pressure [Pa] between specified compounds. Returns 0 if such property doesn't exist.
	double GetInteractionPropertyValue(const std::string& _sCompoundUniqueKey1, const std::string& _sCompoundUniqueKey2, EInteractionProperties _nInterPropType, double _dT, double _dP) const;

	// Returns boundaries of the temperature interval, on which specified property of the compound is defined. Returns interval (-1;-1) on error.
	SInterval GetTPPropertyTInterval(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType) const;
	// Returns boundaries of the pressure interval, on which specified property of the compound is defined. Returns interval (-1;-1) on error.
	SInterval GetTPPropertyPInterval(const std::string& _sCompoundUniqueKey, ECompoundTPProperties _nTPPropType) const;

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with interactions

	// Returns number of defined interactions.
	size_t InteractionsNumber() const;

	/// Returns index of an interaction between compounds with specified keys. If not found -1 is returned.
	size_t GetInteractionIndex(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2) const;

	// Returns pointer to an interaction with specified index. Returns nullptr if such interaction has not been defined.
	CInteraction* GetInteraction(size_t _iInteraction);
	// Returns const pointer to an interaction with specified index. Returns nullptr if such interaction has not been defined.
	const CInteraction* GetInteraction(size_t _iInteraction) const;
	// Returns pointer to an interaction between compounds with specified keys. Returns nullptr if such interaction has not been defined.
	CInteraction* GetInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2);
	// Returns const pointer to an interaction between compounds with specified keys. Returns nullptr if such interaction has not been defined.
	const CInteraction* GetInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2) const;


	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with interactions values

	// Returns value of an interaction property between specified compounds for selected T[K] and P[Pa]. Returns 0 if such property doesn't exist.
	double GetInteractionValue(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2, EInteractionProperties _nInterPropType, double _dT, double _dP) const;


private:
	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with interactions

	// Creates new Interaction between two compounds and returns pointer to it. If such interaction already exists, pointer to it will be returned.
	CInteraction* AddInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2);
	// Removes Interaction between compounds with specified keys from database.
	void RemoveInteraction(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2);
	// Removes Interaction with specified index from database.
	void RemoveInteraction(size_t _iInteraction);
	// Adds default interactions with specified compound, if doesn't exist.
	void ConformInteractionsAdd(const std::string& _sCompoundKey);
	// Removes interactions with specified compound, if exists.
	void ConformInteractionsRemove(const std::string& _sCompoundKey);

	//////////////////////////////////////////////////////////////////////////
	/// Auxiliary function to work with text files

	// Returns a string formatted as a comment.
	static std::string Comment(const std::string& _s);
};

