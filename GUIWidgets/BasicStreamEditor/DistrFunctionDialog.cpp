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

	m_distrFun = EDistrFunction::Normal;
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
		m_vSizes = _pGrid->GetGridDimensionNumeric(_nType)->GetClassesMeans();
	else
		m_vSizes = _pGrid->GetPSDMeans(_nPSDGridType);
}

std::vector<double> CDistrFunctionDialog::GetDistribution() const
{
	return m_vDistr;
}

void CDistrFunctionDialog::CreateDistrFunCombo() const
{
	QSignalBlocker blocker(ui.comboBox);

	ui.comboBox->insertItem(static_cast<int>(EDistrFunction::Normal),		StrConst::FUN_NormalName);
	ui.comboBox->insertItem(static_cast<int>(EDistrFunction::RRSB),			StrConst::FUN_RRSBName);
	ui.comboBox->insertItem(static_cast<int>(EDistrFunction::GGS),			StrConst::FUN_GGSName);
	ui.comboBox->insertItem(static_cast<int>(EDistrFunction::LogNormal),	StrConst::FUN_LogNormalName);

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
	if (m_distrFun == EDistrFunction::Normal)
		ui.labelUnits2->setText(sUnits);
	else
		ui.labelUnits2->setText(QString(StrConst::FUN_EmptyUnits));
}

void CDistrFunctionDialog::UpdateParamLabels() const
{
	switch (m_distrFun)
	{
	case EDistrFunction::Normal:
		ui.labelParam1->setText(StrConst::FUN_NormalParam1);
		ui.labelParam2->setText(StrConst::FUN_NormalParam2);
		break;
	case EDistrFunction::RRSB:
		ui.labelParam1->setText(StrConst::FUN_RRSBParam1);
		ui.labelParam2->setText(StrConst::FUN_RRSBParam2);
		break;
	case EDistrFunction::GGS:
		ui.labelParam1->setText(StrConst::FUN_GGSParam1);
		ui.labelParam2->setText(StrConst::FUN_GGSParam2);
		break;
	case EDistrFunction::LogNormal:
		ui.labelParam1->setText(StrConst::FUN_LogNormalParam1);
		ui.labelParam2->setText(StrConst::FUN_LogNormalParam2);
		break;
	case EDistrFunction::Manual:
		ui.labelParam1->setText(StrConst::FUN_UndefinedParam);
		ui.labelParam2->setText(StrConst::FUN_UndefinedParam);
		break;
	}
}

void CDistrFunctionDialog::UpdateParams() const
{
	ui.lineEditParam1->setText(QString::number(m_dParam1));
	ui.lineEditParam2->setText(QString::number(m_dParam2));
}

void CDistrFunctionDialog::FunctionChanged(int _index)
{
	m_distrFun = static_cast<EDistrFunction>(_index + 1);
	UpdateParamLabels();
	UpdateUnits();
}

void CDistrFunctionDialog::OKClicked()
{
	m_dParam1 = ui.lineEditParam1->text().toDouble();
	m_dParam2 = ui.lineEditParam2->text().toDouble();
	m_distrFun = static_cast<EDistrFunction>(ui.comboBox->currentIndex() + 1);

	switch (m_distrFun)
	{
	case EDistrFunction::Normal:
		if (m_dParam2 == 0) {	QMessageBox::critical(this, StrConst::FUN_ErrorName, QString::fromStdString(StrConst::FUN_ErrorZeroParameter(StrConst::FUN_NormalParam2)));		return;	}
		break;
	case EDistrFunction::RRSB:
		if (m_dParam1 == 0) {	QMessageBox::critical(this, StrConst::FUN_ErrorName, QString::fromStdString(StrConst::FUN_ErrorZeroParameter(StrConst::FUN_RRSBParam1)));		return;	}
		break;
	case EDistrFunction::GGS:
		if (m_dParam1 == 0) {	QMessageBox::critical(this, StrConst::FUN_ErrorName, QString::fromStdString(StrConst::FUN_ErrorZeroParameter(StrConst::FUN_GGSParam1)));		return;	}
		break;
	case EDistrFunction::LogNormal:
		if (m_dParam2 == 0) {	QMessageBox::critical(this, StrConst::FUN_ErrorName, QString::fromStdString(StrConst::FUN_ErrorZeroParameter(StrConst::FUN_LogNormalParam2)));	return;	}
		break;
	case EDistrFunction::Manual:
		break;
	}

	m_vDistr = CreateDistribution(m_distrFun, m_vSizes, m_dParam1, m_dParam2);
	Normalize(m_vDistr);

	QDialog::accept();
}

void CDistrFunctionDialog::CancelClicked()
{
	QDialog::reject();
}
