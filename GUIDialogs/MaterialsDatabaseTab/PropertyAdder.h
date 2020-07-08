/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_PropertyAdder.h"
#include "DefinesMDB.h"

class CPropertyAdder : public QDialog
{
	Q_OBJECT

	Ui::CPropertyAdder ui;

	const std::vector<unsigned> m_activeProperties;

	MDBDescriptors::SPropertyDescriptor* m_property;

public:
	CPropertyAdder(MDBDescriptors::SPropertyDescriptor* _property, std::vector<unsigned> _activeProperties, QWidget *parent = Q_NULLPTR);

public slots:
	void setVisible(bool _visible) override;

private slots:
	void TypeChanged();
	void KeyChanged() const;
	void DataChanged() const;

private:
	void InitializeConnections() const;

	void UpdateWholeView() const;
	void UpdateType() const;
	void UpdateKey() const;
	void UpdateData() const;

	void SetupTypeCombo() const;
	void SetupKeyCombo() const;
	void CheckLimit();

	unsigned FirstAvailableKey() const;
};
