/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DescriptionEditor.h"
#include "StringFunctions.h"
#include "DefinesMDB.h"

CDescriptionEditor::CDescriptionEditor(CDescriptable* _object, QWidget *parent)
	: QDialog(parent),
	m_pObject(_object)
{
	ui.setupUi(this);

	SetText(m_pObject->GetDescription());
	m_sOldText = m_pObject->GetDescription();
	m_bTextChanged = false;

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &CDescriptionEditor::OkClicked);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &CDescriptionEditor::reject);
}

bool CDescriptionEditor::IsDescriptionChanged() const
{
	return m_bTextChanged;
}

QString CDescriptionEditor::TextToDisplay(const std::string& _text)
{
	return QString::fromStdString(StringFunctions::ReplaceAll(_text, MDBDescriptors::NEW_LINE_REPLACER, "\n"));
}

std::string CDescriptionEditor::TextToSave(const QString& _text)
{
	return StringFunctions::ReplaceAll(_text.toStdString(), "\n", MDBDescriptors::NEW_LINE_REPLACER);
}

void CDescriptionEditor::SetText(const std::string& _text) const
{
	const QString sWithLineBreaks = TextToDisplay(_text);
	ui.textEdit->setPlainText(sWithLineBreaks);
}

void CDescriptionEditor::OkClicked()
{
	if (!m_pObject) return;
	const QString sNewText = ui.textEdit->toPlainText();
	const std::string sWithoutLineBreaks = TextToSave(sNewText);
	m_bTextChanged = sWithoutLineBreaks != m_sOldText;
	m_pObject->SetDescription(sWithoutLineBreaks);
	QDialog::accept();
}

