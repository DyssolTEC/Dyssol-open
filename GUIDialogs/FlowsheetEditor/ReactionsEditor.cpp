/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ReactionsEditor.h"
#include "ReactionWidget.h"
#include "ContainerFunctions.h"
#include "SignalBlocker.h"
#include <utility>

CReactionsEditor::CReactionsEditor(const std::vector<CChemicalReaction>& _reactions, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent)
	: QDialog{ _parent }
	, m_compounds{ _compounds }
	, m_database{ _database }
{
	ui.setupUi(this);

	for (const auto& r : _reactions)
		m_reactions.emplace_back(new CChemicalReaction{ r });

	InitializeConnections();
	UpdateWholeView();
}

void CReactionsEditor::setVisible(bool _visible)
{
	QDialog::setVisible(_visible);
	if (_visible)
		UpdateWholeView();
}

void CReactionsEditor::InitializeConnections() const
{
	connect(ui.buttonAddReaction   , &QPushButton::clicked      , this, &CReactionsEditor::AddReaction);
	connect(ui.buttonRemoveReaction, &QPushButton::clicked      , this, &CReactionsEditor::RemoveReaction);
	connect(ui.buttonUpReaction    , &QPushButton::clicked      , this, &CReactionsEditor::UpReaction);
	connect(ui.buttonDownReaction  , &QPushButton::clicked      , this, &CReactionsEditor::DownReaction);
	connect(ui.tableReactions      , &QTableWidget::cellChanged , this, &CReactionsEditor::OnNameChange);
	connect(ui.buttonBox           , &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(ui.buttonBox           , &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CReactionsEditor::UpdateWholeView()
{
	[[maybe_unused]] CSignalBlocker block{ ui.tableReactions };
	const auto oldCell = ui.tableReactions->GetCurrentCellPos();
	ui.tableReactions->setRowCount(0);
	for (auto& r : m_reactions)
		AddReactionItem(r.get());
	ui.tableReactions->SetCurrentCellPos(oldCell);
	ResizeTable();
}

std::vector<CChemicalReaction> CReactionsEditor::GetReactions() const
{
	auto res = ReservedVector<CChemicalReaction>(m_reactions.size());
	for (const auto& r : m_reactions)
		res.push_back(*r);
	return res;
}

void CReactionsEditor::AddReaction()
{
	m_reactions.emplace_back(new CChemicalReaction);
	m_reactions.back()->SetName("Reaction " + std::to_string(m_reactions.size()));
	AddReactionItem(m_reactions.back().get());
	ResizeTable();
}

void CReactionsEditor::RemoveReaction()
{
	[[maybe_unused]] CSignalBlocker block{ ui.tableReactions };
	VectorDelete(m_reactions, static_cast<size_t>(ui.tableReactions->currentRow()));
	const auto oldCell = ui.tableReactions->GetCurrentCellPos();
	ui.tableReactions->removeRow(ui.tableReactions->currentRow());
	ui.tableReactions->SetCurrentCellPos(oldCell);
}

void CReactionsEditor::UpReaction()
{
	const auto index = static_cast<size_t>(ui.tableReactions->currentRow());
	if (index >= m_reactions.size() || index == 0) return;
	std::iter_swap(m_reactions.begin() + index, m_reactions.begin() + index - 1);
	const auto [row, col] = ui.tableReactions->GetCurrentCellPos();
	ui.tableReactions->setCurrentCell(row - 1, col);
	UpdateWholeView();
}

void CReactionsEditor::DownReaction()
{
	const auto index = static_cast<size_t>(ui.tableReactions->currentRow());
	if (index >= m_reactions.size() || index == m_reactions.size() - 1) return;
	std::iter_swap(m_reactions.begin() + index, m_reactions.begin() + index + 1);
	const auto [row, col] = ui.tableReactions->GetCurrentCellPos();
	ui.tableReactions->setCurrentCell(row + 1, col);
	UpdateWholeView();
}

void CReactionsEditor::AddReactionItem(CChemicalReaction* _reaction)
{
	[[maybe_unused]] CSignalBlocker block{ ui.tableReactions };
	auto* widget = new CReactionWidget(_reaction, m_compounds, m_database, this);
	ui.tableReactions->insertRow(ui.tableReactions->rowCount());
	ui.tableReactions->SetItemEditable(ui.tableReactions->rowCount() - 1, ECol::NAME, QString::fromStdString(_reaction->GetName()));
	ui.tableReactions->setCellWidget(ui.tableReactions->rowCount() - 1, ECol::WIDGET, widget);
	connect(widget, &CReactionWidget::SubstancesNumberChanged, this, &CReactionsEditor::ResizeTable);
}

void CReactionsEditor::OnNameChange()
{
	ResizeTable();
	const auto index = ui.tableReactions->currentRow();
	if (index == -1 || index >= static_cast<int>(m_reactions.size())) return;
	m_reactions[index]->SetName(ui.tableReactions->GetItemText(index, ECol::NAME).toStdString());
}

void CReactionsEditor::ResizeTable() const
{
	[[maybe_unused]] CSignalBlocker block{ ui.tableReactions };
	ui.tableReactions->resizeRowsToContents();
	ui.tableReactions->resizeColumnToContents(ECol::NAME);
	ui.tableReactions->horizontalHeader()->setStretchLastSection(true);
}
