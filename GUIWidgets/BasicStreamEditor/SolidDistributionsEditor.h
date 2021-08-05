/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_SolidDistributionsEditor.h"
#include "DistrFunctionDialog.h"

class CMaterialsDatabase;
class CFlowsheet;
class CBaseStream;
class CMDMatrix;
class CMultidimensionalGrid;

class CSolidDistributionsEditor : public QWidget
{
	Q_OBJECT
public:
	Ui::SolidDistributionsEditor ui;

private:
	CMaterialsDatabase* m_materialsDB;
	CFlowsheet* m_pFlowsheet;
	const CMultidimensionalGrid* m_pGrid;
	CMDMatrix* m_pDistribution;
	CBaseStream* m_pStream;

	std::vector<QComboBox*> m_vCombos;
	std::vector<QSlider*> m_vSliders;
	CDistrFunctionDialog* m_pDistributor;
	size_t m_iCurrTime;
	bool m_bExternalViewState;

	std::vector<int> m_vLastCombos;
	std::vector<int> m_vLastSliders;

public:
	CSolidDistributionsEditor(QWidget *parent);
	~CSolidDistributionsEditor();

	void SetFlowsheet(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB);
	void SetDistribution(CMDMatrix* _pDistribution, CBaseStream* _pStream);
	void SetTimePoint(size_t _index);

	void GetViewState(std::vector<int> &_vLastCombos, std::vector<int> &_vLastSliders) const;
	void SetViewState(const std::vector<int> &_vLastCombos, const std::vector<int> &_vLastSliders);

	void SetEditable(bool _bEditable) const; // Turn on/off edit possibility.

public slots:
	void setVisible(bool _bVisible) override;

private:
	void UpdateWholeView();
	void UpdateCombos();
	void UpdateTimeLabel() const;
	void UpdateFractionsTable() const;
	void UpdateDistributionsList() const;
	void UpdateDistributionTable() const;

	void AddAdditionalDim();
	void SetupComboBoxDimensions(QComboBox *_pCombo);
	void SetupComboBoxPSDType() const;
	void SetupComboBoxPSDGridType() const;
	void UpdatePSDComboVisible() const;

	void EnableMainTable() const;
	void CheckCompoundNormalization() const;
	void SetDistributionTableHeaders() const;
	std::vector<std::string> GetHeaders(EDistrTypes _distr) const;
	void CheckDistrNormalization() const;
	EDistrTypes GetSelectedDim() const;
	EDistrTypes GetTypeFromCombo(size_t _index) const;
	void SaveViewState();
	void LoadViewState();

	std::vector<EDistrTypes> ChosenTypes() const;
	std::vector<unsigned> ChosenCoords() const;
	EPSDTypes ChosenPSDType() const;
	EPSDGridType ChosenPSDGridType() const;
	std::string ChosenCompound() const;

private slots:
	void TableItemChanged() const;
	void DimsComboChanged() const;
	void DimsSliderChanged() const;
	void CompoundChanged() const;
	void CompoundFractionChanged( QTableWidgetItem* _pItem );
	void ComboPSDTypeChanged() const;
	void ComboPSDGridTypeChanged() const;
	void FunctionalCalled() const;
	void ApplyPressed();

private:


signals:
	void DataChanged();
};
