/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "FlowsheetViewer.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <graphviz/gvc.h>
#include <QImageReader>
#include <QMenuBar>
#include <QFileDialog>
#include <QScrollBar>
#include <QWheelEvent>
#include <QSettings>

namespace fs = std::filesystem;

CFlowsheetViewer::CFlowsheetViewer(const CFlowsheet* _flowsheet, QSettings* _settings, QWidget* _parent)
	: QDialog{ _parent }
	, m_settings{ _settings }
	, m_flowsheet{ _flowsheet }
{
	ui.setupUi(this);
	ui.scrollArea->viewport()->installEventFilter(this);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	// where to temporary store images
	const std::filesystem::path cachePath = _settings->value(StrConst::Dyssol_ConfigCachePath).toString().toStdString() + StrConst::Dyssol_CacheDir;
	if (!fs::exists(cachePath))
		fs::create_directory(cachePath);
	const fs::path path = fs::exists(cachePath) ? cachePath : fs::current_path();
	m_imageFullName = path / (StringFunctions::GenerateRandomKey() + ".png");

	LoadSettings();
	CreateMenu();
	InitializeConnections();
}

CFlowsheetViewer::~CFlowsheetViewer()
{
	try
	{
		// remove image
		if (fs::exists(m_imageFullName))
			fs::remove(m_imageFullName);
	}
	catch (...)
	{
		std::cerr << "Exception thrown in CFlowsheetViewer::~CFlowsheetViewer()" << std::endl;
	}
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
	if (!SaveToFile(m_imageFullName)) return;
	// load image
	m_image = QImageReader{ QString::fromStdString(m_imageFullName.string()) }.read();
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
	ui.actionShowPorts->setChecked(m_style == EStyle::WITH_PORTS);
	auto* layoutMenu = viewMenu->addMenu("&Layout");
	layoutMenu->addAction(ui.actionHorizontalLayout);
	layoutMenu->addAction(ui.actionVerticalLayout);
	auto* layoutGroup = new QActionGroup{ this };
	layoutGroup->addAction(ui.actionHorizontalLayout);
	layoutGroup->addAction(ui.actionVerticalLayout);
	(m_layout == ELayout::HORIZONTAL ? ui.actionHorizontalLayout : ui.actionVerticalLayout)->setChecked(true);
	viewMenu->addSeparator();
	viewMenu->addAction(ui.actionFitToWindow);
}

void CFlowsheetViewer::SaveAs()
{
	const auto defaultFileName = m_flowsheet->GetFileName().parent_path() / m_flowsheet->GetFileName().stem();
	const auto dir = QString::fromStdString(defaultFileName.string() + ".png");
	const QString file = QFileDialog::getSaveFileName(this, StrConst::FV_DialogSaveAs, dir, StrConst::FV_DialogSaveAsFilter);
	if (file.isEmpty()) return;
	SaveToFile(fs::path{ file.toStdString() });
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
	m_style  = ui.actionShowPorts->isChecked() ? EStyle::WITH_PORTS : EStyle::SIMPLE;
	m_layout = ui.actionHorizontalLayout->isChecked() ? ELayout::HORIZONTAL : ELayout::VERTICAL;
	SaveSettings();
	Update();
}

void CFlowsheetViewer::LoadSettings()
{
	m_style  = static_cast<EStyle >(m_settings->value(StrConst::FV_ConfigStyle ).toUInt());
	m_layout = static_cast<ELayout>(m_settings->value(StrConst::FV_ConfigLayout).toUInt());
}

void CFlowsheetViewer::SaveSettings() const
{
	m_settings->setValue(StrConst::FV_ConfigStyle , E2I(m_style));
	m_settings->setValue(StrConst::FV_ConfigLayout, E2I(m_layout));
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

bool CFlowsheetViewer::SaveToFile(const fs::path& _fileName) const
{
	// whether saving was successful
	bool success = false;
	// try to crate and fill graph
	if (const auto graph = CreateGraph())
	{
		// try to create context
		if (GVC_t* context = gvContext())
		{
			// try to apply dot layout engine
			if (!gvLayout(context, graph, "dot"))
			{
				// get and check file extension
				if (const auto ext = fs::path{ _fileName }.extension().string(); !ext.empty())
				{
					// file rendering itself
					success = !gvRenderFilename(context, graph, ext.substr(1).c_str(), _fileName.string().c_str());
				}
				// clear layout
				gvFreeLayout(context, graph);
			}
			// clear context
			gvFreeContext(context);
		}
		// clear graph
		agclose(graph);
	}
	return success;
}

Agraph_t* CFlowsheetViewer::CreateGraph() const
{
	// temporary z-string values, needed because graphviz functions take non-const char* as parameters
	char empty[] = "";
	char attrNodesep[] = "nodesep";
	char attrFontsize[] = "fontsize";
	char attrRankdir[] = "rankdir";
	char attrShape[] = "shape";
	char attrLabel[] = "label";
	char attrHeadport[] = "headport";
	char attrTailport[] = "tailport";
	char valNodesep[] = "0.4";
	char valFontsize[] = "20";
	char valRankdirLR[] = "LR";
	char valRankdirTB[] = "TB";
	char valShapeBox[] = "box";
	char valShapePlain[] = "plaintext";

	// create directed graph
	Agraph_t* graph = agopen(empty, Agdirected, nullptr);
	agsafeset(graph, attrNodesep, valNodesep, empty);
	agsafeset(graph, attrFontsize, valFontsize, empty);
	switch (m_layout)
	{
	case ELayout::HORIZONTAL: agsafeset(graph, attrRankdir, valRankdirLR, empty); break;
	case ELayout::VERTICAL: agsafeset(graph, attrRankdir, valRankdirTB, empty); break;
	}

	// temporary storage of nodes
	std::map<std::string, Agnode_t*> nodes;

	// list units
	for (const auto& u : m_flowsheet->GetAllUnits())
	{
		if (!u->GetModel()) continue;

		char* nodeName = agstrdup(graph, u->GetKey().c_str());
		auto* node = agnode(graph, nodeName, 1);
		agstrfree(graph, nodeName);

		switch (m_style)
		{
		case EStyle::SIMPLE:
		{
			char* nodeLabel = agstrdup(graph, u->GetName().c_str());
			agsafeset(node, attrShape, valShapeBox, empty);
			agsafeset(node, attrLabel, nodeLabel, empty);
			agstrfree(graph, nodeLabel);
			break;
		}
		case EStyle::WITH_PORTS:
		{
			agsafeset(node, attrShape, valShapePlain, empty);
			const auto& ports = u->GetModel()->GetPortsManager();
			std::string html = "<TABLE BORDER = '1' CELLBORDER = '0' CELLSPACING = '0' CELLPADDING = '4'><TR>";
			if (!ports.GetAllInputPorts().empty())
			{
				html += "<TD><TABLE BORDER='0' CELLBORDER='1' CELLSPACING='0' CELLPADDING='4'>";
				for (const auto& port : ports.GetAllInputPorts())
					html += "<TR><TD PORT='" + port->GetName() + "'>" + port->GetName() + "</TD></TR>";
				html += "</TABLE></TD>";
			}
			html += "<TD>" + u->GetName() + "</TD>";
			if (!ports.GetAllOutputPorts().empty())
			{
				html += "<TD><TABLE BORDER='0' CELLBORDER='1' CELLSPACING='0' CELLPADDING='4'>";
				for (const auto& port : ports.GetAllOutputPorts())
					html += "<TR><TD PORT='" + port->GetName() + "'>" + port->GetName() + "</TD></TR>";
				html += "</TABLE></TD>";
			}
			html += "</TR></TABLE>";
			const char* nodeHtml = agstrdup_html(graph, html.c_str());
			agsafeset(node, attrLabel, nodeHtml, empty);
			agstrfree(graph, nodeHtml);
			break;
		}
		}

		nodes[u->GetKey()] = node;
	}

	// list streams
	for (const auto& c : m_flowsheet->GenerateConnectionsDescription())
	{
		const auto* stream = m_flowsheet->GetStream(c.stream);
		const auto* unitI = m_flowsheet->GetUnit(c.unitI);
		const auto* unitO = m_flowsheet->GetUnit(c.unitO);

		if (!stream || !unitI || !unitO || !nodes[unitO->GetKey()] || !nodes[unitI->GetKey()]) continue;

		char* edgeName = agstrdup(graph, stream->GetName().c_str());
		auto* edge = agedge(graph, nodes[unitO->GetKey()], nodes[unitI->GetKey()], edgeName, 1);
		agsafeset(edge, attrLabel, edgeName, empty);
		agstrfree(graph, edgeName);

		switch (m_style)
		{
		case EStyle::SIMPLE: break;
		case EStyle::WITH_PORTS:
		{
			agsafeset(edge, attrHeadport, c.portI.c_str(), empty);
			agsafeset(edge, attrTailport, c.portO.c_str(), empty);
		}
		}
	}

	return graph;
}