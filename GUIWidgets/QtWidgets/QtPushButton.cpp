/* Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtPushButton.h"
#include <QPainter>

CQtPushButton::CQtPushButton(QWidget* _parent)
	: QPushButton{ _parent }
{
}

void CQtPushButton::setIcon(const QIcon& _icon)
{
	m_icon = _icon;
	m_pixmap = _icon.pixmap(iconSize());
}

void CQtPushButton::setIconSize(const QSize& _size)
{
	QPushButton::setIconSize(_size);
	if (!m_icon.isNull())
		m_pixmap = m_icon.pixmap(iconSize());
}

QSize CQtPushButton::sizeHint() const
{
	const auto parentHint = QPushButton::sizeHint();
	const auto sz = iconSize();
	return { parentHint.width() + 2 * sz.width() + 2 * c_margin, std::max(parentHint.height(), sz.height() + 2 * c_margin) };
}

void CQtPushButton::paintEvent(QPaintEvent* _event)
{
	QPushButton::paintEvent(_event);
	if (!m_pixmap.isNull())
	{
		const int y = (height() - m_pixmap.height()) / 2;
		QPainter painter{ this };
		painter.drawPixmap(c_margin, y, m_pixmap);
	}
}