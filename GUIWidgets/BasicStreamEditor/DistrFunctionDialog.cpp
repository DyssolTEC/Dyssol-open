/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DistrFunctionDialog.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <QMessageBox>

#include "MultidimensionalGrid.h"

CDistrFunctionDialog::CDistrFunctionDialog(QWidget* parent /*= nullptr*/) : QDialog(parent)
{
	ui.setupUi(this);
	setModal(true);

	m_distrFun = EDistributionFunction::NORMAL;
	m_PSDGridType = EPSDGridType::DIAMETER;
	m_nDimType = DISTR_SIZE;
	m_pGrid = nullptr;
	m_dParam1 = 0;
	m_dParam2 = 0;
	SetDistributionsGrid(m_pGrid, m_nDimType, m_PSDGridType);

	CreateDistrFunCombo();

	connect(ui.comboBox,			QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CDistrFunctionDialog::FunctionChanged);
	connect(ui.pushButtonOk,		&QPushButton::clicked,									this, &CDistrFunctionDialog::OKClicked);
	connect(ui.pushButtonCancel,	&QPushButton::clicked,									this, &CDistrFunctionDialog::CancelClicked);
}

void CDistrFunctionDialog::setVisible(bool _bVisible)
{
	if (_bVisible && !this->isVisible())
		UpdateWholeView();
	QDialog::setVisible(_bVisible);
}

void CDistrFunctionDialog::SetDistributionsGrid(const CMultidimensionalGrid* _pGrid, EDistrTypes _nType, EPSDGridType _nPSDGridType)
{
	m_nDimType = _nType;
	if (!_pGrid) return;
	m_pGrid = _pGrid;
	m_PSDGridType = _nPSDGridType;
	if (m_nDimType != DISTR_SIZE)
		m_gridMeans = _pGrid->GetGridDimensionNumeric(_nType)->GetClassesMeans();
	else
		m_gridMeans = _pGrid->GetPSDMeans(_nPSDGridType);
}

std::vector<double> CDistrFunctionDialog::GetDistribution() const
{
	return m_vDistr;
}

void CDistrFunctionDialog::CreateDistrFunCombo() const
{
	QSignalBlocker blocker(ui.comboBox);

	for (int i = 0; i < static_cast<int>(EDistributionFunction::COUNT_); ++i)
	{
		if (i == static_cast<int>(EDistributionFunction::MANUAL))
			continue;

		const auto& descriptor = DistributionFunction::GetFunctionDescriptor(static_cast<EDistributionFunction>(i));
		ui.comboBox->insertItem(i, QString::fromStdString(std::string(descriptor.name)), i);
	}

	ui.comboBox->setCurrentIndex(static_cast<int>(m_distrFun));
}

void CDistrFunctionDialog::UpdateWholeView() const
{
	UpdateDistrFunction();
	UpdateUnits();
	UpdateParamLabels();
	UpdateParams();
}

void CDistrFunctionDialog::UpdateDistrFunction() const
{
	ui.comboBox->setCurrentIndex(static_cast<int>(m_distrFun) - 1);
}

void CDistrFunctionDialog::UpdateUnits() const
{
	const QString sUnits = m_nDimType != DISTR_SIZE ? StrConst::FUN_EmptyUnits : m_PSDGridType == EPSDGridType::DIAMETER ? StrConst::FUN_DiameterUnits : StrConst::FUN_VolumeUnits;
	ui.labelUnits1->setText(sUnits);
	if (m_distrFun == EDistributionFunction::NORMAL)
		ui.labelUnits2->setText(sUnits);
	else
		ui.labelUnits2->setText(QString(StrConst::FUN_EmptyUnits));
}

void CDistrFunctionDialog::UpdateParamLabels() const
{
	const auto& descriptor = DistributionFunction::GetFunctionDescriptor(m_distrFun);
	if (descriptor.params.size() > 0)
		ui.labelParam1->setText(QString::fromStdString(std::string(descriptor.params[0].name)));
	if (descriptor.params.size() > 1)
		ui.labelParam2->setText(QString::fromStdString(std::string(descriptor.params[1].name)));
}

void CDistrFunctionDialog::UpdateParams() const
{
	ui.lineEditParam1->setText(QString::number(m_dParam1));
	ui.lineEditParam2->setText(QString::number(m_dParam2));
}

void CDistrFunctionDialog::FunctionChanged(int _index)
{
	m_distrFun = static_cast<EDistributionFunction>(_index + 1);
	UpdateParamLabels();
	UpdateUnits();
}

void CDistrFunctionDialog::OKClicked()
{
	m_dParam1 = ui.lineEditParam1->text().toDouble();
	m_dParam2 = ui.lineEditParam2->text().toDouble();
	m_distrFun = static_cast<EDistributionFunction>(ui.comboBox->currentIndex() + 1);

	const auto& descriptor = DistributionFunction::GetFunctionDescriptor(m_distrFun);

	auto criticalMessage = [&](double _value, int _idx)
		{
			const QString& message = QString::fromStdString(StrConst::FUN_ErrorZeroParameter(descriptor.params[_idx].name.data()));
			if (_value == 0.0)
			{
				QMessageBox::critical(this, StrConst::FUN_ErrorName, message);
				return true;
			}
			return false;
		};

	switch (m_distrFun)
	{
	case EDistributionFunction::NORMAL:
	case EDistributionFunction::LOG_NORMAL:
		if (criticalMessage(m_dParam2, 1))
			return;
		break;
	case EDistributionFunction::RRSB:
	case EDistributionFunction::GGS:
		if (criticalMessage(m_dParam1, 0))
			return;
		break;
	case EDistributionFunction::MANUAL:
		break;
	case EDistributionFunction::COUNT_:
		assert(false);
	}

	m_vDistr = CreateDistribution(m_distrFun, m_gridMeans, m_dParam1, m_dParam2);
	Normalize(m_vDistr);

	QDialog::accept();
}

void CDistrFunctionDialog::CancelClicked()
{
	QDialog::reject();
}
