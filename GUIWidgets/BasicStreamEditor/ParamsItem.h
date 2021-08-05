/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>

class CMultidimensionalGrid;

class CParamsItem : public QWidget
{
	Q_OBJECT
private:
	QSlider* m_pSlider;
	QComboBox* m_pCombo;
	QLabel* m_pLabel;
	const CMultidimensionalGrid* m_pGrid;
	QHBoxLayout *m_pLayout;

public:
	CParamsItem(const CMultidimensionalGrid* _pGrid, QComboBox *_pCombo, QSlider *_pSlider, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	int GetDistrIndex();
	void SetupSlider();
	void UpdateLabel();

public slots:
	void UpdateWholeView();

private slots:
	void DistrTypeChanged();
	void SliderValueChanged();
};
