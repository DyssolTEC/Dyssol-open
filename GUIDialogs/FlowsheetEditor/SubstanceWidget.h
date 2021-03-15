/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_SubstanceWidget.h"
#include "ChemicalReaction.h"

class CMaterialsDatabase;

class CSubstanceWidget : public QWidget
{
	Q_OBJECT
	Ui::CSubstanceWidget ui{};

	CChemicalReaction::SChemicalSubstanse* m_substance{ nullptr };	// Substance.
	CChemicalReaction::ESubstance m_type{};							// Substance type.
	const std::vector<std::string>* m_compounds{ nullptr };			// All available compounds.
	const CMaterialsDatabase* m_database{ nullptr };				// Materials database.

public:
	CSubstanceWidget(CChemicalReaction::SChemicalSubstanse* _substance, CChemicalReaction::ESubstance _type, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent = nullptr);

	[[nodiscard]] QRadioButton* GetRadioBase() const;	// Returns pointer to the radio button.

private:
	void InitializeConnections();		// Connects all signals.
	void CreatePhaseCombo() const;		// Creates combobox for phases.
	void UpdateWholeView() const;		// Updates all.
	void ApplyChanges() const;			// Updates current state of the substance according to the user's selection.
};
