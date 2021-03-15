/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SubstanceWidget.h"
#include "ChemicalReaction.h"
#include "MaterialsDatabase.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "SignalBlocker.h"

CSubstanceWidget::CSubstanceWidget(CChemicalReaction::SChemicalSubstanse* _substance, CChemicalReaction::ESubstance _type, const std::vector<std::string>* _compounds, const CMaterialsDatabase* _database, QWidget* _parent)
	: QWidget{ _parent }
	, m_substance{ _substance }
	, m_type{ _type }
	, m_compounds{ _compounds }
	, m_database{ _database }
{
	ui.setupUi(this);

	ui.lineCoefficient->setValidator(new QDoubleValidator{ 0.0, std::numeric_limits<double>::max(), 5, this });
	CreatePhaseCombo();
	InitializeConnections();
	UpdateWholeView();
}

void CSubstanceWidget::InitializeConnections()
{
	connect(ui.lineCoefficient, &QLineEdit::editingFinished                        , this, &CSubstanceWidget::ApplyChanges);
	connect(ui.comboMaterial  , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSubstanceWidget::ApplyChanges);
	connect(ui.comboPhase     , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSubstanceWidget::ApplyChanges);
}

void CSubstanceWidget::CreatePhaseCombo() const
{
	ui.comboPhase->addItem(" "     , E2I(EPhase::UNDEFINED));
	ui.comboPhase->addItem("Solid" , E2I(EPhase::SOLID    ));
	ui.comboPhase->addItem("Liquid", E2I(EPhase::LIQUID   ));
	ui.comboPhase->addItem("Gas"   , E2I(EPhase::VAPOR    ));
}

void CSubstanceWidget::UpdateWholeView() const
{
	if (!m_substance) return;
	[[maybe_unused]] CSignalBlocker block{ ui.lineCoefficient, ui.comboMaterial, ui.comboPhase };

	// stoichiometric coefficient
	ui.lineCoefficient->setText(QString::number(std::abs(m_substance->nu)));

	// material
	ui.comboMaterial->clear();
	for (const auto& key : *m_compounds)
	{
		const auto* material = m_database->GetCompound(key);
		const QString name = material ? QString::fromStdString(material->GetName()) : QString("UNKNOWN");
		ui.comboMaterial->addItem(name, QString::fromStdString(key));
	}
	ui.comboMaterial->setCurrentIndex(ui.comboMaterial->findData(QString::fromStdString(m_substance->key)));

	// phase
	ui.comboPhase->setCurrentIndex(ui.comboPhase->findData(E2I(m_substance->phase)));
}

QRadioButton* CSubstanceWidget::GetRadioBase() const
{
	return ui.radioBase;
}


void CSubstanceWidget::ApplyChanges() const
{
	if (!m_substance) return;

	m_substance->nu = ui.lineCoefficient->text().toDouble();
	// reactants must be negative, products must bo positive
	if (m_type != m_substance->GetType())
		m_substance->nu = -m_substance->nu;
	if (ui.comboMaterial->currentIndex() != -1)
		m_substance->key = ui.comboMaterial->currentData().toString().toStdString();
	if (ui.comboPhase->currentIndex() != -1)
		m_substance->phase = static_cast<EPhase>(ui.comboPhase->currentData().toUInt());
}
