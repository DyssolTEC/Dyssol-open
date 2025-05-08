/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DistributionFunctions.h"
#include "DyssolDefines.h"
#include "ui_Functional.h"

class CMultidimensionalGrid;

class CDistrFunctionDialog : public QDialog
{
	Q_OBJECT
private:
	Ui::CFunctional ui;

	double m_dParam1;
	double m_dParam2;
	std::vector<double> m_gridMeans; // mean values of the grid of distributed value
	std::vector<double> m_vDistr;
	EDistributionFunction m_distrFun;
	const CMultidimensionalGrid* m_pGrid;
	EDistrTypes m_nDimType;
	EPSDGridType m_PSDGridType;

public:
	CDistrFunctionDialog(QWidget* parent = nullptr);

	void SetDistributionsGrid(const CMultidimensionalGrid* _pGrid, EDistrTypes _nType, EPSDGridType _nPSDGridType);

	std::vector<double> GetDistribution() const;

private slots:
	void setVisible(bool _bVisible) override;

private:
	void CreateDistrFunCombo() const;

	void UpdateWholeView() const;
	void UpdateDistrFunction() const;
	void UpdateUnits() const;
	void UpdateParamLabels() const;
	void UpdateParams() const;

	void FunctionChanged(int _index);
	void OKClicked();
	void CancelClicked();
};
