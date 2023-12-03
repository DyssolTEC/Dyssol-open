/* Copyright (c) 2021, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "FlowsheetViewer.h"
#include "Flowsheet.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <QImageReader>
#include <QMenuBar>
#include <QFileDialog>
#include <QScrollBar>
#include <QWheelEvent>
#include <QSettings>

namespace fs = std::filesystem;

CFlowsheetViewer::CFlowsheetViewer(const CFlowsheet* _flowsheet, QWidget* _parent)
	: QDialog{ _parent }
	, m_flowsheet{ _flowsheet }
{
	ui.setupUi(this);
	ui.scrollArea->viewport()->setStyleSheet("background-color: white;");
	ui.scrollArea->viewport()->installEventFilter(this);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	InitializeConnections();
}

CFlowsheetViewer::~CFlowsheetViewer()
{
	try
	{
		// remove image
		if (fs::exists(m_imageFullName.toStdU16String()))
			fs::remove(m_imageFullName.toStdU16String());
	}
	catch (...)
	{
		std::cerr << "Exception thrown in CFlowsheetViewer::~CFlowsheetViewer()" << std::endl;
	}
}

void CFlowsheetViewer::SetPointers(CModelsManager* _modelsManager, QSettings* _settings)
{
	CDyssolBaseWidget::SetPointers(_modelsManager, _settings);

	// where to temporary store images
	const std::filesystem::path cachePath = m_settings->value(StrConst::Dyssol_ConfigCachePath).toString().toStdString() + StrConst::Dyssol_CacheDir;
	if (!fs::exists(cachePath))
		fs::create_directory(cachePath);
	const fs::path path = fs::exists(cachePath) ? cachePath : fs::current_path();
	m_imageFullName = QString::fromStdU16String((path / (StringFunctions::GenerateRandomKey() + ".png")).u16string());

	LoadSettings();
	CreateMenu();
}

void CFlowsheetViewer::InitializeConnections() const
{
	connect(ui.actionSaveAs                     , &QAction::triggered      , this, &CFlowsheetViewer::SaveAs);
	connect(ui.actionFitToWindow                , &QAction::triggered      , this, &CFlowsheetViewer::FitToWindow);
	connect(ui.actionShowPorts                  , &QAction::triggered      , this, &CFlowsheetViewer::StyleChanged);
	connect(ui.actionHorizontalLayout           , &QAction::triggered      , this, &CFlowsheetViewer::StyleChanged);
	connect(ui.actionVerticalLayout             , &QAction::triggered      , this, &CFlowsheetViewer::StyleChanged);
	connect(ui.scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, this, &CFlowsheetViewer::UpdateCursor);
	connect(ui.scrollArea->verticalScrollBar()  , &QScrollBar::rangeChanged, this, &CFlowsheetViewer::UpdateCursor);
}

void CFlowsheetViewer::setVisible(bool _visible)
{
	QDialog::setVisible(_visible);
	if (_visible)
		Update();
}

void CFlowsheetViewer::Update()
{
	if (!isVisible()) return;

	// generate and save image
	if (!m_graphBuilder.SaveToFile(m_imageFullName.toStdString())) return;
	// load image
	m_image = QImageReader{ m_imageFullName }.read();
	// show image with appropriate size
	FitToWindow();
}

void CFlowsheetViewer::CreateMenu()
{
	auto* mainMenu = new QMenuBar{ this };
	ui.mainLayout->setMenuBar(mainMenu);
	auto* fileMenu = mainMenu->addMenu("&File");
	fileMenu->addAction(ui.actionSaveAs);
	auto* viewMenu = mainMenu->addMenu("&View");
	viewMenu->addAction(ui.actionShowPorts);
	ui.actionShowPorts->setChecked(m_graphBuilder.Style() == CGraphvizHandler::EStyle::WITH_PORTS);
	auto* layoutMenu = viewMenu->addMenu("&Layout");
	layoutMenu->addAction(ui.actionHorizontalLayout);
	layoutMenu->addAction(ui.actionVerticalLayout);
	auto* layoutGroup = new QActionGroup{ this };
	layoutGroup->addAction(ui.actionHorizontalLayout);
	layoutGroup->addAction(ui.actionVerticalLayout);
	(m_graphBuilder.Layout() == CGraphvizHandler::ELayout::HORIZONTAL ? ui.actionHorizontalLayout : ui.actionVerticalLayout)->setChecked(true);
	viewMenu->addSeparator();
	viewMenu->addAction(ui.actionFitToWindow);
}

void CFlowsheetViewer::SaveAs()
{
	const auto defaultFileName = m_flowsheet->GetFileName().parent_path() / m_flowsheet->GetFileName().stem();
	const auto dir = QString::fromStdString(defaultFileName.string() + ".png");
	const QString file = QFileDialog::getSaveFileName(this, StrConst::FV_DialogSaveAs, dir, StrConst::FV_DialogSaveAsFilter);
	if (file.isEmpty()) return;
	[[maybe_unused]] const auto res = m_graphBuilder.SaveToFile(file.toStdString());
}

void CFlowsheetViewer::FitToWindow()
{
	const auto factorW = static_cast<double>(ui.scrollArea->viewport()->width () - 2 * ui.gridLayout->margin()) / static_cast<double>(m_image.width ());
	const auto factorH = static_cast<double>(ui.scrollArea->viewport()->height() - 2 * ui.gridLayout->margin()) / static_cast<double>(m_image.height());
	m_scaleFactor = std::min(factorW, factorH);
	ShowImage();
}

void CFlowsheetViewer::ScaleImage(double _factor)
{
	m_scaleFactor = std::clamp(m_scaleFactor * _factor, MIN_SCALE, MAX_SCALE);
	ShowImage();
}

void CFlowsheetViewer::ShowImage() const
{
	if (m_image.isNull()) return;
	ui.labelImage->setPixmap(QPixmap::fromImage(m_image.scaled(m_image.size() * m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	ui.labelImage->adjustSize();
}

void CFlowsheetViewer::UpdateCursor()
{
	setCursor(IsImageMovable() ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

bool CFlowsheetViewer::IsImageMovable() const
{
	return ui.scrollArea->horizontalScrollBar()->maximum() || ui.scrollArea->verticalScrollBar()->maximum();
}

void CFlowsheetViewer::StyleChanged()
{
	m_graphBuilder.SetStyle(ui.actionShowPorts->isChecked() ? CGraphvizHandler::EStyle::WITH_PORTS : CGraphvizHandler::EStyle::SIMPLE);
	m_graphBuilder.SetLayout(ui.actionHorizontalLayout->isChecked() ? CGraphvizHandler::ELayout::HORIZONTAL : CGraphvizHandler::ELayout::VERTICAL);
	SaveSettings();
	Update();
}

void CFlowsheetViewer::LoadSettings()
{
	m_graphBuilder.SetStyle (static_cast<CGraphvizHandler::EStyle >(m_settings->value(StrConst::FV_ConfigStyle ).toUInt()));
	m_graphBuilder.SetLayout(static_cast<CGraphvizHandler::ELayout>(m_settings->value(StrConst::FV_ConfigLayout).toUInt()));
}

void CFlowsheetViewer::SaveSettings() const
{
	m_settings->setValue(StrConst::FV_ConfigStyle , E2I(m_graphBuilder.Style ()));
	m_settings->setValue(StrConst::FV_ConfigLayout, E2I(m_graphBuilder.Layout()));
}

void CFlowsheetViewer::mouseMoveEvent(QMouseEvent* _event)
{
	if (!IsImageMovable()) return;

	if (_event->buttons() & Qt::LeftButton)
	{
		const auto dx = _event->x() - m_lastMousePos.x();
		const auto dy = _event->y() - m_lastMousePos.y();
		ui.scrollArea->horizontalScrollBar()->setValue(ui.scrollArea->horizontalScrollBar()->value() - dx);
		ui.scrollArea->verticalScrollBar()  ->setValue(ui.scrollArea->verticalScrollBar()  ->value() - dy);
	}

	m_lastMousePos = _event->pos();
}

void CFlowsheetViewer::mousePressEvent(QMouseEvent* _event)
{
	if (!IsImageMovable()) return;
	setCursor(Qt::ClosedHandCursor);
	m_lastMousePos = _event->pos();
}

void CFlowsheetViewer::mouseReleaseEvent(QMouseEvent* _event)
{
	if (!IsImageMovable()) return;
	setCursor(Qt::OpenHandCursor);
	QDialog::mouseReleaseEvent(_event);
}

void CFlowsheetViewer::wheelEvent(QWheelEvent* _event)
{
	const double steps = _event->angleDelta().y() / 120.0;	// number of zooming steps
	const auto zoomFactor = steps > 0.0 ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;
	m_scaleFactor = std::clamp(m_scaleFactor * std::pow(zoomFactor, std::abs(steps)), MIN_SCALE, MAX_SCALE);
	ShowImage();
}

void CFlowsheetViewer::resizeEvent(QResizeEvent* _event)
{
	UpdateCursor();
	QDialog::resizeEvent(_event);
}

void CFlowsheetViewer::changeEvent(QEvent* _event)
{
	if (_event->type() == QEvent::WindowStateChange)
	{
		const auto prev = dynamic_cast<QWindowStateChangeEvent*>(_event)->oldState();
		const auto curr = windowState();
		if (prev == Qt::WindowNoState && curr == Qt::WindowMaximized || prev == Qt::WindowMaximized && curr == Qt::WindowNoState)
			FitToWindow();
	}
	QDialog::changeEvent(_event);
}

bool CFlowsheetViewer::eventFilter(QObject* _object, QEvent* _event)
{
	// consume wheel event from scroll area to disable scrolling with the wheel
	if (_object == ui.scrollArea->viewport() && _event->type() == QEvent::Wheel)
		return true;
	return false;
}
