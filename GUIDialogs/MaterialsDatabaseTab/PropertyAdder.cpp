/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PropertyAdder.h"
#include "DyssolUtilities.h"
#include "SignalBlocker.h"
#include <QStandardItem>
#include <QMessageBox>

CPropertyAdder::CPropertyAdder(MDBDescriptors::SPropertyDescriptor* _property, std::vector<unsigned> _activeProperties, QWidget *parent)
	: QDialog(parent),
	m_activeProperties{std::move(_activeProperties)},
	m_property{ _property }
{
	ui.setupUi(this);

	// set regular expression for limitation of input to floating point numbers
	ui.lineEditValue->setValidator(new QRegExpValidator(QRegExp{ "^[0-9]*[.]?[0-9]+(?:[eE][-+]?[0-9]+)?$" }, this));

	SetupTypeCombo();
	SetupKeyCombo();
	InitializeConnections();
}

void CPropertyAdder::InitializeConnections() const
{
	connect(ui.comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CPropertyAdder::TypeChanged);
	connect(ui.comboBoxKey,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CPropertyAdder::KeyChanged);
	connect(ui.lineEditName,        &QLineEdit::textChanged, this, &CPropertyAdder::DataChanged);
	connect(ui.lineEditValue,       &QLineEdit::textChanged, this, &CPropertyAdder::DataChanged);
	connect(ui.lineEditUnits,       &QLineEdit::textChanged, this, &CPropertyAdder::DataChanged);
	connect(ui.textEditDescription, &QTextEdit::textChanged, this, &CPropertyAdder::DataChanged);

}

void CPropertyAdder::setVisible(bool _visible)
{
	QDialog::setVisible(_visible);
	if (_visible)
		UpdateWholeView();
}

void CPropertyAdder::UpdateWholeView() const
{
	UpdateType();
	UpdateKey();
	UpdateData();
}

void CPropertyAdder::UpdateType() const
{
	QSignalBlocker blocker(ui.comboBoxType);

	const int index = ui.comboBoxType->findData(E2I(m_property->type));
	ui.comboBoxType->setCurrentIndex(index);
}

void CPropertyAdder::UpdateKey() const
{
	QSignalBlocker blocker(ui.comboBoxKey);

	const int index = ui.comboBoxKey->findData(m_property->key);
	ui.comboBoxKey->setCurrentIndex(index);
	ui.labelKeyValue->setText(QString::number(m_property->key));
}

void CPropertyAdder::UpdateData() const
{
	CSignalBlocker blocker{ ui.lineEditValue, ui.lineEditName, ui.lineEditUnits, ui.textEditDescription };
	ui.lineEditValue->setText(QString::number(m_property->value));
	ui.lineEditName->setText(QString::fromStdString(m_property->name));
	ui.lineEditUnits->setText(QString::fromStdWString(m_property->units));
	ui.textEditDescription->setText(QString::fromStdString(m_property->description));
}

void CPropertyAdder::TypeChanged()
{
	m_property->type = static_cast<MDBDescriptors::EPropertyType>(ui.comboBoxType->currentData().toUInt());
	m_property->key = FirstAvailableKey();
	CheckLimit();
	SetupKeyCombo();
	UpdateKey();
}

void CPropertyAdder::KeyChanged() const
{
	m_property->key = ui.comboBoxKey->currentData().toUInt();
	UpdateKey();
}

void CPropertyAdder::DataChanged() const
{
	m_property->value       = ui.lineEditValue->text().toDouble();
	m_property->name        = ui.lineEditName->text().toStdString();
	m_property->units       = ui.lineEditUnits->text().toStdWString();
	m_property->description = ui.textEditDescription->toPlainText().toStdString();
}

void CPropertyAdder::SetupTypeCombo() const
{
	QSignalBlocker blocker(ui.comboBoxType);

	ui.comboBoxType->clear();
	ui.comboBoxType->addItem("Constant",     E2I(MDBDescriptors::EPropertyType::CONSTANT));
	ui.comboBoxType->addItem("TP-dependent", E2I(MDBDescriptors::EPropertyType::TP_DEPENDENT));
	ui.comboBoxType->addItem("Interaction",  E2I(MDBDescriptors::EPropertyType::INTERACTION));

	const bool isInteraction = m_property->type == MDBDescriptors::EPropertyType::INTERACTION;
	const int index1 = ui.comboBoxType->findData(E2I(MDBDescriptors::EPropertyType::CONSTANT));
	const int index2 = ui.comboBoxType->findData(E2I(MDBDescriptors::EPropertyType::TP_DEPENDENT));
	const int index3 = ui.comboBoxType->findData(E2I(MDBDescriptors::EPropertyType::INTERACTION));
	QStandardItem* item1 = qobject_cast<QStandardItemModel*>(ui.comboBoxType->model())->item(index1);
	QStandardItem* item2 = qobject_cast<QStandardItemModel*>(ui.comboBoxType->model())->item(index2);
	QStandardItem* item3 = qobject_cast<QStandardItemModel*>(ui.comboBoxType->model())->item(index3);
	item1->setEnabled(!isInteraction);
	item2->setEnabled(!isInteraction);
	item3->setEnabled(isInteraction);
}

void CPropertyAdder::SetupKeyCombo() const
{
	QSignalBlocker blocker(ui.comboBoxKey);
	ui.comboBoxKey->clear();

	unsigned startVal{ MDBDescriptors::FIRST_CONST_USER_PROP };
	QString text;
	switch (m_property->type)
	{
	case MDBDescriptors::EPropertyType::CONSTANT:	  startVal = MDBDescriptors::FIRST_CONST_USER_PROP;	text = "CONST_PROP_USER_DEFINED_"; break;
	case MDBDescriptors::EPropertyType::TP_DEPENDENT: startVal = MDBDescriptors::FIRST_TPDEP_USER_PROP;	text = "TP_PROP_USER_DEFINED_";	   break;
	case MDBDescriptors::EPropertyType::INTERACTION:  startVal = MDBDescriptors::FIRST_INTER_USER_PROP;	text = "INT_PROP_USER_DEFINED_";   break;
	}

	for (int i = 0; i < static_cast<int>(MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER); ++i)
	{
		const unsigned key = startVal + i;
		ui.comboBoxKey->addItem(text + tr("%1").arg(i + 1, 2, 10, QChar{ '0' }), key);
		const bool enabled = !VectorContains(m_activeProperties, key);
		QStandardItem* item = qobject_cast<QStandardItemModel*>(ui.comboBoxKey->model())->item(i);
		item->setEnabled(enabled);
	}
}

void CPropertyAdder::CheckLimit()
{
	const bool bFull = m_property->key == CONST_PROP_NO_PROERTY || m_property->key == TP_PROP_NO_PROERTY || m_property->key == INT_PROP_NO_PROERTY;
	ui.comboBoxKey->setEnabled(!bFull);
	ui.labelKeyValue->setEnabled(!bFull);
	ui.lineEditName->setEnabled(!bFull);
	ui.lineEditUnits->setEnabled(!bFull);
	ui.lineEditValue->setEnabled(!bFull);
	ui.pushButtonOK->setEnabled(!bFull);
	if(bFull)
		QMessageBox::warning(this, "Custom properties", "Can not add more properties of this type");
}

unsigned CPropertyAdder::FirstAvailableKey() const
{
	unsigned startVal{ MDBDescriptors::FIRST_CONST_USER_PROP };
	switch (m_property->type)
	{
	case MDBDescriptors::EPropertyType::CONSTANT:	  startVal = MDBDescriptors::FIRST_CONST_USER_PROP; break;
	case MDBDescriptors::EPropertyType::TP_DEPENDENT: startVal = MDBDescriptors::FIRST_TPDEP_USER_PROP; break;
	case MDBDescriptors::EPropertyType::INTERACTION:  startVal = MDBDescriptors::FIRST_INTER_USER_PROP; break;
	}

	for (unsigned i = startVal; i < startVal + MDBDescriptors::MAX_USER_DEFINED_PROP_NUMBER; ++i)
		if (!VectorContains(m_activeProperties, i))
			return i;

	return CONST_PROP_NO_PROERTY;
}
