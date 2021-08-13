/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SignalBlocker.h"

CSignalBlocker::CSignalBlocker(QObject* _object)
{
	m_objects.push_back(_object);
	m_flags.push_back(m_objects.front()->blockSignals(true));
}

CSignalBlocker::CSignalBlocker(std::initializer_list<QObject*> _objects)
{
	m_objects = _objects;
	for (auto& object : m_objects)
		m_flags.push_back(object->blockSignals(true));
}

CSignalBlocker::~CSignalBlocker()
{
	Unblock();
}

void CSignalBlocker::Unblock()
{
	for (size_t i = 0; i < m_objects.size(); ++i)
		m_objects[i]->blockSignals(m_flags[i]);
}
