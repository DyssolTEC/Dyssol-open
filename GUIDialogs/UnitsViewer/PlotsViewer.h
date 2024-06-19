/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_PlotsViewer.h"
#include "QtTable.h"
#include "QtPlot.h"
#include "QtDialog.h"

#define PLOT_LINE_WIDTH	3

class CUnitContainer;

class CPlotsViewer
	: public QWidget
	, public CDyssolBaseWidget
{
	Q_OBJECT

private:
	CUnitContainer* m_pModel;
	CQtTable *m_pTableWidget;	// table for distributions' visualization
	QtPlot::CQtPlot *m_pPlot;	// plot for distributions' visualization
	bool m_bAvoidSignal;

	double m_dSliderBeg;
	double m_dSliderIntervLen;
	int m_iPlot;
	std::vector<int> m_iCurves;

public:
	CPlotsViewer( QWidget *parent = 0 );
	~CPlotsViewer();

	void SetSelectedModel(CUnitContainer* _pModel);
	void SetSelectedPlot( int _nIndex );
	void SetSelectedCurve( int _nIndex );
	void SetSelectedCurve( const std::vector<int>& _vIndexes );

private:
	Ui::CPlotsViewer ui;

	void InitializeConnections();

	void UpdateSlider();
	void UpdateSliderLabel();
	void UpdateSliderLabelName();
	void UpdateSliderPosFromSelectedIndex();
	void UpdateTabWidget();
	void UpdateTable();
	void UpdatePlot();

public slots:
	void UpdateWholeView();
	void setVisible( bool _bVisible ) override;

private slots:
	void SliderPositionChanged( int _nIndex );
	void TabChanged( int _nIndex );
};
