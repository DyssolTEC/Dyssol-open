/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_UnitsViewer.h"
#include "QtPlot.h"
#include "QtDialog.h"
#include <QStackedWidget>

class CPlotsViewer;
class CBasicStreamsViewer;
class CQtTable;
class CMaterialsDatabase;
class CFlowsheet;
class CUnitContainer;

class CUnitsViewer : public CQtDialog
{
	Q_OBJECT

private:
	CFlowsheet* m_pFlowsheet; // pointer to the flowsheet

	QStackedWidget *m_pStackedWidget;
	QTabWidget *m_pTabWidget;
	CBasicStreamsViewer *m_pStreamsViewer;	// streams viewer
	bool m_bAvoidSignal;
	CUnitContainer* m_pSelectedModel;
	int m_nSelectedVariable;
	QtPlot::CQtPlot *m_pPlot;
	CQtTable *m_pTableWidget;
	CPlotsViewer *m_pPlotsViewer;

	// for save/restore view
	int m_nLastTab;
	int m_nLastVariable;
	int m_nLastUnit;
	int m_nLastPlot;
	int m_nLastCurve;

public:
	CUnitsViewer(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDatabase, CModelsManager* _modelsManager, QWidget* _parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	~CUnitsViewer();

	void InitializeConnections();

private:
	Ui::CUnitsViewerClass ui;

	void SaveViewState();
	void LoadViewState();

private slots:
	void TabChanged();
	void UpdateUnitsView();
	void UpdateVariablesView();
	void UpdateValuesView();
	void UpdateHoldupsView();
	void UpdatePlotsView();
	void UpdateCurvesView();
	void UnitChanged();
	void VariableChanged();
	void HoldupChanged();
	void PlotChanged();
	void CurveChanged();
	void FocusChanged(int index);

public slots:
	void UpdateWholeView();
	void setVisible( bool _bVisible );
};
