/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Descriptable.h"
#include "ui_DescriptionEditor.h"

class CDescriptionEditor : public QDialog
{
	Q_OBJECT
private:
	Ui::CDescriptionEditor ui;

	CDescriptable* m_pObject;
	std::string m_sOldText;
	bool m_bTextChanged;

public:
	CDescriptionEditor(CDescriptable* _object, QWidget *parent = Q_NULLPTR);

	bool IsDescriptionChanged() const;

	static QString TextToDisplay(const std::string& _text);
	static std::string TextToSave(const QString& _text);

private:
	void SetText(const std::string& _text) const;
	void OkClicked();
};
