/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_AboutWindow.h"

class CAboutWindow : public QDialog
{
	Q_OBJECT

	Ui::CAboutWindow ui;

public:
	CAboutWindow(QWidget* parent = nullptr);

private:
	void InitializeConnections() const;

	void SetTitle();
	void SetDescription() const;
	void SetLicense() const;
};
