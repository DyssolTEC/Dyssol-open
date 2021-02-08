/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "OptionsEditor.h"
#include "Flowsheet.h"
#include "ParametersHolder.h"
#include "DyssolUtilities.h"

COptionsEditor::COptionsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _pMaterialsDB, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= {}*/)
	: QDialog(parent, flags),
	m_pFlowsheet(_pFlowsheet),
	m_pParams(m_pFlowsheet->GetParameters()),
	m_pMaterialsDB(_pMaterialsDB)
{
	ui.setupUi(this);
	ui.labelWarningStreams->setVisible(false);
	ui.labelWarningHoldups->setVisible(false);
	ui.labelWarningInternal->setVisible(false);
	ui.labelWarningWindow->setVisible(false);
}

void COptionsEditor::InitializeConnections() const
{
	connect(ui.pushButtonOk,			  &QPushButton::clicked,	   this, &COptionsEditor::ApplyChanges);
	connect(ui.pushButtonCancel,		  &QPushButton::clicked,	   this, &COptionsEditor::close);
	connect(ui.checkBoxCacheStreamsFlag,  &QCheckBox::toggled,         this, &COptionsEditor::CacheStreamsFlagChanged);
	connect(ui.checkBoxCacheHoldupsFlag,  &QCheckBox::toggled,	       this, &COptionsEditor::CacheHoldupsFlagChanged);
	connect(ui.checkBoxCacheInternalFlag, &QCheckBox::toggled,	       this, &COptionsEditor::CacheInternalFlagChanged);
	connect(ui.lineEditCacheWindow,	      &QLineEdit::editingFinished, this, &COptionsEditor::CacheWindowChanged);
}

void COptionsEditor::UpdateWholeView()
{
	QSignalBlocker blocker1(ui.checkBoxCacheStreamsFlag);
	QSignalBlocker blocker2(ui.checkBoxCacheHoldupsFlag);
	QSignalBlocker blocker3(ui.checkBoxCacheInternalFlag);
	QSignalBlocker blocker4(ui.lineEditCacheWindow);

	ui.lineEditATol->setText(QString::number(m_pParams->absTol));
	ui.lineEditRTol->setText(QString::number(m_pParams->relTol));
	ui.lineEditMinFraction->setText(QString::number(m_pParams->minFraction));
	ui.lineEditSaveTimeStep->setText(QString::number(m_pParams->saveTimeStep));
	ui.checkBoxSaveTimeStepHoldup->setChecked(m_pParams->saveTimeStepFlagHoldups);
	ui.lineEditTMin->setText(QString::number(m_pParams->enthalpyMinT));
	ui.lineEditTMax->setText(QString::number(m_pParams->enthalpyMaxT));
	ui.lineEditTIntervals->setText(QString::number(m_pParams->enthalpyInt));
	ui.lineEditInitialWindow->setText(QString::number(m_pParams->initTimeWindow));
	ui.lineEditMinWindow->setText(QString::number(m_pParams->minTimeWindow));
	ui.lineEditMaxWindow->setText(QString::number(m_pParams->maxTimeWindow));
	ui.lineEditMaxIterNum->setText(QString::number(m_pParams->maxItersNumber));
	ui.lineEditRatio->setText(QString::number(m_pParams->magnificationRatio));
	ui.lineEditUpperLimit->setText(QString::number(m_pParams->itersUpperLimit));
	ui.lineEditLowerLimit->setText(QString::number(m_pParams->itersLowerLimit));
	ui.lineEdit1stUpperLimit->setText(QString::number(m_pParams->iters1stUpperLimit));
	ui.comboBoxConvMethod->setCurrentIndex(m_pParams->convergenceMethod);
	ui.lineEditAccelParam->setText(QString::number(m_pParams->wegsteinAccelParam));
	ui.lineEditRelaxParam->setText(QString::number(m_pParams->relaxationParam));
	ui.comboBoxExtrapMethod->setCurrentIndex(E2I(static_cast<EExtrapMethod>(m_pParams->extrapolationMethod)));
	ui.checkBoxCacheStreamsFlag->setChecked(m_pParams->cacheFlagStreamsAfterReload);
	ui.checkBoxCacheHoldupsFlag->setChecked(m_pParams->cacheFlagHoldupsAfterReload);
	ui.checkBoxCacheInternalFlag->setChecked(m_pParams->cacheFlagInternalAfterReload);
	ui.lineEditCacheWindow->setText(QString::number(m_pParams->cacheWindowAfterReload));
	ui.checkBoxSplitFile->setChecked(!m_pParams->fileSingleFlag);

	UpdateCacheWindowVisible();
	UpdateWarningsVisible();

	m_cacheWindowSizeBeforeEdit = m_pParams->cacheWindowAfterReload;
}

void COptionsEditor::UpdateCacheWindowVisible() const
{
	ui.lineEditCacheWindow->setEnabled(ui.checkBoxCacheStreamsFlag->isChecked() || ui.checkBoxCacheHoldupsFlag->isChecked() || ui.checkBoxCacheInternalFlag->isChecked());
}

void COptionsEditor::UpdateWarningsVisible() const
{
	const bool bStreams = m_pParams->cacheFlagStreams != ui.checkBoxCacheStreamsFlag->isChecked();
	const bool bHoldups = m_pParams->cacheFlagHoldups != ui.checkBoxCacheHoldupsFlag->isChecked();
	const bool bInternal = m_pParams->cacheFlagInternal != ui.checkBoxCacheInternalFlag->isChecked();
	const bool bWindow = m_pParams->cacheWindow != ui.lineEditCacheWindow->text().toUInt();
	ui.labelWarningStreams->setVisible(bStreams);
	ui.labelWarningHoldups->setVisible(bHoldups);
	ui.labelWarningInternal->setVisible(bInternal);
	ui.labelWarningWindow->setVisible(bWindow);
	ui.labelWarning->setVisible(bStreams || bHoldups || bInternal || bWindow);
}

void COptionsEditor::CacheFlagChanged(QCheckBox* _checkBox, bool _currFlag)
{
	QSignalBlocker blocker(_checkBox);

	if (_checkBox->isChecked() != _currFlag)
	{
		switch (AskReopen())
		{
		case QMessageBox::Yes:
			ApplyChanges();
			emit NeedSaveAndReopen();
			break;
		case QMessageBox::Cancel:
			_checkBox->toggle();
			break;
		default: ;
		}
	}

	UpdateCacheWindowVisible();
	UpdateWarningsVisible();
}

void COptionsEditor::CacheWindowChanged()
{
	QSignalBlocker blocker(ui.lineEditCacheWindow);
	ui.lineEditCacheWindow->clearFocus(); // to prevent re-firing of signals

	if (ui.lineEditCacheWindow->text().toUInt() != m_pParams->cacheWindow)
	{
		switch (AskReopen())
		{
		case QMessageBox::Yes:
			ApplyChanges();
			emit NeedSaveAndReopen();
			break;
		case QMessageBox::No:
			m_cacheWindowSizeBeforeEdit = ui.lineEditCacheWindow->text().toUInt();
			break;
		case QMessageBox::Cancel:
			ui.lineEditCacheWindow->setText(QString::number(m_cacheWindowSizeBeforeEdit));
			break;
		default:;
		}
	}

	UpdateWarningsVisible();
}

QMessageBox::StandardButton COptionsEditor::AskReopen()
{
	return QMessageBox::question(this, "Confirm reopen",
		"To apply this setting, the current flowsheet must be saved and reopened. Save and reopen now?",
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}

void COptionsEditor::CacheStreamsFlagChanged()
{
	CacheFlagChanged(ui.checkBoxCacheStreamsFlag, m_pParams->cacheFlagStreams);
}

void COptionsEditor::CacheHoldupsFlagChanged()
{
	CacheFlagChanged(ui.checkBoxCacheHoldupsFlag, m_pParams->cacheFlagHoldups);
}

void COptionsEditor::CacheInternalFlagChanged()
{
	CacheFlagChanged(ui.checkBoxCacheInternalFlag, m_pParams->cacheFlagInternal);
}

void COptionsEditor::ApplyChanges()
{
	m_pParams->AbsTol(ui.lineEditATol->text().toDouble());
	m_pParams->RelTol(ui.lineEditRTol->text().toDouble());
	m_pParams->MinFraction(ui.lineEditMinFraction->text().toDouble());
	m_pParams->SaveTimeStep(ui.lineEditSaveTimeStep->text().toDouble());
	m_pParams->SaveTimeStepFlagHoldups(ui.checkBoxSaveTimeStepHoldup->isChecked());
	m_pParams->EnthalpyMinT(ui.lineEditTMin->text().toDouble());
	m_pParams->EnthalpyMaxT(ui.lineEditTMax->text().toDouble());
	m_pParams->EnthalpyInt(ui.lineEditTIntervals->text().toDouble());
	m_pParams->InitTimeWindow(ui.lineEditInitialWindow->text().toDouble());
	m_pParams->MinTimeWindow(ui.lineEditMinWindow->text().toDouble());
	m_pParams->MaxTimeWindow(ui.lineEditMaxWindow->text().toDouble());
	m_pParams->MaxItersNumber(ui.lineEditMaxIterNum->text().toDouble());
	m_pParams->MagnificationRatio(ui.lineEditRatio->text().toDouble());
	m_pParams->ItersUpperLimit(ui.lineEditUpperLimit->text().toInt());
	m_pParams->ItersLowerLimit(ui.lineEditLowerLimit->text().toInt());
	m_pParams->Iters1stUpperLimit(ui.lineEdit1stUpperLimit->text().toInt());
	m_pParams->ConvergenceMethod(static_cast<EConvMethod>(ui.comboBoxConvMethod->currentIndex()));
	m_pParams->WegsteinAccelParam(ui.lineEditAccelParam->text().toDouble());
	m_pParams->RelaxationParam(ui.lineEditRelaxParam->text().toDouble());
	m_pParams->ExtrapolationMethod(static_cast<EExtrapMethod>(ui.comboBoxExtrapMethod->currentIndex()));
	m_pParams->CacheWindowAfterReload(ui.lineEditCacheWindow->text().toInt());
	m_pParams->CacheFlagStreamsAfterReload(ui.checkBoxCacheStreamsFlag->isChecked());
	m_pParams->CacheFlagHoldupsAfterReload(ui.checkBoxCacheHoldupsFlag->isChecked());
	m_pParams->CacheFlagInternalAfterReload(ui.checkBoxCacheInternalFlag->isChecked());
	m_pParams->FileSingleFlag(!ui.checkBoxSplitFile->isChecked());

	m_pFlowsheet->UpdateToleranceSettings();
	m_pFlowsheet->UpdateThermodynamicsSettings();
	emit DataChanged();
	QDialog::accept();
}

void COptionsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	QDialog::setVisible(_bVisible);
}
