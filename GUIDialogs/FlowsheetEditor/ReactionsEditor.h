/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_ReactionsEditor.h"
#include "ChemicalReaction.h"

class CReactionsEditor : public QDialog
{
	Q_OBJECT
	Ui::CReactionsEditor ui{};

	enum ECol : int
	{
		NAME = 0, WIDGET = 1
	};

	std::vector<std::unique_ptr<CChemicalReaction>> m_reactions;	// Reactions.
	const std::vector<std::string>* m_compounds{ nullptr };			// All available compounds.
	const CMaterialsDatabase* m_database{ nullptr };				// Materials database.

public:
	CReactionsEditor(const std::vector<CChemicalReaction>& _reactions, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent = nullptr);

	void setVisible(bool _visible) override;

	[[nodiscard]] std::vector<CChemicalReaction> GetReactions() const;	// Returns reactions in the current state.

private:
	void InitializeConnections() const;					// Connects all signals.

	void UpdateWholeView();								// Updates all.

	void AddReaction();									// Adds new reaction.
	void RemoveReaction();								// Removes selected reaction.
	void UpReaction();									// Moves selected reaction upwards in the list.
	void DownReaction();								// Moves selected reaction downwards in the list.

	void AddReactionItem(CChemicalReaction* _reaction);	// Adds new reaction item to the list.
	void OnNameChange();								// Is called when user changes name of the reaction.

	void ResizeTable() const;							// Resizes the table to fit data.
};
