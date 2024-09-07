/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtValidatorDouble.h"

CQtValidatorDouble::CQtValidatorDouble(QObject* _parent)
	: QRegExpValidator{ QRegExp{ "^([+-]?(?:[0-9]+\\.?|[0-9]*\\.[0-9]+))(?:[Ee][+-]?[0-9]+)?$" }, _parent}
{
}

void CQtValidatorDouble::fixup(QString& _input) const
{
	_input = "0";
}
