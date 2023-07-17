/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtPlot.h"
#include <QMenu>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <cmath>

using namespace QtPlot;

const QString CQtPlot::m_csMenuCurves				= QString("Curves");
const QString CQtPlot::m_csMenuCurveName			= QString("Name...");
const QString CQtPlot::m_csMenuCurveVisibility		= QString("Visibility");
const QString CQtPlot::m_csMenuCurveColor			= QString("Color...");
const QString CQtPlot::m_csMenuCurveWidth			= QString("Width...");
const QString CQtPlot::m_csMenuCurveLines			= QString("Lines");
const QString CQtPlot::m_csMenuAutoZoom				= QString("Auto zoom");
const QString CQtPlot::m_csMenuSaveAsPicture		= QString("Save as picture...");
const QString CQtPlot::m_csMenuSaveAsCSVFile		= QString("Save as CSV-file...");
const QString CQtPlot::m_csMenuGridVisibility		= QString("Grid visibility");
const QString CQtPlot::m_csMenuGridLimits			= QString("Change grid settings...");
const QString CQtPlot::m_csMenuLegengPosition		= QString("Legend position");
const QString CQtPlot::m_csMenuLegengPositionNone	= QString("Hide");
const QString CQtPlot::m_csMenuLegengPositionTR		= QString("Set Top Right");
const QString CQtPlot::m_csMenuLegengPositionTL		= QString("Set Top Left");
const QString CQtPlot::m_csMenuLegengPositionBL		= QString("Set Bottom Left");
const QString CQtPlot::m_csMenuLegengPositionBR		= QString("Set Bottom Right");
const QString CQtPlot::m_csDialogSavePictureCaption = QString("Save Image");
const QString CQtPlot::m_csDialogSavePictureFilter	= QString("Image Files (*.png *.jpg *.bmp)");
const QString CQtPlot::m_csDialogSaveCSVFileCaption = QString("Save File");
const QString CQtPlot::m_csDialogSaveCSVFileFilter	= QString("Text Files (*.txt)");
const QString CQtPlot::m_csDialogSetWidthCaption	= QString("Curve Width");
const QString CQtPlot::m_csDialogSetWidthLable		= QString("Select new curve's width");
const QString CQtPlot::m_csDialogSetNameCaption		= QString("Curve Name");
const QString CQtPlot::m_csDialogSetNameLable		= QString("Select new curve's name");
const QString CQtPlot::m_csDialogSetColorCaption	= QString("Select Color");
const QString CQtPlot::m_csDialogSetLimitCaption	= QString("Limit position");
const QString CQtPlot::m_csDialogSetLimitLable		= QString("Select new limit position:");

const QString CQtPlot::m_csaLabels[] =
{
	QString(""),
	QString(""),
	QString("Time [s]"),
	QString("Force [N]"),
	QString("Displacement [m]"),
	QString("Sauter diameter [m]"),
	QString("Number [-]"),
	QString("Mass flow [kg/s]"),
	QString("Mass [kg]"),
	QString("Temperature [K]"),
	QString("Pressure [Pa]"),
	QString("Mass fraction [-]"),
	QString("Size [m]"),
	QString("q3 [1/m]"),
	QString("Value")
};

CQtPlot::CQtPlot(QWidget* parent, Qt::WindowFlags flags): QWidget(parent, flags)
{
	m_nPlotAreaOffsetTop = m_cPlotAreaOffset;

	if(parent)
		setGeometry(parent->rect());

	m_pMainTitle = new QLabel("", this);
	resizeEvent(NULL);
	m_pMainTitle->setFont(QFont("arial", 10));
	m_pMainTitle->setAlignment(Qt::AlignCenter);
	m_pMainTitle->show();
	SetPlotTitle("");

	m_dMinX = -5;
	m_dMaxX = 5;
	m_dDispMinX = -5;
	m_dDispMaxX = 5;
	m_bLogX = false;
	m_nNumXGridLines = 10;
	m_nXLabelsInterval = 1;
	m_dMinY = -5;
	m_dMaxY = 5;
	m_dDispMinY = -5;
	m_dDispMaxY = 5;
	m_bLogY = false;
	m_nNumYGridLines = 10;
	m_nYLabelsInterval = 1;

	m_bIsGridVisible = true;
	m_bFixedBoudaries = false;

	m_nLegendPosition = POSITION_TOP_RIGHT;

	m_bIsAxisLablesVisible = true;
	m_eLastChosenXLabel = LABEL_NONE;
	m_eLastChosenYLabel = LABEL_NONE;

	m_eLeftMouseMode = None;

	m_csNearestCurveIndex = -1;
	m_sDialogSavePicturePath = "";
	m_sDialogSaveCSVFilePath = "";

	m_pLimitsDialog = new CGridLimitsDialog(this);
}

CQtPlot::~CQtPlot()
{
	for (auto& c : m_vpCurves)
		delete c;
}

// Events handlers
void CQtPlot::resizeEvent(QResizeEvent* _resizeEvent)
{
	m_paintRect.setRect(
		m_cPlotAreaOffset + m_cCoordinateOffsetLeft,
		m_nPlotAreaOffsetTop + m_cCoordinateOffsetTop,
		this->width() - m_cPlotAreaOffset * 2 - m_cCoordinateOffsetLeft - m_cCoordinateOffsetRight,
		this->height() - m_cPlotAreaOffset - m_nPlotAreaOffsetTop - m_cCoordinateOffsetTop - m_cCoordinateOffsetBottom);

	m_pMainTitle->setGeometry(0, 0, this->width(), m_nPlotAreaOffsetTop);
}

void CQtPlot::paintEvent(QPaintEvent* _paintEvent)
{
	m_pPainter = new QPainter(this);

	m_pPainter->setRenderHint(QPainter::Antialiasing, true);
	m_pPainter->setPen(QPen(Qt::black, 0, Qt::SolidLine));

	// draw outer boundaries
	m_pPainter->drawRoundedRect(0, 0, this->width() - 1, this->height() - 1, 5, 5);

	// draw plot area
	QBrush whiteBrush(isEnabled() ? Qt::white : Qt::lightGray, Qt::SolidPattern);
	m_pPainter->fillRect(m_cPlotAreaOffset, m_nPlotAreaOffsetTop, this->width() - m_cPlotAreaOffset * 2, this->height() - m_cPlotAreaOffset - m_nPlotAreaOffsetTop, whiteBrush);

	// draw coordinates
	DrawGrid(m_pPainter);

	// draw curves
	DrawCurves(m_pPainter);

	DrawAxisLabels(m_pPainter);

	DrawLegend(m_pPainter);

	DrawMarks(m_pPainter);
}

void CQtPlot::wheelEvent(QWheelEvent* _wheelEvent)
{
	if (_wheelEvent->angleDelta().y() > 0)
		ZoomIn();
	else
		ZoomOut();
	_wheelEvent->accept();

}

void CQtPlot::mousePressEvent(QMouseEvent* _mouseEvent)
{
	m_previousMousePos = _mouseEvent->pos();

	if (m_paintRect.contains(_mouseEvent->pos()))
	{
		if ((_mouseEvent->buttons() & Qt::LeftButton) && (_mouseEvent->modifiers() & Qt::ShiftModifier))
		{
			m_eLeftMouseMode = Move;
		}
		else if ((_mouseEvent->buttons() & Qt::LeftButton) && (_mouseEvent->modifiers() & Qt::ControlModifier))
		{
			m_eLeftMouseMode = Mark;
			m_markPoint = _mouseEvent->pos();
			DetermineNearestCurve();
			RedrawPlot();
		}
		else if ((_mouseEvent->buttons() & Qt::LeftButton))
		{
			m_eLeftMouseMode = Zoom;
			m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
			m_pRubberBand->setGeometry(QRect(m_previousMousePos, QSize()));
			m_pRubberBand->show();
		}
	}
}

void CQtPlot::mouseMoveEvent(QMouseEvent* _mouseEvent)
{
	switch (m_eLeftMouseMode)
	{
	case Move:
	{
		double dCoefX, dCoefY;
		int nDiffX, nDiffY;
		// double dShiftX, dShiftY;
		if (m_paintRect.contains(_mouseEvent->pos()))
		{
			dCoefX = m_dSpanX() / m_paintRect.width();
			nDiffX = _mouseEvent->pos().x() - m_previousMousePos.x();
			// dShiftX = dCoefX * nDiffX;
			m_dMinX -= dCoefX * nDiffX;
			m_dMaxX -= dCoefX * nDiffX;

			dCoefY = m_dSpanY() / m_paintRect.height();
			nDiffY = _mouseEvent->pos().y() - m_previousMousePos.y();
			// dShiftY = dCoefY * nDiffY;
			m_dMinY += dCoefY * nDiffY;
			m_dMaxY += dCoefY * nDiffY;

			RedrawPlot();

			m_previousMousePos = _mouseEvent->pos();
		}
		break;
	}
	case Mark:
		m_markPoint = _mouseEvent->pos();
		if (m_markPoint.x() > m_paintRect.x() + m_paintRect.width())
			m_markPoint.setX(m_paintRect.x() + m_paintRect.width());
		else if (m_markPoint.x() < m_paintRect.x())
			m_markPoint.setX(m_paintRect.x());
		RedrawPlot();
		break;
	case Zoom:
		m_pRubberBand->setGeometry(QRect(m_previousMousePos, _mouseEvent->pos()).normalized());
		if (_mouseEvent->pos().x() <= m_paintRect.x())
			m_pRubberBand->setGeometry(m_paintRect.x(), m_pRubberBand->y(),
			                           m_pRubberBand->width() + (m_pRubberBand->x() - m_paintRect.x()), m_pRubberBand->height());
		if (_mouseEvent->pos().x() > m_paintRect.x() + m_paintRect.width())
			m_pRubberBand->setGeometry(m_pRubberBand->x(), m_pRubberBand->y(),
			                           m_paintRect.x() + m_paintRect.width() - m_pRubberBand->x(), m_pRubberBand->height());
		if (_mouseEvent->pos().y() <= m_paintRect.y())
			m_pRubberBand->setGeometry(m_pRubberBand->x(), m_paintRect.y(),
			                           m_pRubberBand->width(), m_pRubberBand->height() + (m_pRubberBand->y() - m_paintRect.y()));
		if (_mouseEvent->pos().y() > m_paintRect.y() + m_paintRect.height())
			m_pRubberBand->setGeometry(m_pRubberBand->x(), m_pRubberBand->y(),
			                           m_pRubberBand->width(), m_paintRect.y() + m_paintRect.height() - m_pRubberBand->y());
		break;
	default:
		break;
	}
}

void CQtPlot::mouseReleaseEvent(QMouseEvent* _mouseEvent)
{
	switch (m_eLeftMouseMode)
	{
	case Zoom:
		{
		auto qr = QRect(m_pRubberBand->x(), m_pRubberBand->y(), m_pRubberBand->width(), m_pRubberBand->height());
		SetNewBoundaries(&qr);
		delete m_pRubberBand;
		m_eLeftMouseMode = None;
		break;
		}
	case Mark:
		{
		m_eLeftMouseMode = None;
		m_csNearestCurveIndex = -1;
		RedrawPlot();
		break;
		}
	default:
		{
		m_eLeftMouseMode = None;
		m_csNearestCurveIndex = -1;
		break;
		}
	}
}

void CQtPlot::mouseDoubleClickEvent(QMouseEvent* _mouseEvent)
{
}

void CQtPlot::contextMenuEvent(QContextMenuEvent* _contextMenuEvent)
{
	QMenu* pMenu = new QMenu(this);
	m_markPoint = _contextMenuEvent->pos();
	DetermineNearestCurve();
	CreateDropMenu(pMenu);
	pMenu->exec(_contextMenuEvent->globalPos());
}

void CQtPlot::RedrawPlot()
{
	this->update();
}

void CQtPlot::changeEvent(QEvent* _event)
{
	QWidget::changeEvent(_event);
	if(_event->EnabledChange && !isEnabled())
	{
		m_eLastChosenXLabel = LABEL_NONE;
		m_eLastChosenYLabel = LABEL_NONE;
		SetLimits(-5, 5, -5, 5);
	}
}

// Drawing
void CQtPlot::CreateDropMenu(QMenu* _pMenu)
{
	if (m_csNearestCurveIndex == -1)
		CreateFullDropMenu(_pMenu);
	else
		CreateCurveDropMenu(_pMenu);
}

void CQtPlot::CreateFullDropMenu(QMenu* _pMenu)
{
	QMenu* pCurvesMenu = _pMenu->addMenu(m_csMenuCurves);

	// create menu items for curves

	for (int i = 0; i < m_vpCurves.size(); ++i)
	{
		QMenu* pCurvesSettingsMenu = pCurvesMenu->addMenu(m_vpCurves.at(i)->sCurveName);

		QAction* pActionCurveName = pCurvesSettingsMenu->addAction(m_csMenuCurveName);
		connect(pActionCurveName, &QAction::triggered, this, [=] { SetCurveNameSlot(i); });

		QAction* pActionCurveColor = pCurvesSettingsMenu->addAction(m_csMenuCurveColor);
		connect(pActionCurveColor, &QAction::triggered, this, [=] { SetCurveColorSlot(i); });

		QAction* pActionCurveWidth = pCurvesSettingsMenu->addAction(m_csMenuCurveWidth);
		connect(pActionCurveWidth, &QAction::triggered, this, [=] { SetCurveWidthSlot(i); });

		QAction* pActionCurveVisibility = pCurvesSettingsMenu->addAction(m_csMenuCurveVisibility);
		pActionCurveVisibility->setCheckable(true);
		pActionCurveVisibility->setChecked(m_vpCurves.at(i)->bVisibility);
		connect(pActionCurveVisibility, &QAction::triggered, this, [=] { ToggleCurveVisibilitySlot(i); });

		QAction* pActionCurveLines = pCurvesSettingsMenu->addAction(m_csMenuCurveLines);
		pActionCurveLines->setCheckable(true);
		pActionCurveLines->setChecked(m_vpCurves.at(i)->bLinesVisibility);
		connect(pActionCurveLines, &QAction::triggered, this, [=] { ToggleCurveLinesVisibilitySlot(i); });
	}

	_pMenu->addSeparator();

	// create common menu items

	QAction* pActionAutoZoom = _pMenu->addAction(m_csMenuAutoZoom);
	connect(pActionAutoZoom, &QAction::triggered, this, &CQtPlot::AutoZoomSlot);

	QAction* pActionSaveAsPicture = _pMenu->addAction(m_csMenuSaveAsPicture);
	connect(pActionSaveAsPicture, &QAction::triggered, this, &CQtPlot::SaveAsPictureSlot);

	QAction* pActionSaveAsCSVFile = _pMenu->addAction(m_csMenuSaveAsCSVFile);
	connect(pActionSaveAsCSVFile, &QAction::triggered, this, &CQtPlot::SaveAsCSVFileSlot);

	_pMenu->addSeparator();

	QAction* pActionGridVisibility = _pMenu->addAction(m_csMenuGridVisibility);
	pActionGridVisibility->setCheckable(true);
	pActionGridVisibility->setChecked(m_bIsGridVisible);
	connect(pActionGridVisibility, &QAction::triggered, this, &CQtPlot::ToggleGridVisibilitySlot);

	_pMenu->addSeparator();

	// create limits menu items

	QAction* pActionLimits = _pMenu->addAction(m_csMenuGridLimits);
	connect(pActionLimits, &QAction::triggered, this, &CQtPlot::SetLimitsSlot);

	_pMenu->addSeparator();

	// create legend menu items

	QMenu* pLegendMenu = _pMenu->addMenu(m_csMenuLegengPosition);

	QAction* pActionLegendPositionHide = pLegendMenu->addAction(m_csMenuLegengPositionNone);
	QAction* pActionLegendPositionTR   = pLegendMenu->addAction(m_csMenuLegengPositionTR);
	QAction* pActionLegendPositionTL   = pLegendMenu->addAction(m_csMenuLegengPositionTL);
	QAction* pActionLegendPositionBL   = pLegendMenu->addAction(m_csMenuLegengPositionBL);
	QAction* pActionLegendPositionBR   = pLegendMenu->addAction(m_csMenuLegengPositionBR);
	connect(pActionLegendPositionHide,	&QAction::triggered, this, [&] { SetLegendPositionSlot(POSITION_NONE); });
	connect(pActionLegendPositionTR,	&QAction::triggered, this, [&] { SetLegendPositionSlot(POSITION_TOP_RIGHT); });
	connect(pActionLegendPositionTL,	&QAction::triggered, this, [&] { SetLegendPositionSlot(POSITION_TOP_LEFT); });
	connect(pActionLegendPositionBL,	&QAction::triggered, this, [&] { SetLegendPositionSlot(POSITION_BOTTOM_LEFT); });
	connect(pActionLegendPositionBR,	&QAction::triggered, this, [&] { SetLegendPositionSlot(POSITION_BOTTOM_RIGHT); });
}

void CQtPlot::CreateCurveDropMenu(QMenu* _pMenu)
{
	// create menu items for curve

	QAction* pActionCurveName = _pMenu->addAction(m_csMenuCurveName);
	connect(pActionCurveName, &QAction::triggered, this, [&] { SetCurveNameSlot(m_csNearestCurveIndex); });

	QAction* pActionCurveColor = _pMenu->addAction(m_csMenuCurveColor);
	connect(pActionCurveColor, &QAction::triggered, this, [&] { SetCurveColorSlot(m_csNearestCurveIndex); });

	QAction* pActionCurveWidth = _pMenu->addAction(m_csMenuCurveWidth);
	connect(pActionCurveWidth, &QAction::triggered, this, [&] { SetCurveWidthSlot(m_csNearestCurveIndex); });

	QAction* pActionCurveVisibility = _pMenu->addAction(m_csMenuCurveVisibility);
	pActionCurveVisibility->setCheckable(true);
	pActionCurveVisibility->setChecked(m_vpCurves.at(m_csNearestCurveIndex)->bVisibility);
	connect(pActionCurveVisibility, &QAction::triggered, this, [&] { ToggleCurveVisibilitySlot(m_csNearestCurveIndex); });

	QAction* pActionCurveLines = _pMenu->addAction(m_csMenuCurveLines);
	pActionCurveLines->setCheckable(true);
	pActionCurveLines->setChecked(m_vpCurves.at(m_csNearestCurveIndex)->bLinesVisibility);
	connect(pActionCurveLines, &QAction::triggered, this, [&] { ToggleCurveLinesVisibilitySlot(m_csNearestCurveIndex); });
}

void CQtPlot::DrawGrid(QPainter* _painter)
{
	QPen oldPen = _painter->pen();

	_painter->setRenderHint(QPainter::Antialiasing, false);
	_painter->setPen(QPen(Qt::black, 0, Qt::DotLine));

	unsigned nMarkNumber = 0;
	for (unsigned i = 0; i <= m_nNumXGridLines; ++i)
	{
		int nX = m_paintRect.left() + (i * (m_paintRect.width() - 1) / m_nNumXGridLines);

		double dLabel = m_dMinX + (i * m_dSpanX() / m_nNumXGridLines);
		if (m_bLogX)
		{
			dLabel = std::pow(10, dLabel);
		}

		if (m_bIsGridVisible)
			_painter->drawLine(nX, m_paintRect.top(), nX, m_paintRect.bottom());
		_painter->drawLine(nX, m_paintRect.bottom(), nX, m_paintRect.bottom() + 5);
		if (nMarkNumber % m_nXLabelsInterval == 0)
			_painter->drawText(nX - 50, m_paintRect.bottom() + 5, 100, 15, Qt::AlignHCenter | Qt::AlignTop, QString::number(dLabel, 'g', 3));
		nMarkNumber++;
	}

	nMarkNumber = 0;
	for (unsigned j = 0; j <= m_nNumYGridLines; ++j)
	{
		int nY = m_paintRect.bottom() - (j * (m_paintRect.height() - 1) / m_nNumYGridLines);

		double dLabel = m_dMinY + (j * m_dSpanY() / m_nNumYGridLines);
		if (m_bLogY)
		{
			dLabel = std::pow(10, dLabel);
		}

		if (m_bIsGridVisible)
			_painter->drawLine(m_paintRect.left(), nY, m_paintRect.right(), nY);
		_painter->drawLine(m_paintRect.left() - 5, nY, m_paintRect.left(), nY);
		if (nMarkNumber % m_nYLabelsInterval == 0)
			_painter->drawText(m_paintRect.left() - m_cCoordinateOffsetLeft, nY - 10, m_cCoordinateOffsetLeft - 7, 20, Qt::AlignRight | Qt::AlignVCenter, QString::number(dLabel, 'g', 3));
		nMarkNumber++;
	}
	_painter->setPen(QPen(Qt::black, m_cCoordinateRectPenWidth, Qt::SolidLine));
	_painter->drawRect(m_paintRect);

	_painter->setPen(oldPen);
}

void CQtPlot::DrawCurves(QPainter* _painter)
{
	QPen oldPen = _painter->pen();

	_painter->setClipRect(m_paintRect.x() + 1, m_paintRect.y() + 1, m_paintRect.width() - 2, m_paintRect.height() - 2);
	_painter->setRenderHint(QPainter::Antialiasing, true);
	for (int i = 0; i < m_vpCurves.size(); ++i)
	{
		if (m_vpCurves.at(i)->bVisibility)
		{
			QPointF* pPoints = new QPointF[m_vpCurves.at(i)->points.size()];
			int nNumPoints = 0;
			for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
			{
				double dX = m_vpCurves.at(i)->points.at(j).x() - m_dMinX;
				double x = m_paintRect.left() + (dX * (m_paintRect.width() - 1) / (m_dSpanX() != 0.0 ? m_dSpanX() : 0.001));

				double dY = m_vpCurves.at(i)->points.at(j).y() - m_dMinY;
				double y = m_paintRect.bottom() - (dY * (m_paintRect.height() - 1) / (m_dSpanY() != 0.0 ? m_dSpanY() : 0.001));

				if (std::isinf(x) || std::isnan(x)) x = 0.0;
				if (std::isinf(y) || std::isnan(y)) y = 0.0;

				pPoints[nNumPoints] = QPointF(x, y);
				nNumPoints++;
			}
			if (m_vpCurves.at(i)->bLinesVisibility)
			{
				_painter->setPen(QPen(m_vpCurves.at(i)->color, m_vpCurves.at(i)->nLineWidth, Qt::SolidLine));
				_painter->drawPolyline(pPoints, nNumPoints);
			}
			else
			{
				_painter->setPen(QPen(m_vpCurves.at(i)->color, m_vpCurves.at(i)->nLineWidth * 2, Qt::SolidLine, Qt::RoundCap));
				_painter->drawPoints(pPoints, nNumPoints);
			}
		}
	}
	_painter->setClipRect(rect(), Qt::NoClip);

	_painter->setPen(oldPen);
}

void CQtPlot::DrawAxisLabels(QPainter* _painter)
{
	if (m_bIsAxisLablesVisible)
	{
		_painter->save();
		_painter->translate(0, m_cCoordinateOffsetBottom);
		//_painter->drawText(m_paintRect, Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap, GetXLabel());
		QTextDocument tdX;
		tdX.setTextWidth(m_paintRect.width());
		tdX.setHtml("<p align=center vertical-align=bottom>" + GetXLabel() + "</p>");
		_painter->translate(m_paintRect.left(), m_paintRect.bottom() - tdX.documentLayout()->documentSize().height() + tdX.documentMargin());
		tdX.drawContents(_painter);
		_painter->restore();

		_painter->save();
		_painter->translate(m_cPlotAreaOffset, height());
		_painter->rotate(-90);
		//_painter->drawText(QRectF(m_cPlotAreaOffset + m_cCoordinateOffsetBottom, 0, m_paintRect.height(), m_cCoordinateOffsetLeft), Qt::AlignHCenter | Qt::TextWordWrap, GetYLabel());
		QTextDocument tdY;
		tdY.setTextWidth(m_paintRect.height());
		tdY.setHtml("<p align=center>" + GetYLabel() + "</p>");
		_painter->translate(m_cPlotAreaOffset + m_cCoordinateOffsetBottom, 0);
		tdY.drawContents(_painter);
		_painter->restore();
	}
}

void CQtPlot::DrawLegend(QPainter* _painter)
{
	if (m_nLegendPosition != POSITION_NONE)
	{
		QFont font = _painter->font();
		QFontMetrics fm(font);

		int nTextHeight = fm.height();
		int nMaxTextWidth = 0;
		int nLinesNumber = 0;
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			int nTextWidth = fm.horizontalAdvance(m_vpCurves.at(i)->sCurveName);
			if ((m_vpCurves.at(i)->bVisibility) && (nTextWidth > nMaxTextWidth))
				nMaxTextWidth = nTextWidth;
			if (m_vpCurves.at(i)->bVisibility)
				nLinesNumber++;
		}
		if (nMaxTextWidth != 0)
		{
			QPen oldPen = _painter->pen();

			_painter->setRenderHint(QPainter::Antialiasing, false);
			_painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
			_painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
			_painter->setFont(font);

			int nX = 0, nY = 0;
			int nWidth = nMaxTextWidth + nTextHeight + 4;
			int nHeight = nTextHeight * nLinesNumber + 1;
			switch (m_nLegendPosition)
			{
			case POSITION_TOP_RIGHT:
				nX = m_paintRect.x() - m_cCoordinateRectPenWidth - 2 + m_paintRect.width() - nWidth;
				nY = m_paintRect.y() + m_cCoordinateRectPenWidth + 1;
				break;
			case POSITION_TOP_LEFT:
				nX = m_paintRect.x() + m_cCoordinateRectPenWidth + 1;
				nY = m_paintRect.y() + m_cCoordinateRectPenWidth + 1;
				break;
			case POSITION_BOTTOM_LEFT:
				nX = m_paintRect.x() + m_cCoordinateRectPenWidth + 1;
				nY = m_paintRect.y() - m_cCoordinateRectPenWidth - 2 + m_paintRect.height() - nHeight;
				break;
			case POSITION_BOTTOM_RIGHT:
				nX = m_paintRect.x() - m_cCoordinateRectPenWidth - 2 + m_paintRect.width() - nWidth;
				nY = m_paintRect.y() - m_cCoordinateRectPenWidth - 2 + m_paintRect.height() - nHeight;
				break;
			default:
				break;
			}

			QRectF legendRect(nX, nY, nWidth, nHeight);
			_painter->drawRect(legendRect);

			int nLineIndex = 0;
			for (int i = 0; i < m_vpCurves.size(); ++i)
			{
				if (m_vpCurves.at(i)->bVisibility)
				{
					_painter->fillRect(legendRect.x() + 2, legendRect.y() + (nTextHeight * nLineIndex) + 2,  nTextHeight - 2, nTextHeight - 2, m_vpCurves.at(i)->color);
					_painter->drawText(legendRect.x() + nTextHeight + 2, legendRect.y() + (nTextHeight * nLineIndex), legendRect.width(), legendRect.height(), Qt::AlignLeft, m_vpCurves.at(i)->sCurveName);
					nLineIndex++;
				}
			}
			_painter->setPen(oldPen);
		}
	}
}

void CQtPlot::DrawMarks(QPainter* _painter)
{
	if ((m_eLeftMouseMode == Mark) && (m_csNearestCurveIndex != -1))
	{
		// get x-coordinate of m_markPoint
		double dX = GetXCoord(m_markPoint.x());
		// if m_markPoint crosses the curve
		for (int j = 0; j < m_vpCurves.at(m_csNearestCurveIndex)->points.size() - 1; ++j)
		{
			// between which points of curve is m_markPoint situated
			if ((m_vpCurves.at(m_csNearestCurveIndex)->points.at(j).x() <= dX) && (m_vpCurves.at(m_csNearestCurveIndex)->points.at(j + 1).x() >= dX))
			{
				QPen oldPen = _painter->pen();
				double dPixelX = m_markPoint.x();
				// get y-coordinate of curve according to m_markPoint
				double dTemp = (dX - m_vpCurves.at(m_csNearestCurveIndex)->points.at(j).x()) / (m_vpCurves.at(m_csNearestCurveIndex)->points.at(j + 1).x() - dX);
				double dY = (dTemp * m_vpCurves.at(m_csNearestCurveIndex)->points.at(j + 1).y() + m_vpCurves.at(m_csNearestCurveIndex)->points.at(j).y()) / (dTemp + 1);
				// get screen y-coordinate
				double dPixelY = m_paintRect.bottom() - ((dY - m_dMinY) * (m_paintRect.height() - 1) / (m_dSpanY() != 0.0 ? m_dSpanY() : 0.001));
				if (dPixelY < m_paintRect.y())
				{
					dPixelY = m_paintRect.y();
					_painter->setPen(QPen(Qt::transparent, 0, Qt::SolidLine, Qt::RoundCap));
				}
				else if (dPixelY >= m_paintRect.y() + m_paintRect.height())
				{
					dPixelY = m_paintRect.y() + m_paintRect.height();
					_painter->setPen(QPen(Qt::transparent, 0, Qt::SolidLine, Qt::RoundCap));
				}
				else
					_painter->setPen(QPen(Qt::black, 10, Qt::SolidLine, Qt::RoundCap));
				// draw point on curve
				_painter->setRenderHint(QPainter::Antialiasing, true);
				_painter->drawPoint(QPointF(dPixelX, dPixelY));
				// draw lines between point on curve and axis
				_painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
				_painter->setRenderHint(QPainter::Antialiasing, false);
				_painter->drawLine(dPixelX, dPixelY, dPixelX, m_paintRect.y() + m_paintRect.height());
				_painter->drawLine(dPixelX, dPixelY, m_paintRect.x(), dPixelY);
				// draw coordinates of point on curve
				QFont font = _painter->font();
				QFontMetrics fm(font);

				// determine points to display on screen in case of logarithmic scaling
				double dXPrint = dX;
				if (m_bLogX)
					dXPrint = std::pow(10, dX);

				double dYPrint = dY;
				if (m_bLogY)
					dYPrint = std::pow(10, dY);

				if (fm.horizontalAdvance(QString::number(dX)) < m_paintRect.x() + m_paintRect.width() - dPixelX - 5)
					_painter->drawText(dPixelX + 5, m_paintRect.y() + m_paintRect.height() - 5, QString::number(dXPrint));
				else
					_painter->drawText(dPixelX - fm.horizontalAdvance(QString::number(dX)) - 5, m_paintRect.y() + m_paintRect.height() - 5, QString::number(dXPrint));
				if (fm.height() < dPixelY - m_paintRect.y() - 5)
					_painter->drawText(m_paintRect.x() + 5, dPixelY - 5, QString::number(dYPrint));
				else
					_painter->drawText(m_paintRect.x() + 5, dPixelY + fm.height() + 5, QString::number(dYPrint));

				_painter->setPen(oldPen);
			}
		}
	}
}
// Work with curves
unsigned CQtPlot::AddCurve() // add new curve to the plot
{
	QVector<QPointF> testPoints;
	QVector<QPointF> testPlotPoints;

	SCurve* testCurve = new SCurve;
	testCurve->sCurveName = "New Curve " + QString::number(m_vpCurves.size());
	testCurve->rawPoints = testPoints;
	testCurve->points = testPlotPoints;
	testCurve->color = QColor(Qt::black);
	testCurve->nLineWidth = 1;
	testCurve->bVisibility = false;
	testCurve->bLinesVisibility = true;
	testCurve->eXLabelType = LABEL_NONE;
	testCurve->eYLabelType = LABEL_NONE;
	m_vpCurves.append(testCurve);

	m_bFixedBoudaries = false;

	return m_vpCurves.size() - 1;
}

unsigned CQtPlot::AddCurve(SCurve* _curve)
{
	m_vpCurves.append(_curve);
	if ((m_bFixedBoudaries) && ((m_eLastChosenXLabel != _curve->eXLabelType) || (m_eLastChosenYLabel != _curve->eYLabelType)))
		m_bFixedBoudaries = false;
	if (_curve->bVisibility)
	{
		m_eLastChosenXLabel = _curve->eXLabelType;
		m_eLastChosenYLabel = _curve->eYLabelType;
	}
	RecalcBoundaries();

	return m_vpCurves.size() - 1;
}

void CQtPlot::DeleteCurve(unsigned _nCurveIndex)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->sCurveName = "";
		m_vpCurves.at(_nCurveIndex)->rawPoints.clear();
		m_vpCurves.at(_nCurveIndex)->points.clear();
		m_vpCurves.at(_nCurveIndex)->color = QColor(Qt::transparent);
		m_vpCurves.at(_nCurveIndex)->nLineWidth = 0;
		m_vpCurves.at(_nCurveIndex)->bVisibility = false;

		RecalcBoundaries();
	}
}

unsigned CQtPlot::GetCurvesNumber()
{
	return m_vpCurves.size();
}

void CQtPlot::SetCurveData(unsigned _nCurveIndex, const QVector<QPointF>& _pPoints)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->rawPoints = _pPoints;
		SetCurvePlotData();
		RecalcBoundaries();
	}
}

void CQtPlot::SetCurvePlotData()
{
	for (int i = 0; i < m_vpCurves.size(); ++i)
	{
		// rawPoints are saved to points by AddCurve-function -> save to rawPoints
		if (m_vpCurves.at(i)->rawPoints.empty())
			m_vpCurves.at(i)->rawPoints = m_vpCurves.at(i)->points;

		// clear points
		m_vpCurves.at(i)->points.clear();
		for (int j = 0; j < m_vpCurves.at(i)->rawPoints.size(); ++j)
		{
			double dX = m_vpCurves.at(i)->rawPoints.at(j).x();
			double dY = m_vpCurves.at(i)->rawPoints.at(j).y();

			QPointF pointPlot;

			if (!m_bLogX)
				pointPlot.setX(dX);
			else if (dX <= 0.)
				continue;
			else
				pointPlot.setX(std::log10(dX));

			if (!m_bLogY)
				pointPlot.setY(dY);
			else if (dY <= 0.)
				continue;
			else
				pointPlot.setY(std::log10(dY));

			m_vpCurves.at(i)->points.push_back(pointPlot);
		}
	}
}

void CQtPlot::SetCurveName(unsigned _nCurveIndex, QString _sName)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->sCurveName = _sName;
		RedrawPlot();
	}
}

void CQtPlot::SetCurveVisibility(unsigned _nCurveIndex, bool _bIsVisible)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->bVisibility = _bIsVisible;
		if (_bIsVisible)
		{
			m_eLastChosenXLabel = m_vpCurves.at(_nCurveIndex)->eXLabelType;
			m_eLastChosenYLabel = m_vpCurves.at(_nCurveIndex)->eYLabelType;
			ChangeCurvesVisibilityAccordingToLabels();
		}
		RecalcBoundaries();
		RedrawPlot();
	}
}

void CQtPlot::SetCurveColor(unsigned _nCurveIndex, QColor _color)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->color = _color;
		RedrawPlot();
	}
}

void CQtPlot::SetCurveLineWidth(unsigned _nCurveIndex, unsigned _nLineWidth)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->nLineWidth = _nLineWidth;
		RedrawPlot();
	}
}

void CQtPlot::SetCurveLinesVisibility(unsigned _nCurveIndex, bool _bLinesVisible)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->bLinesVisibility = _bLinesVisible;
		RedrawPlot();
	}
}

void CQtPlot::SetCurveAxisLabels(unsigned _nCurveIndex, LabelTypes _eXAxisLabel, LabelTypes _eYAxisLabel)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->eXLabelType = _eXAxisLabel;
		m_vpCurves.at(_nCurveIndex)->eYLabelType = _eYAxisLabel;

		bool bChangeVisibleLabels = true;
		for (int i = 0; i < m_vpCurves.size(); ++i)
			if ((m_vpCurves.at(i)->bVisibility) && (i != _nCurveIndex))
			{
				bChangeVisibleLabels = false;
				break;
			}
		if (bChangeVisibleLabels)
		{
			m_eLastChosenXLabel = _eXAxisLabel;
			m_eLastChosenYLabel = _eYAxisLabel;
		}
		ChangeCurvesVisibilityAccordingToLabels();
		RedrawPlot();
	}
}

SCurve* CQtPlot::GetCurve(unsigned _nCurveIndex)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
		return m_vpCurves.at(_nCurveIndex);
	else
		return NULL;
}

void CQtPlot::AddPoint(unsigned _nCurveIndex, const QPointF& _point)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves.at(_nCurveIndex)->points.append(_point);
		RecalcBoundaries();
		RedrawPlot();
	}
}

void CQtPlot::AddPoints(unsigned _nCurveIndex, const std::vector<double>& _vX, const std::vector<double>& _vY)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		if (_vX.size() != _vY.size()) return;

		for (unsigned i = 0; i < _vX.size(); ++i)
			m_vpCurves.at(_nCurveIndex)->points.append(QPointF(_vX[i], _vY[i]));

		SetCurvePlotData();
		RecalcBoundaries();
		RedrawPlot();
	}
}

void CQtPlot::ClearCurve(unsigned _nCurveIndex)
{
	if ((int)_nCurveIndex < m_vpCurves.size())
	{
		m_vpCurves[_nCurveIndex]->points.clear();
		m_vpCurves[_nCurveIndex]->rawPoints.clear();
		RecalcBoundaries();
		RedrawPlot();
	}
}

void CQtPlot::ClearPlot()
{
	for (int i = 0; i < m_vpCurves.size(); ++i)
		delete m_vpCurves[i];
	m_vpCurves.clear();
	RedrawPlot();
}

void CQtPlot::SetManualLabelsNames(const QString& _sXLabelName, const QString& _sYLabelName)
{
	m_sManualXLabelName = _sXLabelName;
	m_sManualYLabelName = _sYLabelName;
}

// Work with plot boundaries
void CQtPlot::RecalcBoundaries()
{
	if (m_bFixedBoudaries /*&& !m_bLogX && !m_bLogY*/) return;
	double dMaxX = 0;
	double dMinX = 0;
	double dMaxY = 0;
	double dMinY = 0;
	int i = 0;
	while (i < m_vpCurves.size())
	{
		if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
		{
			dMaxX = m_vpCurves.at(i)->points.at(0).x();
			dMinX = m_vpCurves.at(i)->points.at(0).x();
			dMaxY = m_vpCurves.at(i)->points.at(0).y();
			dMinY = m_vpCurves.at(i)->points.at(0).y();
			break;
		}
		i++;
	}
	if (i != m_vpCurves.size())
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if (m_vpCurves.at(i)->bVisibility)
			{
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
				{
					if (m_vpCurves.at(i)->points.at(j).x() > dMaxX)
						dMaxX = m_vpCurves.at(i)->points.at(j).x();
					if (m_vpCurves.at(i)->points.at(j).y() > dMaxY)
						dMaxY = m_vpCurves.at(i)->points.at(j).y();
					if (m_vpCurves.at(i)->points.at(j).x() < dMinX)
						dMinX = m_vpCurves.at(i)->points.at(j).x();
					if (m_vpCurves.at(i)->points.at(j).y() < dMinY)
						dMinY = m_vpCurves.at(i)->points.at(j).y();
				}
			}
		}
	m_dMinX = dMinX;
	m_dMaxX = dMaxX;
	m_dMinY = dMinY;
	m_dMaxY = dMaxY;
	if (m_dMaxY - m_dMinY < ACCURACY)
	{
		m_dMinY -= 0.001;
		m_dMaxY += 0.001;
	}

	// set new limits displayed min/max values
	m_dDispMinX = m_dMinX;
	m_dDispMaxX = m_dMaxX;
	m_dDispMinY = m_dMinY;
	m_dDispMaxY = m_dMaxY;
	// if log scale, display 10^value
	if (m_bLogX)
	{
		m_dDispMinX = std::pow(10, m_dDispMinX);
		m_dDispMaxX = std::pow(10, m_dDispMaxX);
	}
	if (m_bLogY)
	{
		m_dDispMinY = std::pow(10, m_dDispMinY);
		m_dDispMaxY = std::pow(10, m_dDispMaxY);
	}

}

// minimum x-value of plot data (points)
double CQtPlot::CalcPlotXMin()
{
	double dMinX = 1;
	int i = 0;
	while (i < m_vpCurves.size())
	{
		if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
		{
			dMinX = m_vpCurves.at(i)->points.at(0).x();
			break;
		}
		i++;
	}
	if (i != m_vpCurves.size())
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if (m_vpCurves.at(i)->bVisibility)
			{
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
				{
					if (m_vpCurves.at(i)->points.at(j).x() < dMinX)
						dMinX = m_vpCurves.at(i)->points.at(j).x();
				}
			}
		}
	return dMinX;
}

// maximum x-value of plot data (points)
double CQtPlot::CalcPlotXMax()
{
	double dMaxX = 1;
	int i = 0;
	while (i < m_vpCurves.size())
	{
		if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
		{
			dMaxX = m_vpCurves.at(i)->points.at(0).x();
			break;
		}
		i++;
	}
	if (i != m_vpCurves.size())
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if (m_vpCurves.at(i)->bVisibility)
			{
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
				{
					if (m_bLogX && (m_vpCurves.at(i)->points.at(j).x() < 0))
						continue;
					if (m_vpCurves.at(i)->points.at(j).x() > dMaxX)
						dMaxX = m_vpCurves.at(i)->points.at(j).x();
				}
			}
		}
	return dMaxX;
}

// minimum y-value of plot data (points)
double CQtPlot::CalcPlotYMin()
{
	double dMinY = 1;
	int i = 0;
	while (i < m_vpCurves.size())
	{
		if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
		{
			dMinY = m_vpCurves.at(i)->points.at(0).y();
			break;
		}
		i++;
	}
	if (i != m_vpCurves.size())
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if (m_vpCurves.at(i)->bVisibility)
			{
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
				{
					if (m_bLogY && (m_vpCurves.at(i)->points.at(j).y() < 0))
						continue;
					if (m_vpCurves.at(i)->points.at(j).y() < dMinY)
						dMinY = m_vpCurves.at(i)->points.at(j).y();
				}
			}
		}
	return dMinY;
}

// maximum y-value of plot data (points)
double CQtPlot::CalcPlotYMax()
{
	double dMaxY = 1;
	int i = 0;
	while (i < m_vpCurves.size())
	{
		if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
		{
			dMaxY = m_vpCurves.at(i)->points.at(0).y();
			break;
		}
		i++;
	}
	if (i != m_vpCurves.size())
		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if (m_vpCurves.at(i)->bVisibility)
			{
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
				{
					if (m_bLogY && (m_vpCurves.at(i)->points.at(j).y() < 0))
						continue;
					if (m_vpCurves.at(i)->points.at(j).y() > dMaxY)
						dMaxY = m_vpCurves.at(i)->points.at(j).y();
				}
			}
		}
	return dMaxY;
}

void CQtPlot::SetLimits(double _dMinX, double _dMaxX, double _dMinY, double _dMaxY)
{
	m_dMinX = _dMinX;
	m_dMaxX = _dMaxX;
	m_dMinY = _dMinY;
	m_dMaxY = _dMaxY;
	RedrawPlot();
}

void CQtPlot::UpdateLimits()
{
	// transfer display minimum and maximum to internal values
	// in case of logarithmic scaling check if values are positive, if not recalculate new value (min/max) and reset display value

	// x-axis
	if (m_bLogX) // logarithmic scale
	{
		if (m_dDispMinX <= 0)
		{
			m_dMinX = CalcPlotXMin();
			m_dDispMinX = std::pow(10, m_dMinX);
		}
		else
		{
			m_dMinX = std::log10(m_dDispMinX);
		}
		if (m_dDispMaxX <= 0)
		{
			m_dMaxX = CalcPlotXMax();
			m_dDispMaxX = std::pow(10, m_dMaxX);
		}
		else
		{
			m_dMaxX = std::log10(m_dDispMaxX);
		}
	}
	else // non-logarithmic scale
	{
		m_dMinX = m_dDispMinX;
		m_dMaxX = m_dDispMaxX;
	}

	// y-axis
	if (m_bLogY) // logarithmic scale
	{
		if (m_dDispMinY <= 0)
		{
			m_dMinY = CalcPlotYMin();
			m_dDispMinY = std::pow(10, m_dMinY);
		}
		else
		{
			m_dMinY = std::log10(m_dDispMinY);
		}

		if (m_dDispMaxY <= 0)
		{
			m_dMaxY = CalcPlotYMax();
			m_dDispMaxY = std::pow(10, m_dMaxY);
		}
		else
		{
			m_dMaxY = std::log10(m_dDispMaxY);
		}
	}
	else // non-logarithmic scale
	{
		m_dMinY = m_dDispMinY;
		m_dMaxY = m_dDispMaxY;
	}

}

void CQtPlot::SetLogScaling(bool _bLogX, bool _bLogY)
{
	m_bLogX = _bLogX;
	m_bLogY = _bLogY;
	RedrawPlot();
}

void CQtPlot::SetMaxX(double _dMaxX, bool bRedrawPlot)
{
	m_dMaxX = _dMaxX;
	if (bRedrawPlot)
		RedrawPlot();
}
void CQtPlot::SetMinX(double _dMinX, bool bRedrawPlot)
{
	m_dMinX = _dMinX;
	if (bRedrawPlot)
		RedrawPlot();
}
void CQtPlot::SetMaxY(double _dMaxY, bool bRedrawPlot)
{
	m_dMaxY = _dMaxY;
	if (bRedrawPlot)
		RedrawPlot();
}
void CQtPlot::SetMinY(double _dMinY, bool bRedrawPlot)
{
	m_dMinY = _dMinY;
	if (bRedrawPlot)
		RedrawPlot();
}
double CQtPlot::GetMaxX()
{
	return m_dMaxX;
}
double CQtPlot::GetMinX()
{
	return m_dMinX;
}
double CQtPlot::GetMaxY()
{
	return m_dMaxY;
}
double CQtPlot::GetMinY()
{
	return m_dMinY;
}

void CQtPlot::SetNumXGridLines(unsigned _nNumXGridLines)
{
	if (_nNumXGridLines > 0)
		m_nNumXGridLines = _nNumXGridLines;
}
unsigned CQtPlot::GetNumXGridLines()
{
	return m_nNumXGridLines;
}
void CQtPlot::SetNumYGridLines(unsigned _nNumYGridLines)
{
	if (_nNumYGridLines > 0)
		m_nNumYGridLines = _nNumYGridLines;
}
unsigned CQtPlot::GetNumYGridLines()
{
	return m_nNumYGridLines;
}
void CQtPlot::AutoCentrateView()
{
	m_bFixedBoudaries = false;
	RecalcBoundaries();
	RedrawPlot();
}

void CQtPlot::SetNewBoundaries(QRect* _rect)
{
	if ((_rect->width() > 10) || (_rect->height() > 10))
	{
		double newMaxX = GetXCoord(_rect->x() + _rect->width());
		double newMinX = GetXCoord(_rect->x());
		double newMaxY = GetYCoord(_rect->y());
		double newMinY = GetYCoord(_rect->y() + _rect->height());
		SetLimits(newMinX, newMaxX, newMinY, newMaxY);
	}
}

// Plot settings

void CQtPlot::SetPlotTitle(const QString& _sPlotTitle)
{
	m_pMainTitle->setText(_sPlotTitle);
	if (_sPlotTitle.isEmpty())
		m_nPlotAreaOffsetTop = m_cPlotAreaOffset;
	else
		m_nPlotAreaOffsetTop = m_cPlotAreaOffsetTop;
}
QString CQtPlot::GetPlotTitle()
{
	return m_pMainTitle->text();
}

QString CQtPlot::GetXLabel()
{
	if (m_eLastChosenXLabel >= LABEL_LAST)
		return m_csaLabels[0];
	else if (m_eLastChosenXLabel == LABEL_MANUAL)
		return m_sManualXLabelName;
	else
		return m_csaLabels[m_eLastChosenXLabel];
}

QString CQtPlot::GetYLabel()
{
	if (m_eLastChosenYLabel >= LABEL_LAST)
		return m_csaLabels[0];
	else if (m_eLastChosenYLabel == LABEL_MANUAL)
		return m_sManualYLabelName;
	else
		return m_csaLabels[m_eLastChosenYLabel];
}

void CQtPlot::SetXLabelsInterval(unsigned _nXLabelsInterval)
{
	if (m_nXLabelsInterval > 0)
		m_nXLabelsInterval = _nXLabelsInterval;
}
unsigned CQtPlot::GetXLabelsInterval()
{
	return m_nXLabelsInterval;
}
void CQtPlot::SetYLabelsInterval(unsigned _nYLabelsInterval)
{
	if (m_nYLabelsInterval > 0)
		m_nYLabelsInterval = _nYLabelsInterval;
}
unsigned CQtPlot::GetYLabelsInterval()
{
	return m_nYLabelsInterval;
}

void CQtPlot::SaveAsPicture(const QString& _sFileName)
{
	QImage image(this->size(), QImage::Format_ARGB32);
	QPainter painter(&image);
	this->render(&painter);
	image.save(_sFileName, 0, 100);
}

void CQtPlot::SaveAsCSVFile(const QString& _sFileName)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	using Qt::endl;
#endif
	QFile file(_sFileName);
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream textStream(&file);

		for (int i = 0; i < m_vpCurves.size(); ++i)
		{
			if ((m_vpCurves.at(i)->bVisibility) && (m_vpCurves.at(i)->points.size() > 0))
			{
				textStream << m_vpCurves.at(i)->sCurveName << endl;
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
					textStream << m_vpCurves.at(i)->points.at(j).x() << ", ";
				textStream << endl;
				for (int j = 0; j < m_vpCurves.at(i)->points.size(); ++j)
					textStream << m_vpCurves.at(i)->points.at(j).y() << ", ";
				textStream << endl;
			}
		}
	}
}
void CQtPlot::ZoomIn()
{
	double dNewMinX = m_dMinX + m_dSpanX() / 4;
	double dNewMaxX = m_dMaxX - m_dSpanX() / 4;
	double dNewMinY = m_dMinY + m_dSpanY() / 4;
	double dNewMaxY = m_dMaxY - m_dSpanY() / 4;
	if (std::fabs(dNewMinX - dNewMaxX) > ZOOM_IN_LIMIT)
	{
		SetMaxX(dNewMaxX, false);
		SetMinX(dNewMinX, true);
	}
	if (std::fabs(dNewMinY - dNewMaxY) > ZOOM_IN_LIMIT)
	{
		SetMaxY(dNewMaxY, false);
		SetMinY(dNewMinY, true);
	}
}
void CQtPlot::ZoomOut()
{
	double dNewMinX = m_dMinX - m_dSpanX() / 2;
	double dNewMaxX = m_dMaxX + m_dSpanX() / 2;
	double dNewMinY = m_dMinY - m_dSpanY() / 2;
	double dNewMaxY = m_dMaxY + m_dSpanY() / 2;
	if (std::fabs(dNewMinX - dNewMaxX) < ZOOM_OUT_LIMIT)
	{
		SetMaxX(dNewMaxX, false);
		SetMinX(dNewMinX, true);
	}
	if (std::fabs(m_dMinY - m_dMaxY) < ZOOM_OUT_LIMIT)
	{
		SetMaxY(dNewMaxY, false);
		SetMinY(dNewMinY, true);
	}
}

void CQtPlot::SetGridVisibility(bool _bIsGridVisible)
{
	m_bIsGridVisible = _bIsGridVisible;
	RedrawPlot();
}
bool CQtPlot::GetGridVisibility()
{
	return m_bIsGridVisible;
}

void CQtPlot::SetLegendPosition(int8_t _position)
{
	m_nLegendPosition = _position;
	RedrawPlot();
}
int8_t CQtPlot::GetLegendPosition()
{
	return m_nLegendPosition;
}

void CQtPlot::SetAxisLablesVisible(bool _bIsVisible)
{
	m_bIsAxisLablesVisible = _bIsVisible;
	RedrawPlot();
}
bool CQtPlot::GetAxisLablesVisible()
{
	return m_bIsAxisLablesVisible;
}
void CQtPlot::ChangeCurvesVisibilityAccordingToLabels()
{
	for (int i = 0; i < m_vpCurves.size(); ++i)
		if ((m_vpCurves.at(i)->bVisibility) && ((m_vpCurves.at(i)->eXLabelType != m_eLastChosenXLabel) || (m_vpCurves.at(i)->eYLabelType != m_eLastChosenYLabel)))
			m_vpCurves.at(i)->bVisibility = false;
}

void CQtPlot::DetermineNearestCurve()
{
	m_csNearestCurveIndex = -1;
	bool bStop = false;
	for (int i = 0; i < m_vpCurves.size(); ++i)
	{
		// look only for visible curves
		if (m_vpCurves.at(i)->bVisibility)
		{
			// get coordinates of m_markPoint
			double dX = GetXCoord(m_markPoint.x());
			// if m_markPoint crosses the curve
			if ((m_vpCurves.at(i)->points.size() < 2) || (dX < m_vpCurves.at(i)->points.at(0).x()) || (dX > m_vpCurves.at(i)->points.last().x()))
				continue;
			for (int j = 0; j < m_vpCurves.at(i)->points.size() - 1; ++j)
			{
				// between which points of curve the m_markPoint situated
				if ((m_vpCurves.at(i)->points.at(j).x() <= dX) && (m_vpCurves.at(i)->points.at(j + 1).x() >= dX))
				{
					double dPTLD = GetPointToLineDistance(m_markPoint, GetAbsCoord(m_vpCurves.at(i)->points.at(j)), GetAbsCoord(m_vpCurves.at(i)->points.at(j + 1)));
					if (dPTLD < MOUSE_PX_PRECISION + m_vpCurves.at(i)->nLineWidth / 2)
					{
						if (m_csNearestCurveIndex == -1)
							m_csNearestCurveIndex = i;
						else
						{
							m_csNearestCurveIndex = -1;
							bStop = true;
							break;
						}
					}
				}
			}
			if (bStop)
				break;
		}
	}
}

double CQtPlot::GetXCoord(double _dAbsXCoord)
{
	return ((_dAbsXCoord - m_paintRect.left()) * m_dSpanX()) / (m_paintRect.width() - 1) + m_dMinX;
}

double CQtPlot::GetYCoord(double _dAbsYCoord)
{
	return m_dMinY - ((_dAbsYCoord - m_paintRect.bottom()) * m_dSpanY()) / (m_paintRect.height() - 1);
}

double CQtPlot::GetAbsXCoord(double _dXCoord)
{
	return m_paintRect.left() + ((_dXCoord - m_dMinX) * (m_paintRect.width() - 1) / (m_dSpanX() != 0.0 ? m_dSpanX() : 0.001));
}

double CQtPlot::GetAbsYCoord(double _dYCoord)
{
	return m_paintRect.bottom() - ((_dYCoord - m_dMinY) * (m_paintRect.height() - 1) / (m_dSpanY() != 0.0 ? m_dSpanY() : 0.001));
}

QPointF CQtPlot::GetCoord(QPointF _dAbsCoord)
{
	return QPointF(GetXCoord(_dAbsCoord.x()), GetYCoord(_dAbsCoord.y()));
}

QPointF CQtPlot::GetAbsCoord(QPointF _dCoord)
{
	return QPointF(GetAbsXCoord(_dCoord.x()), GetAbsYCoord(_dCoord.y()));
}

double CQtPlot::GetPointToLineDistance(QPointF _dPoint, QPointF _dLine1, QPointF _dLine2)
{
	return std::fabs((_dLine1.y() - _dLine2.y()) * _dPoint.x() + (_dLine2.x() - _dLine1.x()) * _dPoint.y() + (_dLine1.x() * _dLine2.y() - _dLine2.x() * _dLine1.y())) /
	       (std::sqrt((_dLine2.x() - _dLine1.x()) * (_dLine2.x() - _dLine1.x()) + (_dLine2.y() - _dLine1.y()) * (_dLine2.y() - _dLine1.y())));
}

// Private slots
void CQtPlot::SetCurveNameSlot(int _nCurveIndex)
{
	if (_nCurveIndex < m_vpCurves.size())
	{
		bool bOk;
		QString sText = QInputDialog::getText(this, m_csDialogSetNameCaption, m_csDialogSetNameLable, QLineEdit::Normal, m_vpCurves.at(_nCurveIndex)->sCurveName, &bOk);
		if (bOk && !sText.isEmpty())
			SetCurveName(_nCurveIndex, sText);
	}
}

void CQtPlot::ToggleCurveVisibilitySlot(int _nCurveIndex)
{
	if (_nCurveIndex < m_vpCurves.size())
		SetCurveVisibility(_nCurveIndex, !m_vpCurves.at(_nCurveIndex)->bVisibility);
}

void CQtPlot::SetCurveColorSlot(int _nCurveIndex)
{
	if (_nCurveIndex < m_vpCurves.size())
	{
		QColor color;
		color = QColorDialog::getColor(m_vpCurves.at(_nCurveIndex)->color, this, m_csDialogSetColorCaption);
		if (color.isValid())
			SetCurveColor(_nCurveIndex, color);
	}
}

void CQtPlot::SetCurveWidthSlot(int _nCurveIndex)
{
	if (_nCurveIndex < m_vpCurves.size())
	{
		bool bOk;
		int nWidth = QInputDialog::getInt(this, m_csDialogSetWidthCaption, m_csDialogSetWidthLable, m_vpCurves.at(_nCurveIndex)->nLineWidth, 1, 50, 1, &bOk);
		if (bOk)
			SetCurveLineWidth(_nCurveIndex, nWidth);
	}
}

void CQtPlot::ToggleCurveLinesVisibilitySlot(int _nCurveIndex)
{
	if (_nCurveIndex < m_vpCurves.size())
		SetCurveLinesVisibility(_nCurveIndex, !m_vpCurves.at(_nCurveIndex)->bLinesVisibility);
}

void CQtPlot::AutoZoomSlot()
{
	AutoCentrateView();
}

void CQtPlot::SaveAsPictureSlot()
{
	QString sFileName = QFileDialog::getSaveFileName(this, m_csDialogSavePictureCaption, m_sDialogSavePicturePath, m_csDialogSavePictureFilter);
	SaveAsPicture(sFileName);
	m_sDialogSavePicturePath = sFileName;
}

void CQtPlot::SaveAsCSVFileSlot()
{
	QString sFileName = QFileDialog::getSaveFileName(this, m_csDialogSaveCSVFileCaption, m_sDialogSaveCSVFilePath, m_csDialogSaveCSVFileFilter);
	SaveAsCSVFile(sFileName);
	m_sDialogSaveCSVFilePath = sFileName;
}

void CQtPlot::ToggleGridVisibilitySlot()
{
	SetGridVisibility(!GetGridVisibility());
}

void CQtPlot::SetLimitsSlot()
{
	m_pLimitsDialog->SetLabels("X: (" + GetXLabel() + ")", "Y: (" + GetYLabel() + ")");
	m_pLimitsDialog->SetLogScaling(m_bLogX, m_bLogY);
	m_pLimitsDialog->SetLimits(m_dDispMinX, m_dDispMaxX, m_dDispMinY, m_dDispMaxY);

	QObject::connect(m_pLimitsDialog, SIGNAL(ApplyPressed()), this, SLOT(GridLimitsAcceptedSlot()));
	QObject::connect(m_pLimitsDialog, SIGNAL(OkPressed()), this, SLOT(GridLimitsAcceptedSlot()));

	m_pLimitsDialog->raise();
	m_pLimitsDialog->show();
}

void CQtPlot::SetLegendPositionSlot(int _nPosition)
{
	SetLegendPosition(_nPosition);
}

void QtPlot::CQtPlot::GridLimitsAcceptedSlot()
{
	// Set points (plot data) from rawPoints (raw data)
	SetCurvePlotData();
	// Check the limits that were set in the UI. For log scaling no negative values are allowed
	UpdateLimits();
	// Set the (adjusted) display limits to the UI.
	m_pLimitsDialog->SetLimits(m_dDispMinX, m_dDispMaxX, m_dDispMinY, m_dDispMaxY);

	m_bFixedBoudaries = true;
	RedrawPlot();
}
