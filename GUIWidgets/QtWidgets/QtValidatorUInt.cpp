/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtValidatorUInt.h"

CQtValidatorUInt::CQtValidatorUInt(QObject* _parent)
	: QRegExpValidator{ QRegExp{ "^([+]?([\\d]+))([Ee][+]?[0-9]+)?$" }, _parent }
{
}

void CQtValidatorUInt::fixup(QString& _input) const
{
	_input = "0";
}
