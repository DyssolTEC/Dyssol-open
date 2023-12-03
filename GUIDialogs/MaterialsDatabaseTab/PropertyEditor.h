/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_PropertyEditor.h"
#include "TPDProperty.h"

class CPropertyEditor : public QWidget
{
	Q_OBJECT

private:
	Ui::CPropertyEditor ui;

	CTPDProperty* m_pProperty;
	bool m_bAvoidSignal;

public:
	CPropertyEditor(QWidget* parent = nullptr);

	void SetProperty(CTPDProperty* _pProperty);

public slots:
	void setVisible(bool _bVisible) override;

private:
	void InitializeConnections() const;
	void Clear();

	void UpdateWholeView();
	void UpdateCorrelations();
	void UpdatePropertyName() const;
	void UpdateCalculator() const;
	void UpdatePlot() const;
	void UpdatePlotType() const;

private slots:
	void AddCorrelation();
	void RemoveCorrelation();
	void UpCorrelation();     // move current Correlation upwards {get index of a row of selected Correlation. If shift are possible, than do it. If not - go back, and continue. Than update control panel}
	void DownCorrelation();   // move current Correlation downwards {get index of a row of selected Correlation. If shift are possible, than do it. If not - go back, and continue. Than update control panel}
	void CorrelationChanged();
	void PlotTypeChanged() const;

signals:
	void MDBPropertyChanged();
};
