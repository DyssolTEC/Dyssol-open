/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_Functional.h"
#include "DistributionsGrid.h"

class CDistrFunctionDialog : public QDialog
{
	Q_OBJECT
private:
	Ui::CFunctional ui;

	double m_dParam1;
	double m_dParam2;
	std::vector<double> m_vSizes;
	std::vector<double> m_vDistr;
	EDistrFunction m_distrFun;
	CDistributionsGrid* m_pGrid;
	EDistrTypes m_nDimType;
	EPSDGridType m_PSDGridType;

public:
	CDistrFunctionDialog(QWidget* parent = nullptr);

	void SetDistributionsGrid(CDistributionsGrid* _pGrid, EDistrTypes _nType, EPSDGridType _nPSDGridType);

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
