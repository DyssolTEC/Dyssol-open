/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtTooltipMenuBar.h"
#include <QHelpEvent>
#include <QToolTip>

CQtTooltipMenuBar::CQtTooltipMenuBar(QWidget* _parent)
	: QMenuBar(_parent)
{
}

bool CQtTooltipMenuBar::event(QEvent* _event)
{
	if (_event->type() == QEvent::ToolTip)
	{
		const QHelpEvent* help = dynamic_cast<QHelpEvent*>(_event);
		const QMenu* menu = help ? actionAt(help->pos())->menu() : nullptr;
		if (menu)
		{
			QToolTip::showText(help->globalPos(), menu->toolTip(), this);
			return true;
		}
		if (_event->type() == QEvent::Paint && QToolTip::isVisible())
		{
			QToolTip::hideText();
		}
	}
	return QMenuBar::event(_event);
}
