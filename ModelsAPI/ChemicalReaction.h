/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <string>
#include <vector>
#include <memory>
#include <utility>

class CMaterialsDatabase;
class CH5Handler;

class CChemicalReaction
{
	static const unsigned m_saveVersion{ 0 }; // Current version of the saving procedure.

public:
	enum class ESubstance
	{
		REACTANT, PRODUCT
	};

	struct SChemicalSubstanse
	{
		std::string key;					// Unique key of the chemical substance from MDB.
		double nu{ 1.0 };					// Stoichiometric coefficient of this substance in the reaction.
		EPhase phase{ EPhase::UNDEFINED };	// Phase of this substance.

		double MM{};						// Molar mass of this substance.

		SChemicalSubstanse() = default;
		// Creates a new substance. _nu > 0: product, _nu < 0: reactant.
		SChemicalSubstanse(std::string _key, double _nu, EPhase _phase) : key{ std::move(_key) }, nu{ _nu }, phase{ _phase } {}
		// Returns type of the substance based on the value of stoichiometric coefficient.
		[[nodiscard]] ESubstance GetType() const { return nu < 0 ? ESubstance::REACTANT : ESubstance::PRODUCT; }
	};

private:
	std::string m_name;												// Name of the reaction.
	double m_enthalpy{ 0.0 };										// Specific reaction enthalpy [J/mol].
	std::vector<std::unique_ptr<SChemicalSubstanse>> m_substances;	// Chemical substances taking part in the reaction.
	size_t m_iBase = -1;											// Index of the base substance.

public:
	CChemicalReaction() = default;
	CChemicalReaction(const CChemicalReaction& _other);
	CChemicalReaction& operator=(const CChemicalReaction& _other);
	CChemicalReaction(CChemicalReaction&& _other) = default;
	CChemicalReaction& operator=(CChemicalReaction&& _other) = default;
	~CChemicalReaction() = default;
	// Swaps two reactions.
	void Swap(CChemicalReaction& _other) noexcept;

	// Sets name of the reaction.
	void SetName(const std::string& _name);
	// Returns name of the reaction.
	[[nodiscard]] std::string GetName() const;

	// Adds a new empty chemical substance to the reaction.
	SChemicalSubstanse* AddSubstance();
	// Adds a new chemical substance to the reaction.
	void AddSubstance(const SChemicalSubstanse& _substance);
	// Removes substance with the specified index.
	void RemoveSubstance(size_t _index);
	// Returns all defined substances.
	[[nodiscard]] std::vector<const SChemicalSubstanse*> GetSubstances() const;
	// Returns all defined substances.
	std::vector<SChemicalSubstanse*> GetSubstances();
	// Returns number of defined substances.
	[[nodiscard]] size_t GetSubstancesNumber() const;

	// Returns all defined substances of the specified phase.
	[[nodiscard]] std::vector<const SChemicalSubstanse*> GetSubstances(EPhase _phase) const;
	// Returns all defined substances of the specified phase.
	std::vector<SChemicalSubstanse*> GetSubstances(EPhase _phase);

	// Returns all defined substances of the specified type.
	[[nodiscard]] std::vector<const SChemicalSubstanse*> GetSubstances(ESubstance _type) const;
	// Returns all defined substances of the specified type.
	std::vector<SChemicalSubstanse*> GetSubstances(ESubstance _type);

	// Returns all defined substances of the specified phase and type.
	[[nodiscard]] std::vector<const SChemicalSubstanse*> GetSubstances(EPhase _phase, ESubstance _type) const;
	// Returns all defined substances of the specified phase and type.
	std::vector<SChemicalSubstanse*> GetSubstances(EPhase _phase, ESubstance _type);

	// Sets index of the base substance.
	void SetBaseSubstance(size_t _index);
	// Returns base substance.
	[[nodiscard]] const SChemicalSubstanse* GetBaseSubstance() const;
	// Returns base substance.
	SChemicalSubstanse* GetBaseSubstance();
	// Returns index of the base substance.
	[[nodiscard]] size_t GetBaseSubstanceIndex() const;

	// Sets specific reaction enthalpy [J/mol].
	void SetEnthalpy(double _enthalpy);
	// Returns specific reaction enthalpy [J/mol].
	[[nodiscard]] double GetEnthalpy() const;

	// Calculates reaction enthalpy and sets molar masses of all substances using the provided materials database. Otherwise these values must be set manually.
	void Initialize(const CMaterialsDatabase& _materials);

	// Saves the reaction into the HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads the reaction from the HDF5 file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};

