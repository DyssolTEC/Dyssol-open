/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QRegExpValidator>

class CQtValidatorInt : public QRegExpValidator
{
public:
    CQtValidatorInt(QObject* _parent = nullptr);

private:
    void fixup(QString& _input) const override;
};

