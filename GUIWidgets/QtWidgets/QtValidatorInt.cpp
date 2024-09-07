/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtValidatorInt.h"

CQtValidatorInt::CQtValidatorInt(QObject* _parent)
	: QRegExpValidator{ QRegExp{ "^([+-]?([\\d]+))([Ee][+]?[0-9]+)?$" }, _parent }
{
}

void CQtValidatorInt::fixup(QString& _input) const
{
	_input = "0";
}
