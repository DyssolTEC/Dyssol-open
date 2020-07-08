/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_CorrelationEditor.h"
#include "Correlation.h"

class CCorrelationEditor : public QWidget
{
	Q_OBJECT
private:
	Ui::CCorrelationEditor ui;
	CCorrelation* m_pCorrelation;

public:
	CCorrelationEditor(CCorrelation* _pCorrelation, QWidget *parent = Q_NULLPTR);

	QSize sizeHint() const override;

public slots:
	bool eventFilter(QObject *_obj, QEvent *_event) override;

private:
	void InitializeConnections();
	void UpdateWholeView() const;

	void UpdateTemperature() const;
	void UpdatePressure() const;
	void UpdateType() const;
	void UpdateTypeEquation() const;
	void UpdateParameters() const;
	void UpdateInfoButton() const;

	void TemperatureChanged();
	void PressureChanged();
	void TypeChanged();
	void ParameterChanged(int _iRow, int _iCol);
	void InfoButtonClicked();

	void SetupComboType() const;

signals:
	void CorrelationChanged();
};
