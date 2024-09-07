/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ModelsManager.h"
#include <QDialog>
#include <QSettings>

class CFlowsheet;

class CDyssolBaseWidget
{
protected:
	CFlowsheet* m_flowsheet{};							// Pointer to the global flowsheet.
	QSettings* m_settings{};							// Pointer to global settings.
	CModelsManager* m_modelsManager{};					// Pointer to models manager.

	std::vector<CDyssolBaseWidget*> m_childWidgets{};	// Pointers to child widgets.

public:
	CDyssolBaseWidget() = default;
	CDyssolBaseWidget(const CDyssolBaseWidget& _other) = default;
	CDyssolBaseWidget(CDyssolBaseWidget&& _other) noexcept = default;
	CDyssolBaseWidget& operator=(const CDyssolBaseWidget& _other) = default;
	CDyssolBaseWidget& operator=(CDyssolBaseWidget&& _other) noexcept = default;
	virtual ~CDyssolBaseWidget() = default;

	/**
	 * \brief Sets pointers to child widgets.
	 * \param _widgets List of pointers to child widgets.
	 */
	void SetChildWidgets(std::initializer_list<CDyssolBaseWidget*> _widgets);
	/**
	 * \brief Adds a pointer to a child widget.
	 * \param _widget Pointer to a child widget.
	 */
	void AddChildWidget(CDyssolBaseWidget* _widget);
	/**
	 * \brief Sets all required pointers.
	 * Also sets the pointers to all child widgets.
	 * \param _flowsheet Pointer to the flowsheet.
	 * \param _modelsManager Pointer to models manager.
	 * \param _settings pointer to global settings.
	 */
	virtual void SetPointers(CFlowsheet* _flowsheet, CModelsManager* _modelsManager, QSettings* _settings);
	/**
	 * \brief Is called after all external pointers are set.
	 */
	virtual void OnPointersSet() {}

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
	/**
	 * \brief Reaction to loading of a new flowsheet.
	 */
	virtual void OnNewFlowsheet();
	/**
	 * \brief Updates all information on the widget.
	 */
	virtual void UpdateWholeView() {}

	/**
	 * \brief Reaction on the change of visibility of the dialog.
	 * \param _flag
	 */
	void setVisible(bool _flag) override;
	/**
	 * \brief Reaction on the key event.
	 * \param _event Event.
	 */
	void keyPressEvent(QKeyEvent* _event) override;

signals:
	void DataChanged();
};
