/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ModelsManager.h"
#include <QDialog>
#include <QSettings>

class CDyssolBaseWidget
{
protected:
	QSettings* m_settings{};           // Pointer to global settings.
	CModelsManager* m_modelsManager{}; // Pointer to models manager.

public:
	CDyssolBaseWidget() = default;
	CDyssolBaseWidget(const CDyssolBaseWidget& _other) = default;
	CDyssolBaseWidget(CDyssolBaseWidget&& _other) noexcept = default;
	CDyssolBaseWidget& operator=(const CDyssolBaseWidget& _other) = default;
	CDyssolBaseWidget& operator=(CDyssolBaseWidget&& _other) noexcept = default;
	virtual ~CDyssolBaseWidget() = default;

	/**
	 * \brief Sets all required pointers.
	 * \param _modelsManager Pointer to models manager.
	 * \param _settings pointer to global settings.
	 */
	virtual void SetPointers(CModelsManager* _modelsManager, QSettings* _settings);

	/**
	 * \brief Returns pointer to models manager.
	 * \return Pointer to models manager.
	 */
	[[nodiscard]] CModelsManager* GetModelsManager() const;
	/**
	 * \brief Returns pointer to global settings.
	 * \return Pointer to global settings.
	 */
	[[nodiscard]] QSettings* GetSettings() const;
};

class CQtDialog
	: public QDialog
	, public CDyssolBaseWidget
{
	Q_OBJECT

	QString m_helpLink;

public:
	CQtDialog(QWidget* _parent);
	CQtDialog(QWidget* _parent, Qt::WindowFlags _flags);

	void SetHelpLink(const QString& _helpLink);
	QString GetHelpLink();

	/**
	 * Opens help file.
	 */
	void OpenHelp(const QString& _link);
};
