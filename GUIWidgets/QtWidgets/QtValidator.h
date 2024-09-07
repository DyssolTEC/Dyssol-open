/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QRegExpValidator>

template<typename T>
class CQtValidator : public QRegExpValidator
{
	void fixup(QString& _input) const override
	{
		_input = "0";
	}
};

template<>
class CQtValidator<double> : public QRegExpValidator
{
public:
	explicit CQtValidator(QObject* _parent = nullptr)
		: QRegExpValidator(QRegExp{ "^([+-]?(?:[0-9]+\\.?|[0-9]*\\.[0-9]+))(?:[Ee][+-]?[0-9]+)?$" }, _parent)
	{
	}
};

template<>
class CQtValidator<int64_t> : public QRegExpValidator
{
public:
	explicit CQtValidator(QObject* _parent = nullptr)
		: QRegExpValidator(QRegExp{ "^([+-]?([\\d]+))([Ee][+]?[0-9]+)?$" }, _parent)
	{
	}
};

template<>
class CQtValidator<uint64_t> : public QRegExpValidator
{
public:
	explicit CQtValidator(QObject* _parent = nullptr)
		: QRegExpValidator(QRegExp{ "^([+]?([\\d]+))([Ee][+]?[0-9]+)?$" }, _parent)
	{
	}
};
