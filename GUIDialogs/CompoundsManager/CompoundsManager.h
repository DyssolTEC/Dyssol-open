/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_CompoundsManager.h"

class CFlowsheet;
class CMaterialsDatabase;

class CCompoundsManager: public QDialog
{
	Q_OBJECT
private:
	Ui::CCompoundsManagerClass ui;
	bool m_bAvoidSignal;
	CFlowsheet* m_pFlowsheet; // pointer to the flowsheet
	CMaterialsDatabase* m_pMaterialsDatabase;
	std::vector<std::string> m_vAddedCompKeys;

public slots:
	void setVisible( bool _bVisible );
	void UpdateWholeView();
	void UpdateAvailableCompounds();
	void UpdateUsedCompounds();
	void AddSelectedCompound();
	void RemoveSelectedCompound();
	void close();
	void accept();

public:
	CCompoundsManager( CFlowsheet* _pFlowsheet, CMaterialsDatabase* _pDatabase, QWidget *parent = 0 );

	void InitializeConnections();

signals:
	void DataChanged();

private:
	void ApplyChanges();
	int GetAddedCompIndex( const std::string& _sKey ) const;
};
