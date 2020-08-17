/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_AboutWindow.h"

class CAboutWindow : public QDialog
{
	Q_OBJECT

	struct S3rdParty
	{
		QString name;
		QString link;
		QString text;
		QString licenseName;
		QString licenseLink;
	};

	Ui::CAboutWindow ui{};

	QString m_headerProgramName;
	QString m_headerTeamName;
	QString m_headerUpdatesLink;

	std::vector<QString> m_mainDevelopers;
	std::vector<QString> m_otherDevelopers;

	std::vector<S3rdParty> m_libraries;

public:
	CAboutWindow(QWidget* parent = nullptr);

private:
	void InitializeConnections() const;

	void SetHeaderText() const;
	void SetLicense() const;
	void SetContributors() const;
	void SetThirdParties() const;
};
