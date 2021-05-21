/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ReactionWidget.h"
#include "ContainerFunctions.h"
#include "MaterialsDatabase.h"
#include "DyssolUtilities.h"
#include "SignalBlocker.h"
#include <QScrollBar>

CReactionWidget::CReactionWidget(CChemicalReaction* _reaction, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent)
	: QWidget{ _parent }
	, m_reaction{ _reaction }
	, m_compounds{ _compounds }
	, m_database{ _database }
{
	ui.setupUi(this);
	m_radioGroup.setExclusive(true);
	InitializeConnections();
	UpdateWholeView();
}

void CReactionWidget::InitializeConnections() const
{
	connect(ui.buttonAddSubstance   , &QPushButton::clicked             , this, &CReactionWidget::AddSubstance   );
	connect(ui.buttonRemoveSubstance, &QPushButton::clicked             , this, &CReactionWidget::RemoveSubstance);
	connect(ui.tableSubstances      , &CQtTable::itemChanged            , this, &CReactionWidget::ApplyChanges   );
	connect(ui.tableSubstances      , &CQtTable::RadioButtonStateChanged, this, &CReactionWidget::ApplyChanges   );
	connect(ui.tableSubstances      , &CQtTable::ComboBoxIndexChanged   , this, &CReactionWidget::ApplyChanges   );
}

void CReactionWidget::UpdateWholeView()
{
	if (!m_reaction) return;
	[[maybe_unused]] CSignalBlocker blocker{ ui.tableSubstances, &m_radioGroup };

	// clean radio buttons
	for (auto* b : m_radioGroup.buttons())
		m_radioGroup.removeButton(b);

	// currently selected rows
	const auto selection = ui.tableSubstances->CurrentCellPos();

	// update substances
	ui.tableSubstances->clearContents();
	auto substances = m_reaction->GetSubstances();
	ui.tableSubstances->setRowCount(static_cast<int>(substances.size()));
	for (int i = 0; i < static_cast<int>(substances.size()); ++i)
	{
		auto* radio = ui.tableSubstances->SetRadioButton(i, ECols::BASE, static_cast<int>(m_reaction->GetBaseSubstanceIndex()) == i);
		m_radioGroup.addButton(radio, i);
		ui.tableSubstances->SetItemEditable(i, ECols::NU, std::abs(substances[i]->nu));
		ui.tableSubstances->SetComboBox(i, ECols::MATERIAL, m_database->GetCompoundsNames(*m_compounds), *m_compounds, substances[i]->key);
		ui.tableSubstances->SetComboBox(i, ECols::PHASE, { "", "Solid", "Liquid", "Gas" }, vector_cast<size_t>(std::vector{ EPhase::UNDEFINED, EPhase::SOLID, EPhase::LIQUID, EPhase::VAPOR }), E2I(substances[i]->phase));
		ui.tableSubstances->SetComboBox(i, ECols::TYPE, { "Reactant", "Product" }, vector_cast<size_t>(std::vector{ CChemicalReaction::ESubstance::REACTANT, CChemicalReaction::ESubstance::PRODUCT }), E2I(substances[i]->GetType()));
		ui.tableSubstances->SetItemEditable(i, ECols::ORDER, std::abs(substances[i]->order));
	}

	// restore selection
	ui.tableSubstances->RestoreSelectedCell(selection);

	ui.tableSubstances->resizeColumnsToContents();
	ui.tableSubstances->resizeRowsToContents();
}

void CReactionWidget::AddSubstance()
{
	if (!m_reaction) return;
	m_reaction->AddSubstance({ "", -1, 0, EPhase::SOLID });
	UpdateWholeView();
	emit SubstancesNumberChanged();
}

void CReactionWidget::RemoveSubstance()
{
	if (!m_reaction) return;
	m_reaction->RemoveSubstance(ui.tableSubstances->currentRow());
	UpdateWholeView();
	emit SubstancesNumberChanged();
}

void CReactionWidget::ApplyChanges() const
{
	if (!m_reaction) return;

	m_reaction->SetBaseSubstance(m_radioGroup.checkedId());
	auto substances = m_reaction->GetSubstances();
	for (int i = 0; i < static_cast<int>(substances.size()); ++i)
	{
		substances[i]->nu = ui.tableSubstances->GetItem(i, ECols::NU).toDouble();
		substances[i]->order = ui.tableSubstances->GetItem(i, ECols::ORDER).toDouble();
		// reactants must be negative, products must bo positive
		if (ui.tableSubstances->GetComboBox(i, ECols::TYPE)->currentData().toUInt() != static_cast<uint>(substances[i]->GetType()))
			substances[i]->nu = -substances[i]->nu;
		if (ui.tableSubstances->GetComboBox(i, ECols::MATERIAL)->currentIndex() != -1)
			substances[i]->key = ui.tableSubstances->GetComboBox(i, ECols::MATERIAL)->currentData().toString().toStdString();
		if (ui.tableSubstances->GetComboBox(i, ECols::PHASE)->currentIndex() != -1)
			substances[i]->phase = static_cast<EPhase>(ui.tableSubstances->GetComboBox(i, ECols::PHASE)->currentData().toUInt());
	}
}

QSize CReactionWidget::sizeHint() const
{
	if (!m_reaction) return QWidget::sizeHint();
	const int number = static_cast<int>(m_reaction->GetSubstancesNumber());
	const int constHeight = ui.layoutButtons->sizeHint().height() + ui.tableSubstances->horizontalHeader()->height() + ui.verticalLayout->spacing() + ui.verticalLayout->contentsMargins().top() + ui.verticalLayout->contentsMargins().bottom() + 4;
	const int varHeight = number != 0 ? ui.tableSubstances->verticalHeader()->sectionSize(0) * number : 0;
	const int scrollHeight = ui.tableSubstances->horizontalScrollBar()->isVisible() ? ui.tableSubstances->horizontalScrollBar()->height() : 0;
	const int w = ui.tableSubstances->horizontalHeader()->width();
	const int h = constHeight + varHeight + scrollHeight;
	return QSize{ w, h };
}
