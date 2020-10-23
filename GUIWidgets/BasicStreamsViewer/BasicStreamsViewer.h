/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_BasicStreamsViewer.h"
#include "DyssolDefines.h"

#define PLOT_LINE_WIDTH	3

class CMaterialsDatabase;
class CFlowsheet;
class CBaseStream;
class CTimeDependentValue;
class CMDMatrix;

class CBasicStreamsViewer : public QWidget
{
	Q_OBJECT

private:
	enum class EPropertyType : int { Mass, Temperatue, Pressure, PhaseFraction, Phase1, Phase2, Phase3, Phase4, SolidDistr, SauterDiameter };
	enum class EDimType : int { Row, Col };
	enum class EDistrCombination : int { Empty, Compounds, TwoDimensional, OneDimensionalVertical, OneDimensionalHorizontal };
	enum class ETabType : int { Table, Plot };

	CFlowsheet* m_pFlowsheet;	/// Pointer to the flowsheet.
	CMaterialsDatabase* m_materialsDB;	// Pointer to materials database.

	std::vector<const CBaseStream*> m_vSelectedStreams;	/// Currently selected streams.
	std::vector<const CTimeDependentValue*> m_vSelected2D;	/// Currently selected 2D distributions from all selected streams.
	std::vector<const CMDMatrix*> m_vSelectedMD;		/// Currently selected MD distributions from all selected streams.
	std::vector<double> m_vSelectedTP;					/// Union of time points in all selected streams.

	double m_dCurrentTime;								/// Currently chosen time point.

public:
	CBasicStreamsViewer(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* parent = nullptr);

	void InitializeConnections() const;

	/// Sets list of streams to visualize.
	void SetStreams(const std::vector<const CBaseStream*>& _vStreams);

public slots:
	void UpdateWholeView();
	void setVisible(bool _bVisible) override;

private:
	Ui::CBasicStreamsViewer ui;

	/// Creates combo boxes for properties, dimensions and compounds.
	void SetupComboBoxes() const;
	/// Creates main combo box with the list of stream properties.
	void SetupComboBoxProperties() const;
	/// Creates combo box for a dimension, and selects the item with _defaultPos if nothing has been selected yet.
	void SetupComboBoxDims(QComboBox* _combo, int _defaultPos) const;
	/// Creates combo box for compounds.
	void SetupComboBoxCompounds() const;
	/// Creates combo box with PSD types.
	void SetupComboBoxPSDType() const;
	/// Creates combo box with PSD grid types.
	void SetupComboBoxPSDGrid() const;
	/// Creates time slider.
	void SetupTimeSlider() const;

	/// Updates the list of all time points m_vSelectedTP according to selected streams.
	void GetSelectedTimePoints();
	/// Updates the lists of all distributions m_vSelected2D and m_vSelectedMD according to selected streams.
	void GetSelectedDistributions();

	/// Sets position of time slider according to the current time m_dCurrentTime.
	void UpdateSliderPosition();
	/// Sets text to the time editor according to the current time m_dCurrentTime.
	void UpdateTimeLabel() const;
	/// Sets number of time points to the info label.
	void UpdateTPNumberLabel() const;
	/// Update main tab.
	void UpdateTabView();
	/// Update table view.
	void UpdateTableTab();
	/// Update plot view.
	void UpdatePlotTab();
	/// Sets enabled all controls according to the selection of streams.
	void UpdateWidgetsEnable() const;
	/// Sets visible all controls used for multidimensional stuff.
	void UpdateWidgetsVisible() const;

	/// Returns currently chosen property.
	EPropertyType ChosenProperty() const;
	/// Returns currently chosen distribution type for rows or columns.
	EDistrTypes ChosenDim(EDimType _dim) const;
	/// Returns currently chosen PSD type.
	EPSDTypes ChosenPSDType() const;
	/// Returns currently chosen PSD grid type.
	EPSDGridType ChosenPSDGridType() const;
	/// Returns the key of currently chosen compound.
	std::string ChosenCompound() const;
	/// Returns currently chosen tab.
	ETabType ChosenTab() const;
	/// Returns headers for the current table view according to provided distribution type _distr.
	std::vector<std::string> TableHeaders(EDistrTypes _distr) const;
	/// Return the name of the chosen PSD type if _distr is chosen.
	std::string PSDSymbolicName(EDistrTypes _distr) const;
	/// Return the name of the chosen distribution type _distr.
	QString DistrSymbolicName(EDistrTypes _distr) const;

	/// Sets selected mass/temperature/pressure to the table.
	void SetMTPToTable(int _type);
	/// Sets selected phase fractions to the table.
	void SetPhaseFractionsToTable();
	/// Sets selected compounds distribution to the table.
	void SetPhaseCompoundsToTable();
	/// Sets Sauter diameter of the selected distribution to the table.
	void SetSauterDiameterToTable();
	/// Sets selected distribution to the table.
	void SetSolidDistrsToTable();
	/// Configures the table according to the selected combination of settings _type.
	void SetSolidDistrsToTableHeaders(EDistrCombination _type);
	/// Sets data to the table according to the selected combination of settings _type.
	void SetSolidDistrsToTableData(EDistrCombination _type);

	/// Sets selected mass/temperature/pressure to the plot.
	void SetMTPToPlot(int _type);
	/// Sets selected phase fractions to the plot.
	void SetPhaseFractionsToPlot();
	/// Sets selected compounds distribution to the plot.
	void SetPhaseCompoundsToPlot();
	/// Sets Sauter diameter of the selected distribution to the plot.
	void SetSauterDiameterToPlot();
	/// Sets selected distribution to the plot.
	void SetSolidDistrsToPlot();

	/// Restores previous selected position if possible.
	static void RestorePosition(QComboBox* _combo, int _position, int _defaultPosition = 0);

	/// Exports selected data to a csv-file.
	void ExportToFile();
	/// Create and show context menu on right-click.
	void contextMenuEvent(QContextMenuEvent* _event) override;

private slots:
	void PropertyChanged();
	void SliderMoved();
	void TimeEdited();
	void PrevTimeClicked();
	void NextTimeClicked();
	void ComboRowsChanged();
	void ComboColsChanged();
	void ComboCompoundsChanged();
	void ComboPSDTypeChanged();
	void ComboPSDGridTypeChanged();
	void TabChanged();
};
