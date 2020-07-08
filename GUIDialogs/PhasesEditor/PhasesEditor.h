/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_PhasesEditor.h"
#include "Flowsheet.h"
#include <QDialog>
#include <QComboBox>

class CPhasesEditor : public QDialog
{
	Q_OBJECT

private:
	CFlowsheet *m_pFlowsheet;
	std::vector<QComboBox*> m_vCombos;

public:
	CPhasesEditor( CFlowsheet* _pFlowsheet, QWidget *parent = 0, Qt::WindowFlags flags = 0 );
	~CPhasesEditor();

	void InitializeConnections();

private:
	Ui::CPhasesEditorClass ui;

	bool ValidateInput();

public slots:
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
