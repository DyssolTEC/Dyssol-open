/* Copyright (c) 2021, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <string>
#include <vector>
#include <memory>
#include <utility>

class CMaterialsDatabase;
class CH5Handler;

/**
 * \brief Description of the chemical reaction.
 */
class CChemicalReaction
{
	static const unsigned m_saveVersion{ 0 }; // Current version of the saving procedure.

public:
	/**
	 * \brief Substance type.
	 */
	enum class ESubstance
	{
		REACTANT, ///< Educt substance.
		PRODUCT,  ///< Product substance.
	};

	/**
	 * \brief Description of the chemical substance.
	 */
	struct SChemicalSubstance
	{
		std::string key;					///< Unique key of the chemical substance from MDB.
		double nu = 1.0;					///< Stoichiometric coefficient of this substance in the reaction.
		double order = 0.0;					///< Partial order of reaction for this substance.
		EPhase phase = EPhase::UNDEFINED;	///< Phase of this substance.

		double MM = 0.0;					///< Molar mass of this substance.

		/**
		 * \brief Default constructor.
		 */
		SChemicalSubstance() = default;
		/**
		 * \brief Creates a new substance.
		 * \param _key Unique key of the chemical substance from MDB.
		 * \param _nu Stoichiometric coefficient of the substance in the reaction. >0 - product, <0 - reactant.
		 * \param _order Partial order of reaction for the substance.
		 * \param _phase Phase of the substance.
		 */
		SChemicalSubstance(std::string _key, double _nu, double _order, EPhase _phase) : key{ std::move(_key) }, nu{ _nu }, order{ _order }, phase{ _phase } {}
		/**
		 * \brief Equality comparison.
		 * \param _other Second object.
		 * \return Whether objects are equal.
		 */
		bool operator==(const SChemicalSubstance& _other) const { return key == _other.key && nu == _other.nu && order == _other.order && phase == _other.phase; }
		/**
		 * \brief Inequality comparison.
		 * \param _other Second object.
		 * \return Whether objects are not equal.
		 */
		bool operator!=(const SChemicalSubstance& _other) const { return !(*this == _other); }
		/**
		 * \brief Returns type of the substance based on the value of stoichiometric coefficient.
		 * \return Type of the substance in the reaction.
		 */
		[[nodiscard]] ESubstance GetType() const { return nu < 0 ? ESubstance::REACTANT : ESubstance::PRODUCT; }
	};

private:
	std::string m_name;												///< Name of the reaction.
	double m_enthalpy{ 0.0 };										///< Specific reaction enthalpy [J/mol].
	std::vector<std::unique_ptr<SChemicalSubstance>> m_substances;	///< Chemical substances taking part in the reaction.
	size_t m_iBase = -1;											///< Index of the base substance.

public:
	/**
	 * \brief Default constructor.
	 */
	CChemicalReaction() = default;
	/**
	 * \brief Copy constructor.
	 * \param _other Target chemical reaction.
	 */
	CChemicalReaction(const CChemicalReaction& _other);
	/**
	 * \brief Copy assignment operator.
	 * \param _other Target chemical reaction.
	 * \return Reference to this reaction.
	 */
	CChemicalReaction& operator=(const CChemicalReaction& _other);
	/**
	 * \brief Move constructor.
	 * \param _other Target chemical reaction.
	 */
	CChemicalReaction(CChemicalReaction&& _other) = default;
	/**
	 * \brief Move assignment operator.
	 * \param _other Target chemical reaction.
	 * \return Reference to this reaction.
	 */
	CChemicalReaction& operator=(CChemicalReaction&& _other) = default;
	/**
	 * \private
	 * \brief Destructor.
	 */
	~CChemicalReaction() = default;
	/**
	 * \brief Swaps two reactions.
	 * \param _other Target chemical reaction.
	 */
	void Swap(CChemicalReaction& _other) noexcept;

	/**
	 * \brief Comparison.
	 * \param _other Second object.
	 * \return Whether objects are equal.
	 */
	bool operator==(const CChemicalReaction& _other) const;

	/**
	 * \brief Sets name of the reaction.
	 * \param _name Reaction name.
	 */
	void SetName(const std::string& _name);
	/**
	 * \brief Returns name of the reaction.
	 * \return Reaction name.
	 */
	[[nodiscard]] std::string GetName() const;

	/**
	 * \brief Adds a new empty chemical substance to the reaction.
	 * \return Pointer to the added reaction.
	 */
	SChemicalSubstance* AddSubstance();
	/**
	 * \brief Adds a new chemical substance to the reaction.
	 * \param _substance New substance.
	 */
	void AddSubstance(const SChemicalSubstance& _substance);
	/**
	 * \brief Removes substance with the specified index.
	 * \param _index Index of the substance.
	 */
	void RemoveSubstance(size_t _index);
	/**
	 * \brief Returns all defined substances.
	 * \return Constant pointers to all defined substances.
	 */
	[[nodiscard]] std::vector<const SChemicalSubstance*> GetSubstances() const;
	/**
	 * \brief Returns all defined substances.
	 * \return Pointers to all defined substances.
	 */
	std::vector<SChemicalSubstance*> GetSubstances();
	/**
	 * \brief Returns number of defined substances.
	 * \return Number of defined substances.
	 */
	[[nodiscard]] size_t GetSubstancesNumber() const;

	/**
	 * \brief Returns all defined substances of the specified phase.
	 * \param _phase Target phase.
	 * \return Constant pointers to substances.
	 */
	[[nodiscard]] std::vector<const SChemicalSubstance*> GetSubstances(EPhase _phase) const;
	/**
	 * \brief Returns all defined substances of the specified phase.
	 * \param _phase Target phase.
	 * \return Pointers to substances.
	 */
	std::vector<SChemicalSubstance*> GetSubstances(EPhase _phase);

	/**
	 * \brief Returns all defined substances of the specified type.
	 * \param _type Type of the substance.
	 * \return Constant pointers to substances.
	 */
	[[nodiscard]] std::vector<const SChemicalSubstance*> GetSubstances(ESubstance _type) const;
	/**
	 * \brief Returns all defined substances of the specified type.
	 * \param _type Type of the substance.
	 * \return Pointers to substances.
	 */
	std::vector<SChemicalSubstance*> GetSubstances(ESubstance _type);

	/**
	 * \brief Returns all defined substances of the specified phase and type.
	 * \param _phase Target phase.
	 * \param _type Type of the substance.
	 * \return Constant pointers to substances.
	 */
	[[nodiscard]] std::vector<const SChemicalSubstance*> GetSubstances(EPhase _phase, ESubstance _type) const;
	/**
	 * \brief Returns all defined substances of the specified phase and type.
	 * \param _phase Target phase.
	 * \param _type Type of the substance.
	 * \return Pointers to substances.
	 */
	std::vector<SChemicalSubstance*> GetSubstances(EPhase _phase, ESubstance _type);

	/**
	 * \brief Sets index of the base substance.
	 * \param _index Index of the substance.
	 */
	void SetBaseSubstance(size_t _index);
	/**
	 * \brief Returns base substance.
	 * \return Pointer to the substance.
	 */
	[[nodiscard]] const SChemicalSubstance* GetBaseSubstance() const;
	/**
	 * \brief Returns base substance.
	 * \return Pointer to the substance.
	 */
	SChemicalSubstance* GetBaseSubstance();
	/**
	 * \brief Returns index of the base substance.
	 * \return Index to the substance.
	 */
	[[nodiscard]] size_t GetBaseSubstanceIndex() const;

	/**
	 * \brief Sets specific reaction enthalpy [J/mol].
	 * \param _enthalpy Reaction enthalpy.
	 */
	void SetEnthalpy(double _enthalpy);
	/**
	 * \brief Returns specific reaction enthalpy [J/mol].
	 * \return Reaction enthalpy.
	 */
	[[nodiscard]] double GetEnthalpy() const;

	/**
	 * \private
	 * \brief Initializes the reaction.
	 * \details Calculates reaction enthalpy and sets molar masses of all substances using the provided materials database.
	 * Otherwise these values must be set manually.
	 * \param _materials Reference to the materials database.
	 */
	void Initialize(const CMaterialsDatabase& _materials);

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
};

