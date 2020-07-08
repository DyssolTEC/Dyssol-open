/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "GridLimitsDialog.h"
#include <QRubberBand>

namespace QtPlot
{
#define MAX_X	0
#define MIN_X	1
#define MAX_Y	2
#define MIN_Y	3

#define POSITION_NONE			0
#define POSITION_TOP_RIGHT		1
#define POSITION_TOP_LEFT		2
#define POSITION_BOTTOM_LEFT	3
#define POSITION_BOTTOM_RIGHT	4

#define ZOOM_OUT_LIMIT 1e+15
#define ZOOM_IN_LIMIT 1e-15

#define MOUSE_PX_PRECISION 5

#define ACCURACY 1e-15

enum LabelTypes
{
	LABEL_NONE,
	LABEL_MANUAL,
	LABEL_TIME,
	LABEL_FORCE,
	LABEL_DISPLACEMENT,
	LABEL_SAUTER,
	LABEL_NUMBER,
	LABEL_MASS_FLOW,
	LABEL_MASS,
	LABEL_TEMPERATURE,
	LABEL_PRESSURE,
	LABEL_MASS_FRACTION,
	LABEL_SIZE,
	LABEL_q3,
	LABEL_VALUE,
	LABEL_LAST
};

struct SCurve
{
	QString sCurveName;
	QVector<QPointF> points;
	QVector<QPointF> rawPoints;
	QColor color;
	unsigned nLineWidth;
	bool bVisibility;
	bool bLinesVisibility;
	LabelTypes eXLabelType;
	LabelTypes eYLabelType;
	SCurve()
	{
		sCurveName = "New curve";
		nLineWidth = 3;
		bVisibility = true;
		bLinesVisibility = true;
		eXLabelType = eYLabelType = LABEL_NONE;
		color = Qt::blue;
	}
	SCurve(const QString& _sCurveName, const QColor& _color, unsigned _nLineWidth,
	       bool _bVisibility, bool _bLinesVisibility, LabelTypes _eXLabelType, LabelTypes _eYLabelType)
	{
		sCurveName = _sCurveName;
		nLineWidth = _nLineWidth;
		bVisibility = _bVisibility;
		bLinesVisibility = _bLinesVisibility;
		eXLabelType = _eXLabelType;
		eYLabelType = _eYLabelType;
		color = _color;
	}
	SCurve(const std::string& _sCurveName, const QColor& _color, LabelTypes _eXLabelType, LabelTypes _eYLabelType)
	{
		sCurveName = QString::fromStdString(_sCurveName);
		nLineWidth = 3;
		bVisibility = true;
		bLinesVisibility = true;
		eXLabelType = _eXLabelType;
		eYLabelType = _eYLabelType;
		color = _color;
	}
};

class CQtPlot : public QWidget
{
	Q_OBJECT
private:

	enum MouseMode
	{
		None, Move, Zoom, Mark
	};

	static const unsigned m_cPlotAreaOffset = 5;
	static const unsigned m_cPlotAreaOffsetTop = 20;
	static const unsigned m_cCoordinateOffsetTop = 10;
	static const unsigned m_cCoordinateOffsetBottom = 30;
	static const unsigned m_cCoordinateOffsetLeft = 75;
	static const unsigned m_cCoordinateOffsetRight = 20;
	static const unsigned m_cCoordinateRectPenWidth = 2;

	unsigned m_nPlotAreaOffsetTop;

	static const QString m_csMenuCurves;
	static const QString m_csMenuCurveName;
	static const QString m_csMenuCurveVisibility;
	static const QString m_csMenuCurveColor;
	static const QString m_csMenuCurveWidth;
	static const QString m_csMenuCurveLines;
	static const QString m_csMenuAutoZoom;
	static const QString m_csMenuSaveAsPicture;
	static const QString m_csMenuSaveAsCSVFile;
	static const QString m_csMenuGridVisibility;
	static const QString m_csMenuGridLimits;
	static const QString m_csMenuGridScaling;
	static const QString m_csMenuLegengPosition;
	static const QString m_csMenuLegengPositionNone;
	static const QString m_csMenuLegengPositionTR;
	static const QString m_csMenuLegengPositionTL;
	static const QString m_csMenuLegengPositionBL;
	static const QString m_csMenuLegengPositionBR;
	static const QString m_csDialogSavePictureCaption;
	static const QString m_csDialogSavePictureFilter;
	static const QString m_csDialogSaveCSVFileCaption;
	static const QString m_csDialogSaveCSVFileFilter;
	static const QString m_csDialogSetWidthCaption;
	static const QString m_csDialogSetWidthLable;
	static const QString m_csDialogSetColorCaption;
	static const QString m_csDialogSetNameCaption;
	static const QString m_csDialogSetNameLable;
	static const QString m_csDialogSetLimitCaption;
	static const QString m_csDialogSetLimitLable;
	static const QString m_csaLabels[];

	QVector<SCurve*> m_vpCurves;		// pointers to the curves which are stored in the class
	QPainter* m_pPainter;
	QLabel* m_pMainTitle;			// plot title
	QRect m_paintRect;				// plot area rectangle
	QRubberBand* m_pRubberBand;
	QPoint m_previousMousePos;
	bool m_bIsGridVisible;
	bool m_bIsAxisLablesVisible;
	__int8 m_nLegendPosition;
	LabelTypes m_eLastChosenXLabel;
	LabelTypes m_eLastChosenYLabel;
	MouseMode m_eLeftMouseMode;
	QPointF m_markPoint;
	int m_csNearestCurveIndex;
	QString m_sDialogSavePicturePath;
	QString m_sDialogSaveCSVFilePath;

	double m_dSpanX() const { return m_dMaxX - m_dMinX; }
	double m_dSpanY() const { return m_dMaxY - m_dMinY; }
	double m_dMinX;					// minimal x value
	double m_dMaxX;					// maximal x value
	double m_dDispMinX;				// minimal x value
	double m_dDispMaxX;				// maximal x value
	bool m_bLogX;					// flag variable for plotting x-axis in log-scale
	unsigned m_nXLabelsInterval;	// interval between labels on marks on x-axis
	unsigned m_nNumXGridLines;		// number of grid lines on x-axis
	double m_dMinY;					// minimal y value
	double m_dMaxY;					// maximal y value
	double m_dDispMinY;				// minimal y value
	double m_dDispMaxY;				// maximal y value
	bool m_bLogY;					// flag variable for plotting y-axis in log-scale
	unsigned m_nYLabelsInterval;	// interval between labels on marks on y-axis
	unsigned m_nNumYGridLines;		// number of grid lines on x-axis
	bool m_bFixedBoudaries;			// plot limits are not changes in RecalcBoundaries() according to the curves on the plot

	CGridLimitsDialog* m_pLimitsDialog;

	QString m_sManualXLabelName;
	QString m_sManualYLabelName;

public:
	CQtPlot(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~CQtPlot();

	unsigned AddCurve();														// add new empty curve to the plot
	unsigned AddCurve(SCurve* _curve);											// add new curve to the plot
	unsigned GetCurvesNumber();													// returns current number of curves in plot
	void DeleteCurve(unsigned _nCurveIndex);									// deletes specified curve
	void SetCurveData(unsigned _nCurveIndex, const QVector<QPointF>& _pPoints);	// set new raw data
	void SetCurvePlotData();													// set new plotting data
	void SetCurveName(unsigned _nCurveIndex, QString _sName);					// sets the name of the specified curve
	void SetCurveVisibility(unsigned _nCurveIndex, bool _bVisible);				// change visibility of specified curve
	void SetCurveColor(unsigned _nCurveIndex, QColor _color);					// sets color of the specified curve
	void SetCurveLineWidth(unsigned _nCurveIndex, unsigned _nLineWidth);		// change width of specified curve
	void SetCurveLinesVisibility(unsigned _nCurveIndex, bool _bLinesVisible);	// set visibility of lines in curves
	void SetCurveAxisLabels(unsigned _nCurveIndex, LabelTypes _eXAxisLabel, LabelTypes _eYAxisLabel);   // labels of x and y axis
	SCurve* GetCurve(unsigned _nCurveIndex);									// returns pointer to the curve with specified index
	void AddPoint(unsigned _nCurveIndex, const QPointF& _point);				// adds new point to the specified curve
	void AddPoints(unsigned _nCurveIndex, const std::vector<double>& _vX, const std::vector<double>& _vY);   // adds new points to the specified curve
	void ClearCurve(unsigned _nCurveIndex);										// clear all time points from the specified curve
	void ClearPlot();															// deletes all curves from the plot

	void SetManualLabelsNames(const QString& _sXLabelName, const QString& _sYLabelName);   // manual names for labels of x and y axis, will be used if LABEL_MANUAL is specified (independently for x and y axis)
	void SetLimits(double _dMinX, double _dMaxX, double _dMinY, double _dMaxY);    // set limitation of all axis
	void UpdateLimits();    // update limitation of all axis
	void SetLogScaling(bool _bLogX, bool _bLogY);    // set scaling of all axis
	void SetMaxX(double _dMaxX, bool bRedrawPlot = true);   // set limitation of the x axis
	double GetMaxX();										// returns value of the maximal x value
	void SetMinX(double _dMinX, bool bRedrawPlot = true);   // set limitation of the x axis
	double GetMinX();										// returns value of the minimal x value
	void SetMaxY(double _dMaxY, bool bRedrawPlot = true);   // set limitation of the y axis
	double GetMaxY();										// returns value of the maximal y value
	void SetMinY(double _dMinY, bool bRedrawPlot = true);   // set limitation of the y axis
	double GetMinY();										// returns value of the minimal y value
	void SetNumXGridLines(unsigned _nNumXGridLines);		// set number of grid lines on x-axis
	unsigned GetNumXGridLines();							// returns number of grid lines on x-axis
	void SetNumYGridLines(unsigned _nNumYGridLines);		// set number of grid lines on y-axis
	unsigned GetNumYGridLines();							// returns number of grid lines on y-axis
	void SetXLabelsInterval(unsigned _nXLabelsInterval);	// sets interval with which the axis subscriptions will be shown
	unsigned GetXLabelsInterval();
	void SetYLabelsInterval(unsigned _nYLabelsInterval);	// sets interval with which the axis subscriptions will be shown
	unsigned GetYLabelsInterval();

	void AutoCentrateView();						// auto-scale + auto centrate
	void SetPlotTitle(const QString& _sPlotTitle);	// sets the title of the whole plot
	QString GetPlotTitle();							// get plot title
	QString GetXLabel();
	QString GetYLabel();
	void SaveAsPicture(const QString& _sFileName);	// save actual view as picture
	void SaveAsCSVFile(const QString& _sFileName);	// save visible curves to CSV-file
	void SetGridVisibility(bool _bIsGridVisible);   // set visibility of the grid
	bool GetGridVisibility();						// returns current visibility of the grid
	void SetLegendPosition(__int8 _position);
	__int8 GetLegendPosition();
	void SetAxisLablesVisible(bool _bIsVisible);
	bool GetAxisLablesVisible();


private:
	void CreateDropMenu(QMenu* _pMenu);
	void CreateFullDropMenu(QMenu* _pMenu);
	void CreateCurveDropMenu(QMenu* _pMenu);
	void DrawGrid(QPainter* _painter);
	void DrawCurves(QPainter* _painter);
	void DrawAxisLabels(QPainter* _painter);
	void DrawLegend(QPainter* _painter);
	void DrawMarks(QPainter* _painter);
	void RecalcBoundaries();
	double CalcPlotXMin();
	double CalcPlotXMax();
	double CalcPlotYMin();
	double CalcPlotYMax();
	void ZoomIn();
	void ZoomOut();
	void ChangeCurvesVisibilityAccordingToLabels();
	void SetNewBoundaries(QRect* _rect);
	void DetermineNearestCurve();
	double GetXCoord(double _dAbsXCoord);
	double GetYCoord(double _dAbsYCoord);
	double GetAbsXCoord(double _dXCoord);
	double GetAbsYCoord(double _dYCoord);
	QPointF GetCoord(QPointF _dAbsCoord);
	QPointF GetAbsCoord(QPointF _dCoord);
	double GetPointToLineDistance(QPointF _dPoint, QPointF _dLine1, QPointF _dLine2);


protected:
	virtual void resizeEvent(QResizeEvent* _resizeEvent);
	virtual void paintEvent(QPaintEvent* _paintEvent);
	virtual void wheelEvent(QWheelEvent* _wheelEvent);
	virtual void mousePressEvent(QMouseEvent* _mouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent* _mouseEvent);
	virtual void mouseMoveEvent(QMouseEvent* _mouseEvent);
	virtual void mouseDoubleClickEvent(QMouseEvent* _mouseEvent);
	virtual void contextMenuEvent(QContextMenuEvent* _contextMenuEvent);


public slots:
	void RedrawPlot(); // redraw QTPlot
	void changeEvent(QEvent *_event) override;

private slots:
	void SetCurveNameSlot(int _nCurveIndex);
	void ToggleCurveVisibilitySlot(int _nCurveIndex);
	void SetCurveColorSlot(int _nCurveIndex);
	void SetCurveWidthSlot(int _nCurveIndex);
	void ToggleCurveLinesVisibilitySlot(int _nCurveIndex);
	void AutoZoomSlot();
	void SaveAsPictureSlot();
	void SaveAsCSVFileSlot();
	void ToggleGridVisibilitySlot();
	void SetLimitsSlot();
	void SetLegendPositionSlot(int _nPosition);
	void GridLimitsAcceptedSlot();
};
};
