/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_DustFormationTesterTab.h"
#include "DustFormationTester.h"

class CFlowsheet;
class CUnitContainer;
class CBaseStream;
class CStream;
class CHoldup;
class CMaterialsDatabase;

class CDustFormationTesterTab : public QDialog
{
	Q_OBJECT
private:
	Ui::DustFormationTesterTabClass ui;
	CDustFormationTester m_tester;
	const CFlowsheet* m_pFlowsheet;
	const CMaterialsDatabase* m_matrialsDB;
	enum class EType { STREAMS = 0, UNITS = 1 } m_focusType{ EType::STREAMS };

public:
	CDustFormationTesterTab(const CFlowsheet* _pFlowsheet, const CMaterialsDatabase* _matrialsDB, QWidget* parent = Q_NULLPTR);

	void InitializeConnections();
	void UpdateWholeView();

public slots:
	void setVisible(bool _bVisible) override;

private:
	void UpdateStreams() const;
	void UpdateUnits() const;
	void UpdateHoldups() const;
	void UpdatePorosity() const;
	void UpdateMoisture() const;
	void UpdateMoisture90() const;
	void UpdateCompounds() const;
	void UpdateDataTable();

	static void AddItemToList(QListWidget* _pList, const std::string& _sName, const std::string& _sKey);
	static void RestoreSelectedRow(QListWidget* _pList, int iRow);
	const CBaseStream* GetSelectedStream() const;
	const CUnitContainer* GetSelectedModel() const;
	const CHoldup* GetSelectedHoldup() const;
	const CStream* GetSelectedMaterialStream() const;
	std::string GetSelectedCompound() const;

private slots:
	void StreamsUnitsFocusChanged(int _index);
	void SelectedStreamChanged();
	void SelectedUnitChanged();
	void SelectedHoldupChanged();
	void PorosityChanged();
	void MoistureChanged();
	void Moisture90Changed();
	void CompoundChanged();
};
