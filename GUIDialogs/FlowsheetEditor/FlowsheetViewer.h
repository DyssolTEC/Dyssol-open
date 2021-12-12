/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_FlowsheetViewer.h"
#include "GraphvizHandler.h"

class QSettings;
class CFlowsheet;
class QLabel;
class QScrollArea;

/*
 * Uses graphviz library to render flowsheet as a picture and shows it.
 */
class CFlowsheetViewer : public QDialog
{
	Q_OBJECT

	Ui::CFlowsheetViewer ui{};

	const double MIN_SCALE       = 0.2;	// Minimum scale factor for showing image.
	const double MAX_SCALE       = 4.0;	// Maximum scale factor for showing image.
	const double DEFAULT_SCALE   = 1.0;	// Default scale factor for showing image.
	const double ZOOM_IN_FACTOR  = 1.1;	// Scaling factor for each zoom in step.
	const double ZOOM_OUT_FACTOR = 0.9;	// Scaling factor for each zoom out step.

	QSettings* m_settings{};				// Pointer to global settings.
	const CFlowsheet* m_flowsheet{};		// Pointer to flowsheet.
	QString m_imageFullName;				// Path and name to store temporary images.

	CGraphvizHandler m_graphBuilder{ m_flowsheet };	// Image builder.
	QImage m_image;									// Current image.
	double m_scaleFactor{ DEFAULT_SCALE };			// Current image scale factor relative to original size.
	QPoint m_lastMousePos;							// Last mouse position needed to track mouse movements.

public:
	CFlowsheetViewer(const CFlowsheet* _flowsheet, QSettings* _settings, QWidget* _parent = nullptr);
	~CFlowsheetViewer() override;

	CFlowsheetViewer(const CFlowsheetViewer& _other)                = delete;
	CFlowsheetViewer(CFlowsheetViewer&& _other) noexcept            = delete;
	CFlowsheetViewer& operator=(const CFlowsheetViewer& _other)     = delete;
	CFlowsheetViewer& operator=(CFlowsheetViewer&& _other) noexcept = delete;

	// Connects signals and slots.
	void InitializeConnections() const;

	// Changes visibility of this dialog.
	void setVisible(bool _visible) override;

	// Completely updates view using data from flowsheet.
	void Update();

private:
	// Creates main menu of the dialog.
	void CreateMenu();
	// Reaction on action Save As.
	void SaveAs();
	// Reaction on action Fit To Window.
	void FitToWindow();
	// Resize image according to given factor.
	void ScaleImage(double _factor);
	// Shows image with current scaling factor.
	void ShowImage() const;
	// Updates cursor view depending on the state of scroll bars.
	void UpdateCursor();
	// Checks if the shown image can be moved.
	[[nodiscard]] bool IsImageMovable() const;
	// Changes rendering style.
	void StyleChanged();

	// Load view settings.
	void LoadSettings();
	// Save view settings.
	void SaveSettings() const;

	void mouseMoveEvent(QMouseEvent* _event) override;
	void mousePressEvent(QMouseEvent* _event) override;
	void mouseReleaseEvent(QMouseEvent* _event) override;
	void wheelEvent(QWheelEvent* _event) override;
	void resizeEvent(QResizeEvent* _event) override;
	void changeEvent(QEvent* _event) override;
	bool eventFilter(QObject* _object, QEvent* _event) override;
};
