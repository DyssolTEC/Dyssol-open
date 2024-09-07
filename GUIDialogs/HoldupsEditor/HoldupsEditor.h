/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_HoldupsEditor.h"
#include "QtDialog.h"

class CFlowsheet;
class CUnitContainer;

class CHoldupsEditor
	: public CQtDialog
{
	Q_OBJECT
private:
	CFlowsheet* m_pFlowsheet; // pointer to the flowsheet

	CUnitContainer* m_pSelectedModel; // pointer to the selected unit

	// for save/restore view
	int m_nLastModel;
	int m_nLastHoldup;

public:
	Ui::CHoldupsEditorClass ui;

public slots:
	void setVisible(bool _bVisible) override;
	void NewFlowsheetDataSet();
	void UpdateWholeView();
	void UpdateUnitsList() const;
	void UpdateHoldupsList(); // update the list of holdups of specified unit

public:
	CHoldupsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* _parent = nullptr);

	void InitializeConnections() const;

private:
	void SaveViewState();
	void LoadViewState() const;

private slots:
	void NewHoldupSelected() const;
	void ChangeData();

signals:
	void DataChanged();	// some information in holdup have been changed
};
