/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "OptionsEditor.h"
#include "Flowsheet.h"
#include "ParametersHolder.h"
#include "DyssolUtilities.h"
#include "SignalBlocker.h"

COptionsEditor::COptionsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _pMaterialsDB, QWidget* _parent, Qt::WindowFlags _flags)
	: CQtDialog{ _parent, _flags }
	, m_pFlowsheet{ _pFlowsheet }
	, m_pParams{ m_pFlowsheet->GetParameters() }
	, m_materialsDB{ _pMaterialsDB }
{
	ui.setupUi(this);
	ui.labelWarningStreams->setVisible(false);
	ui.labelWarningHoldups->setVisible(false);
	ui.labelWarningInternal->setVisible(false);
	ui.labelWarningWindow->setVisible(false);

	SetHelpLink("001_ui/gui.html#sec-gui-menu-setup-options");
}

void COptionsEditor::InitializeConnections() const
{
	connect(ui.pushButtonOk,			  &QPushButton::clicked,	   this, &COptionsEditor::ApplyChangesAndClose);
	connect(ui.pushButtonApply,			  &QPushButton::clicked,	   this, &COptionsEditor::ApplyChanges);
	connect(ui.pushButtonCancel,		  &QPushButton::clicked,	   this, &COptionsEditor::close);
	connect(ui.checkBoxCacheStreamsFlag,  &QCheckBox::toggled,         this, &COptionsEditor::CacheStreamsFlagChanged);
	connect(ui.checkBoxCacheHoldupsFlag,  &QCheckBox::toggled,	       this, &COptionsEditor::CacheHoldupsFlagChanged);
	connect(ui.checkBoxCacheInternalFlag, &QCheckBox::toggled,	       this, &COptionsEditor::CacheInternalFlagChanged);
	connect(ui.lineEditCacheWindow,	      &QLineEdit::editingFinished, this, &COptionsEditor::CacheWindowChanged);
}

void COptionsEditor::NewFlowsheetDataSet()
{
	if (isVisible())
		UpdateWholeView();
}

void COptionsEditor::UpdateWholeView()
{
	CSignalBlocker blocker{ ui.checkBoxCacheStreamsFlag, ui.checkBoxCacheHoldupsFlag, ui.checkBoxCacheInternalFlag, ui.lineEditCacheWindow };

	ShowValueAndLabel(ui.lineEditRTol        , ui.labelRTol        , m_pParams->relTol      );
	ShowValueAndLabel(ui.lineEditATol        , ui.labelATol        , m_pParams->absTol      );
	ShowValueAndLabel(ui.lineEditMinFraction , ui.labelMinFraction , m_pParams->minFraction );
	ShowValueAndLabel(ui.lineEditSaveTimeStep, ui.labelSaveTimeStep, m_pParams->saveTimeStep);
	ShowValueAndLabel(ui.lineEditTMin        , ui.labelTMin        , m_pParams->enthalpyMinT);
	ShowValueAndLabel(ui.lineEditTMax        , ui.labelTMax        , m_pParams->enthalpyMaxT);
	ShowValueAndLabel(ui.lineEditTIntervals  , ui.labelTIntervals  , m_pParams->enthalpyInt );
	ui.checkBoxSaveTimeStepHoldup->setChecked(m_pParams->saveTimeStepFlagHoldups);

	ShowValueAndLabel(ui.lineEditInitialWindow, ui.labelInitialWindow, m_pParams->initTimeWindow    );
	ShowValueAndLabel(ui.lineEditMinWindow    , ui.labelMinWindow    , m_pParams->minTimeWindow     );
	ShowValueAndLabel(ui.lineEditMaxWindow    , ui.labelMaxWindow    , m_pParams->maxTimeWindow     );
	ShowValueAndLabel(ui.lineEditMaxIterNum   , ui.labelMaxIterNum   , m_pParams->maxItersNumber    );
	ShowValueAndLabel(ui.lineEditRatio        , ui.labelRatio        , m_pParams->magnificationRatio);
	ShowValueAndLabel(ui.lineEditUpperLimit   , ui.labelUpperLimit   , m_pParams->itersUpperLimit   );
	ShowValueAndLabel(ui.lineEditLowerLimit   , ui.labelLowerLimit   , m_pParams->itersLowerLimit   );
	ShowValueAndLabel(ui.lineEdit1stUpperLimit, ui.label1stUpperLimit, m_pParams->iters1stUpperLimit);
	ShowValueAndLabel(ui.lineEditAccelParam   , ui.labelAccelParam   , m_pParams->wegsteinAccelParam);
	ShowValueAndLabel(ui.lineEditRelaxParam   , ui.labelRelaxParam   , m_pParams->relaxationParam   );
	ui.comboBoxConvMethod->setCurrentIndex(static_cast<int>(static_cast<EConvergenceMethod>(m_pParams->convergenceMethod)));
	ui.comboBoxExtrapMethod->setCurrentIndex(static_cast<int>(static_cast<EExtrapolationMethod>(m_pParams->extrapolationMethod)));

	ShowValueAndLabel(ui.lineEditCacheWindow, ui.labelCacheWindow, m_pParams->cacheWindowAfterReload);
	ui.checkBoxCacheStreamsFlag->setChecked(m_pParams->cacheFlagStreamsAfterReload);
	ui.checkBoxCacheHoldupsFlag->setChecked(m_pParams->cacheFlagHoldupsAfterReload);
	ui.checkBoxCacheInternalFlag->setChecked(m_pParams->cacheFlagInternalAfterReload);
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
	CSignalBlocker blocker{ _checkBox };

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
	CSignalBlocker blocker{ ui.lineEditCacheWindow };
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
	m_pParams->RelTol(ReadValue(ui.lineEditRTol));
	m_pParams->AbsTol(ReadValue(ui.lineEditATol));
	m_pParams->MinFraction(ReadValue(ui.lineEditMinFraction));
	m_pParams->SaveTimeStep(ReadValue(ui.lineEditSaveTimeStep));
	m_pParams->EnthalpyMinT(ReadValue(ui.lineEditTMin));
	m_pParams->EnthalpyMaxT(ReadValue(ui.lineEditTMax));
	m_pParams->EnthalpyInt(static_cast<uint32_t>(ReadValue(ui.lineEditTIntervals)));
	m_pParams->SaveTimeStepFlagHoldups(ui.checkBoxSaveTimeStepHoldup->isChecked());

	m_pParams->InitTimeWindow(ReadValue(ui.lineEditInitialWindow));
	m_pParams->MinTimeWindow(ReadValue(ui.lineEditMinWindow));
	m_pParams->MaxTimeWindow(ReadValue(ui.lineEditMaxWindow));
	m_pParams->MaxItersNumber(static_cast<uint32_t>(ReadValue(ui.lineEditMaxIterNum)));
	m_pParams->MagnificationRatio(ReadValue(ui.lineEditRatio));
	m_pParams->ItersUpperLimit(static_cast<uint32_t>(ReadValue(ui.lineEditUpperLimit)));
	m_pParams->ItersLowerLimit(static_cast<uint32_t>(ReadValue(ui.lineEditLowerLimit)));
	m_pParams->Iters1stUpperLimit(static_cast<uint32_t>(ReadValue(ui.lineEdit1stUpperLimit)));
	m_pParams->WegsteinAccelParam(ReadValue(ui.lineEditAccelParam));
	m_pParams->RelaxationParam(ReadValue(ui.lineEditRelaxParam));
	m_pParams->ConvergenceMethod(static_cast<EConvergenceMethod>(ui.comboBoxConvMethod->currentIndex()));
	m_pParams->ExtrapolationMethod(static_cast<EExtrapolationMethod>(ui.comboBoxExtrapMethod->currentIndex()));

	m_pParams->CacheWindowAfterReload(static_cast<uint32_t>(ReadValue(ui.lineEditCacheWindow)));
	m_pParams->CacheFlagStreamsAfterReload(ui.checkBoxCacheStreamsFlag->isChecked());
	m_pParams->CacheFlagHoldupsAfterReload(ui.checkBoxCacheHoldupsFlag->isChecked());
	m_pParams->CacheFlagInternalAfterReload(ui.checkBoxCacheInternalFlag->isChecked());
	m_pParams->FileSingleFlag(!ui.checkBoxSplitFile->isChecked());

	m_pFlowsheet->UpdateToleranceSettings();
	m_pFlowsheet->UpdateThermodynamicsSettings();
	emit DataChanged();
	UpdateWholeView();
}

void COptionsEditor::ApplyChangesAndClose()
{
	ApplyChanges();
	QDialog::accept();
}

void COptionsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	CQtDialog::setVisible(_bVisible);
}
