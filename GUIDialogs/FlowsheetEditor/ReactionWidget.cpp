/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ReactionWidget.h"
#include "SubstanceWidget.h"
#include "ContainerFunctions.h"
#include "SignalBlocker.h"

CReactionWidget::CReactionWidget(CChemicalReaction* _reaction, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent)
	: QWidget{ _parent }
	, m_reaction{ _reaction }
	, m_compounds{ _compounds }
	, m_database{ _database }
	, m_substanceWidgetHeightHint{ CSubstanceWidget(nullptr, CChemicalReaction::ESubstance::REACTANT, nullptr, nullptr, this).size().height() }
{
	ui.setupUi(this);
	m_radioGroup.setExclusive(true);
	InitializeConnections();
	UpdateWholeView();
}

void CReactionWidget::InitializeConnections() const
{
	connect(ui.buttonAddReactant   , &QPushButton::clicked, this, &CReactionWidget::AddReactant   );
	connect(ui.buttonAddProduct    , &QPushButton::clicked, this, &CReactionWidget::AddProduct    );
	connect(ui.buttonRemoveReactant, &QPushButton::clicked, this, &CReactionWidget::RemoveReactant);
	connect(ui.buttonRemoveProduct , &QPushButton::clicked, this, &CReactionWidget::RemoveProduct );
}

void CReactionWidget::UpdateWholeView()
{
	if (!m_reaction) return;
	[[maybe_unused]] CSignalBlocker blocker{ ui.listReactants, ui.listProducts, &m_radioGroup };

	// clean radio buttons
	for (auto* b : m_radioGroup.buttons())
		m_radioGroup.removeButton(b);

	// currently selected rows
	const int iOldReactant = ui.listReactants->currentRow();
	const int iOldProduct  = ui.listProducts->currentRow();

	// update substances
	ui.listReactants->clear();
	ui.listProducts->clear();
	auto substances = m_reaction->GetSubstances();
	for (size_t i = 0; i < substances.size(); ++i)
		AddListItem(substances[i], i);

	// restore selection
	ui.listReactants->RestoreCurrentRow(iOldReactant);
	ui.listProducts->RestoreCurrentRow(iOldProduct);

	// check radio buttons
	if (auto* radio = m_radioGroup.button(static_cast<int>(m_reaction->GetBaseSubstanceIndex())))
		radio->setChecked(true);
}

void CReactionWidget::ApplyBaseSubstance() const
{
	if (!m_reaction) return;
	m_reaction->SetBaseSubstance(m_radioGroup.checkedId());
}

void CReactionWidget::AddReactant()
{
	AddSubstance(CChemicalReaction::ESubstance::REACTANT);
}

void CReactionWidget::AddProduct()
{
	AddSubstance(CChemicalReaction::ESubstance::PRODUCT);
}

void CReactionWidget::RemoveReactant()
{
	RemoveSubstance(CChemicalReaction::ESubstance::REACTANT);
	emit SubstancesNumberChanged();
}

void CReactionWidget::RemoveProduct()
{
	RemoveSubstance(CChemicalReaction::ESubstance::PRODUCT);
	emit SubstancesNumberChanged();
}

void CReactionWidget::AddSubstance(CChemicalReaction::ESubstance _type)
{
	if (!m_reaction) return;
	m_reaction->AddSubstance({ "", (_type == CChemicalReaction::ESubstance::REACTANT ? -1.0 : 1.0), EPhase::UNDEFINED });
	UpdateWholeView();
	emit SubstancesNumberChanged();
}

void CReactionWidget::RemoveSubstance(CChemicalReaction::ESubstance _type)
{
	if (!m_reaction) return;
	CQtList* list = _type == CChemicalReaction::ESubstance::REACTANT ? ui.listReactants : ui.listProducts;
	if (!list || !list->currentItem()) return;
	m_reaction->RemoveSubstance(list->CurrentItemUserDataUInt());
	UpdateWholeView();
}

void CReactionWidget::AddListItem(CChemicalReaction::SChemicalSubstanse* _substance, size_t _index)
{
	if (!m_reaction) return;
	const auto type = _substance->GetType();
	QListWidget* list = type == CChemicalReaction::ESubstance::REACTANT ? ui.listReactants : ui.listProducts;
	[[maybe_unused]] CSignalBlocker block{ list, &m_radioGroup };
	auto* widget = new CSubstanceWidget(_substance, type, m_compounds, m_database, this);
	auto* item = new QListWidgetItem();
	item->setData(Qt::UserRole, _index);
	item->setSizeHint(widget->minimumSizeHint());
	list->addItem(item);
	list->setItemWidget(item, widget);
	auto* radio = widget->GetRadioBase();
	m_radioGroup.addButton(radio, static_cast<int>(VectorFind(m_reaction->GetSubstances(), _substance)));
	connect(radio, &QRadioButton::clicked, this, &CReactionWidget::ApplyBaseSubstance);
}

QSize CReactionWidget::sizeHint() const
{
	if (!m_reaction) return QWidget::sizeHint();
	const int number = static_cast<int>(std::max(m_reaction->GetSubstances(CChemicalReaction::ESubstance::PRODUCT).size(), m_reaction->GetSubstances(CChemicalReaction::ESubstance::REACTANT).size()));
	const int constHeight = ui.horizontalLayoutReactants->sizeHint().height() + ui.gridLayout->verticalSpacing() + ui.gridLayout->contentsMargins().top() + ui.gridLayout->contentsMargins().bottom() + 4;
	const int varHeight = std::max(m_substanceWidgetHeightHint * number, m_substanceWidgetHeightHint);
	const int w = width();
	const int h = constHeight + varHeight;
	return QSize{ w, h };
}
