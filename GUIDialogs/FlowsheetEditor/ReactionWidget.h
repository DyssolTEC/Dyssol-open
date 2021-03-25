/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_ReactionWidget.h"
#include "ChemicalReaction.h"
#include <QButtonGroup>

class CMaterialsDatabase;

class CReactionWidget : public QWidget
{
	Q_OBJECT
	Ui::CReactionWidget ui{};

	CChemicalReaction* m_reaction{ nullptr };				// Reaction.
	const std::vector<std::string>* m_compounds{ nullptr };	// All available compounds.
	const CMaterialsDatabase* m_database{ nullptr };		// Materials database.

	QButtonGroup m_radioGroup;								// Group to manage radio buttons in substance widgets.
	const int m_substanceWidgetHeightHint;					// Hint for the substance widget height.

public:
	CReactionWidget(CChemicalReaction* _reaction, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent = nullptr);

	[[nodiscard]] QSize sizeHint() const override;

private:
	void InitializeConnections() const;	// Connects all signals.

	void UpdateWholeView();				// Updates all.

	void ApplyBaseSubstance() const;	// Sets new base substance.

	void AddReactant();					// Adds new reactant.
	void AddProduct();					// Adds new product.
	void RemoveReactant();				// Removes selected reactant.
	void RemoveProduct();				// Removes selected product.

	void AddSubstance(CChemicalReaction::ESubstance _type);								// Adds new substance of the specified type.
	void RemoveSubstance(CChemicalReaction::ESubstance _type);							// Removes selected substance of the specified type.
	void AddListItem(CChemicalReaction::SChemicalSubstanse* _substance, size_t _index);	// Adds new substance item to the proper list.

signals:
	void SubstancesNumberChanged();			// Emitted when the number of substances changes.
};
