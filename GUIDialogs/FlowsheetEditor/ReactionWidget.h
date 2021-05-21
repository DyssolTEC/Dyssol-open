/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_ReactionWidget.h"
#include "ChemicalReaction.h"
#include <QButtonGroup>

class CMaterialsDatabase;

class CReactionWidget : public QWidget
{
	enum ECols : int { BASE = 0, NU, MATERIAL, PHASE, TYPE, ORDER };
	Q_OBJECT
	Ui::CReactionWidget ui{};

	CChemicalReaction* m_reaction{ nullptr };				// Reaction.
	const std::vector<std::string>* m_compounds{ nullptr };	// All available compounds.
	const CMaterialsDatabase* m_database{ nullptr };		// Materials database.

	QButtonGroup m_radioGroup;								// Group to manage radio buttons in substance widgets.

public:
	CReactionWidget(CChemicalReaction* _reaction, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent = nullptr);

	[[nodiscard]] QSize sizeHint() const override;

private:
	void InitializeConnections() const;	// Connects all signals.

	void UpdateWholeView();				// Updates all.

	void AddSubstance();				// Adds new substance.
	void RemoveSubstance();				// Removes selected substance.

	void ApplyChanges() const;			// Updates current state of the reaction according to the user's selection.

signals:
	void SubstancesNumberChanged();		// Emitted when the number of substances changes.
};
