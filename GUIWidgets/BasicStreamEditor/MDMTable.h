/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MDMatrix.h"
#include "QtTable.h"
#include <QHBoxLayout>

class CMDMTable : public QWidget
{
	Q_OBJECT

private:
	CMDMatrix* m_pData;
	CQtTable* m_pTable;
	bool m_bAvoidSignal;
	QHBoxLayout *layout;

public:
	CMDMTable(QWidget *parent);
	~CMDMTable();

	void SetDistribution( CMDMatrix* _pDistribution, std::vector<std::string>& _sNames );
	void SetEditable(bool _bEditable);				// Turn on/off edit possibility.

private:
	void SetHeaders( std::vector<std::string>& _sNames );
	void CheckNormalization();

signals:
	void DataChanged();

public slots:
	void UpdateWholeView();
	//void setVisible( bool _bVisible );

private slots:
	void ItemWasChanged( QTableWidgetItem* _pItem );
};
