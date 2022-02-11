/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_GridLimitsDialog.h"

class CGridLimitsDialog : public QDialog
{
	Q_OBJECT

private:
	double* m_dXMin;
	double* m_dXMax;
	double* m_dYMin;
	double* m_dYMax;
	bool* m_bLogX;
	bool* m_bLogY;

public:
	CGridLimitsDialog(QWidget *parent);
	~CGridLimitsDialog();

	void SetLabels( QString _sXLabel, QString _sYLabel );
	void SetLimits( double& _dXMin, double& _dXMax, double& _dYMin, double& _dYmax );

	void SetLogScaling(bool &_bLogX, bool &_bLogY);

	void UpdateLogScaling(void);

private:
	Ui::CGridLimitsDialog ui;
	void UpdateWholeView();
	void ApplyChanges();

public slots:
	void ApplySlot();
	void accept();

signals:
	void ApplyPressed();
	void OkPressed();
};
