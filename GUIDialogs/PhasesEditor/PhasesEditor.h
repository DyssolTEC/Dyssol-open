/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_PhasesEditor.h"
#include "QtDialog.h"
#include <QComboBox>

class CFlowsheet;

class CPhasesEditor
	: public CQtDialog
{
	Q_OBJECT

private:
	CFlowsheet *m_pFlowsheet;
	std::vector<QComboBox*> m_vCombos;

public:
	CPhasesEditor(CFlowsheet* _pFlowsheet, QWidget* _parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	~CPhasesEditor();

	void InitializeConnections();

private:
	Ui::CPhasesEditorClass ui;

	bool ValidateInput();

public slots:
	void NewFlowsheetDataSet();
	void UpdateWholeView();
	void setVisible( bool _bVisible );
	void accept();

private slots:
	void AddPhase();
	void RemovePhase();
	bool ApplyChanges();

signals:
	void DataChanged();
};
