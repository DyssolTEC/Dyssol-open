/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_OptionsEditor.h"
#include "QtDialog.h"
#include <QMessageBox>

class CFlowsheet;
class CMaterialsDatabase;
class CParametersHolder;

class COptionsEditor
	: public CQtDialog
{
	Q_OBJECT

	Ui::COptionsEditorClass ui;

	CFlowsheet* m_pFlowsheet;			// pointer to the flowsheet
	CParametersHolder* m_pParams;	// pointer to the flowsheet parameters
	CMaterialsDatabase* m_materialsDB; // pointer to materials database

	unsigned m_cacheWindowSizeBeforeEdit; // stores old the value of cacheWindowSize for undo

public:
	COptionsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _pMaterialsDB, QWidget* _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags());

	void InitializeConnections() const;

public slots:
	void UpdateWholeView();

	void setVisible(bool _bVisible) override;

private:
	void UpdateCacheWindowVisible() const;
	void UpdateWarningsVisible() const;
	void CacheFlagChanged(QCheckBox* _checkBox, bool _currFlag);
	void CacheWindowChanged();
	QMessageBox::StandardButton AskReopen();

private slots:
	void CacheStreamsFlagChanged();
	void CacheHoldupsFlagChanged();
	void CacheInternalFlagChanged();
	void ApplyChanges();

signals:
	void NeedSaveAndReopen();
	void DataChanged();
};
