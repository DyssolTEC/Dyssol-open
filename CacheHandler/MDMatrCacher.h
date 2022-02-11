/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseCacheHandler.h"

class CMDMatrCacher :
	public CBaseCacheHandler
{
public:
	CMDMatrCacher(void);
	~CMDMatrCacher(void);

	void ReadFromCache(double _dT, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData, double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const;
	void ReadFromCache(double _dT1, double _dT2, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData, double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const;
	void WriteToCache( const std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData, bool _bCoherent );

private:
	void WriteData(size_t _nIndex, std::vector<std::vector<double>>& _vvData, const std::vector<double>& _vTP, size_t _nStartTP, size_t _nSize, bool _bInsert);

	void ReadFromFile(size_t _nIndex, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData) const;
	void WriteToFile(SDescriptor& _currDescr, bool _bInsert, size_t _nNumber, const std::vector<double>& _vTimePoints, const std::vector<std::vector<double>>& _vvData, size_t _nOffset) const;
};

