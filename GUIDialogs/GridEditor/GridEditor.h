/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_GridEditor.h"
#include "DimensionParameters.h"
#include "Flowsheet.h"

class CGridEditor : public QDialog
{
	Q_OBJECT
private:
	Ui::CGridEditorClass ui;

	CFlowsheet* m_pFlowsheet; // pointer to the flowsheet
	CDistributionsGrid* m_pGrid;
	bool m_bAvoidSignal;

public:
	CGridEditor(CFlowsheet* _pFlowsheet, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~CGridEditor();

	void InitializeConnections();
	void UpdateWholeView();

private:
	QComboBox* AddComboBoxDimensions(unsigned _nRow);
	CDimensionParameters* AddDimensionParam();

	bool Validate();

public slots:
	void setVisible(bool _bVisible);
	void accept();

private slots:
	void AddDimensionClicked();
	void RemoveDimensionClicked();
	void DimensionTypeChanged(int _nRow, int _nCol, QComboBox* _pComboBox) const;
	bool ApplyChanges();

signals:
	void DataChanged();
};
