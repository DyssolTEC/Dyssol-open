/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QRegExpValidator>

class CQtValidatorUInt : public QRegExpValidator
{
public:
    CQtValidatorUInt(QObject* _parent = nullptr);

private:
    void fixup(QString& _input) const override;
};

