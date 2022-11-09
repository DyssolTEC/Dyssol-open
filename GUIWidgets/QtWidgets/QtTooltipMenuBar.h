/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <QMenuBar>

/*
* Class to enable tooltips for top-level menu entries
*/

class CQtTooltipMenuBar :
	public QMenuBar
{
	Q_OBJECT

public:
	CQtTooltipMenuBar(QWidget* _parent);

private:
	bool event(QEvent* _event) override;
};

