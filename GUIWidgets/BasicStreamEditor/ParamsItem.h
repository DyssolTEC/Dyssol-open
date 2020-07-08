/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DistributionsGrid.h"
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>

class CParamsItem : public QWidget
{
	Q_OBJECT
private:
	QSlider* m_pSlider;
	QComboBox* m_pCombo;
	QLabel* m_pLabel;
	CDistributionsGrid* m_pGrid;
	QHBoxLayout *m_pLayout;

public:
	CParamsItem(CDistributionsGrid *_pGrid, QComboBox *_pCombo, QSlider *_pSlider, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);

	int GetDistrIndex();
	void SetupSlider();
	void UpdateLabel();

public slots:
	void UpdateWholeView();

private slots:
	void DistrTypeChanged();
	void SliderValueChanged();
};
