/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Simulator.h"
#include "Flowsheet.h"
#include "ParametersHolder.h"
#include "Stream.h"
#include "Phase.h"
#include "UnitContainer.h"
#include "DynamicUnit.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"

CSimulator::CSimulator()
{
	ClearLogState();
	m_pFlowsheet = nullptr;
	m_pParams = nullptr;
	m_nCurrentStatus = ESimulatorState::IDLE;
}

void CSimulator::SetFlowsheet(CFlowsheet* _pFlowsheet)
{
	m_pFlowsheet = _pFlowsheet;
	m_pSequence = m_pFlowsheet->GetCalculationSequence();
	m_pParams = m_pFlowsheet->GetParameters();
	m_hasError = false;
}

void CSimulator::SetCurrentStatus(ESimulatorState _nStatus)
{
	m_nCurrentStatus = _nStatus;
}

ESimulatorState CSimulator::GetCurrentStatus() const
{
	return m_nCurrentStatus;
}

bool CSimulator::HasError() const
{
	return m_hasError;
}

CSimulator::SPartitionStatus CSimulator::GetCurrentPartitionStatus() const
{
	if (m_iCurrentPartition >= m_partitionsStatus.size()) return {};
	return m_partitionsStatus[m_iCurrentPartition];
}

void CSimulator::Simulate()
{
	m_nCurrentStatus = ESimulatorState::RUNNING;
	m_hasError = false;

	// Prepare
	SetupConvergenceMethod();
	ClearLogState();

	m_hasError = false;
	InitializePartitionsStatus();

	// Clear initialization flags
	m_vInitialized.clear();
	for (const auto& partition : m_pSequence->Partitions())
		for (const auto& model : partition.models)
			m_vInitialized[model->GetKey()] = false;

	// run logger updater
	m_logUpdater.Run();

	// set initial values to tear streams
	m_pFlowsheet->GetCalculationSequence()->CopyInitToTearStreams(m_pParams->initTimeWindow);

	// Simulate all units
	const auto partitions = m_pSequence->Partitions();
	// TODO: work only with partition index, when getting a partition data by index will be a fast operation
	for (size_t iPart = 0; iPart < partitions.size(); ++iPart)
	{
		m_iCurrentPartition = iPart;
		SimulateUntilEndSimulationTime(iPart, partitions[iPart]);

		if (m_nCurrentStatus == ESimulatorState::TO_BE_STOPPED) break;

		// remove excessive data
		ReduceData(partitions[iPart], m_pParams->startSimulationTime, m_pParams->endSimulationTime);

		// Finalize all units within partition
		for (auto& model : partitions[iPart].models)
		{
			m_log.WriteInfo(StrConst::Sim_InfoUnitFinalization(model->GetName(), model->GetModel()->GetUnitName()));
			model->GetModel()->DoFinalizeUnit();
		}
	}

	// Save new initial values of tear streams
	if(m_pParams->initializeTearStreamsAutoFlag)
	{
		m_log.WriteInfo(StrConst::Sim_InfoSaveInitTearStreams);
		m_pFlowsheet->GetCalculationSequence()->CopyTearToInitStreams(m_pParams->initTimeWindow);
	}

	// remove buffer streams
	for (size_t i = 0; i < m_partitionsStatus.size(); ++i)
		for (size_t j = 0; j < m_partitionsStatus[i].vRecyclesPrev.size(); ++j)
		{
			delete m_partitionsStatus[i].vRecyclesPrev[j];
			delete m_partitionsStatus[i].vRecyclesPrevPrev[j];
		}

	m_log.WriteInfo("");

	// stop logger updater
	m_logUpdater.Stop();
	m_nCurrentStatus = ESimulatorState::IDLE;
}

void CSimulator::Stop()
{
	if (GetCurrentStatus() != ESimulatorState::IDLE)
		SetCurrentStatus(ESimulatorState::TO_BE_STOPPED);
}

void CSimulator::InitializePartitionsStatus()
{
	m_partitionsStatus.clear();
	for (const auto& partition : m_pSequence->Partitions())
	{
		SPartitionStatus& status = m_partitionsStatus.emplace_back();

		// create and initialize structure of buffer streams
		const std::vector<CStream*>& vRecycles = partition.tearStreams;
		status.vRecyclesPrev = std::vector<CStream*>(vRecycles.size());			// previous state of recycles
		status.vRecyclesPrevPrev = std::vector<CStream*>(vRecycles.size());		// pre-previous state of recycles
		for (size_t i = 0; i < vRecycles.size(); ++i)
		{
			status.vRecyclesPrev[i] = new CStream(*vRecycles[i]);
			status.vRecyclesPrev[i]->RemoveAllTimePoints();
			status.vRecyclesPrevPrev[i] = new CStream(*vRecycles[i]);
			status.vRecyclesPrevPrev[i]->RemoveAllTimePoints();
		}
	}
}

void CSimulator::SimulateUntilEndSimulationTime(size_t _iPartition, const CCalculationSequence::SPartition& _partition)
{
	if (_partition.tearStreams.empty())	// step without cycles
		SimulateUnits(_partition, m_pParams->startSimulationTime, m_pParams->endSimulationTime);		// simulation on time interval itself
	else															// step with recycles
		SimulateUnitsWithRecycles(_iPartition, _partition, m_pParams->startSimulationTime, m_pParams->endSimulationTime);		// waveform relaxation on time interval
}

void CSimulator::SimulateUnitsWithRecycles(size_t _iPartition, const CCalculationSequence::SPartition& _partition, double _t1, double _t2)
{
	const std::vector<CStream*>& vRecycles = _partition.tearStreams;

	// initialize simulation's parameters
	SPartitionStatus& partVars = m_partitionsStatus[_iPartition];

	if (_t1 == 0)
	{
		// check whether initial values were set to recycle streams
		partVars.bTearStreamsFromInit = false; // will be true if data from m_vvInitTearStreams were used to initialize tear streams
		for (auto recycle : vRecycles)
			partVars.bTearStreamsFromInit |= !recycle->GetTimePoints(0, m_pParams->initTimeWindow).empty();

		partVars.dTWLength = m_pParams->initTimeWindow;
	}

	partVars.dTWStart = _t1;
	partVars.dTWEnd = std::min(partVars.dTWStart + partVars.dTWLength, _t2);

	// main calculation sequence
	while (partVars.dTWStart < _t2)
	{
		if (partVars.dTWLength < m_pParams->minTimeWindow)
			RaiseError(StrConst::Sim_ErrMinTWLength);
		if (partVars.iTWIterationFull == m_pParams->maxItersNumber)
			RaiseError(StrConst::Sim_ErrMaxTWIterations);
		if (m_nCurrentStatus == ESimulatorState::TO_BE_STOPPED)
			break;

		// write log
		m_log.WriteInfo(StrConst::Sim_InfoRecycleStreamCalculating(partVars.iWindowNumber, partVars.iTWIterationFull, partVars.dTWStart, partVars.dTWEnd), true);

		// save copies of streams
		for (size_t j = 0; j < vRecycles.size(); ++j)
		{
			partVars.vRecyclesPrevPrev[j]->CopyFromStream(partVars.dTWStartPrev, partVars.dTWEnd, partVars.vRecyclesPrev[j]);
			partVars.vRecyclesPrev[j]->CopyFromStream(partVars.dTWStartPrev, partVars.dTWEnd, vRecycles[j]);
		}

		// load units state
		for (auto& model : _partition.models)
			model->GetModel()->DoLoadStateUnit();

		// simulation itself
		SimulateUnits(_partition, partVars.dTWStart, partVars.dTWEnd);

		partVars.iTWIterationFull++;
		partVars.iTWIterationCurr++;

		// check convergence
		if (!CheckConvergence(vRecycles, partVars.vRecyclesPrev, partVars.dTWStart, partVars.dTWEnd))
		{
			// cannot converge with automatic defined initial conditions in tear streams. set defaults and try again
			if (partVars.dTWStart == 0 && partVars.iTWIterationCurr > m_pParams->iters1stUpperLimit && m_pParams->initializeTearStreamsAutoFlag && partVars.bTearStreamsFromInit)
			{
				m_log.WriteInfo(StrConst::Sim_InfoFalseInitTearStreams, true);					// warn the user
				for (auto& stream : vRecycles)					stream->RemoveAllTimePoints();			// clear recycle streams
				for (auto& stream : partVars.vRecyclesPrev)		stream->RemoveAllTimePoints();			// clear previous state of recycles
				for (auto& stream : partVars.vRecyclesPrevPrev)	stream->RemoveAllTimePoints();			// clear pre-previous state of recycles
				for (auto& stream : vRecycles)													// make sure, there is at least one time point in the stream
					if (stream->GetAllTimePoints().empty())
						stream->AddTimePoint(0.0);
				partVars.bTearStreamsFromInit = false;											// turn off the control flag to prevent a repeated reset
				partVars.iTWIterationFull = 0;													// reset iteration number
				partVars.iTWIterationCurr = 0;													// reset iteration number
				continue;																		// repeat calculations
			}

			// apply chosen convergence method
			if (partVars.iTWIterationFull > 2)
				ApplyConvergenceMethod(vRecycles, partVars.vRecyclesPrev, partVars.vRecyclesPrevPrev, partVars.dTWStart, partVars.dTWEnd);

			// reduce time window if necessary
			if (((partVars.dTWStart == 0) && (partVars.iTWIterationCurr > m_pParams->iters1stUpperLimit)) ||	// for the first window
				((partVars.dTWStart != 0) && (partVars.iTWIterationCurr > m_pParams->itersUpperLimit)))		// for other windows
			{
				partVars.dTWLength /= m_pParams->magnificationRatio;
				partVars.dTWEnd = std::min(partVars.dTWStart + partVars.dTWLength, _t2);
				partVars.iTWIterationCurr = 0;
			}

			continue; // repeat calculations on time window
		}

		// first time window && converged in the first iteration -> proper initial values -> no parameters have been changed from the previous run ->
		// use old initial values instead of calculated ones to maintain the consistency of the results in consecutive simulations of the same flowsheet
		if (partVars.dTWStart == 0 && partVars.iTWIterationFull == 1)
			for (size_t i = 0; i < vRecycles.size(); ++i)
				vRecycles[i]->CopyFromStream(partVars.dTWStartPrev, partVars.dTWEnd, partVars.vRecyclesPrev[i]);

		// save units state
		for (auto& model : _partition.models)
			model->GetModel()->DoSaveStateUnit(partVars.dTWStart, partVars.dTWEnd);

		if (partVars.dTWEnd < _t2)
		{
			// recalculate time window if necessary
			if (partVars.iTWIterationCurr < m_pParams->itersLowerLimit)
				partVars.dTWLength *= m_pParams->magnificationRatio;	// increase time window
			else if (partVars.iTWIterationCurr > m_pParams->itersUpperLimit)
				partVars.dTWLength /= m_pParams->magnificationRatio;	// decrease time window
			if (partVars.dTWLength > m_pParams->maxTimeWindow)
				partVars.dTWLength = m_pParams->maxTimeWindow;			// set maximum time window

			// setup simulation's parameters and move to the next time window
			partVars.iTWIterationCurr = 0;
			partVars.iTWIterationFull = 0;
			partVars.iWindowNumber++;
			partVars.dTWStartPrev = partVars.dTWStart;
			partVars.dTWStart = partVars.dTWEnd;
			partVars.dTWEnd = std::min(partVars.dTWEnd + partVars.dTWLength, _t2);

			// make prediction
			ApplyExtrapolationMethod(vRecycles, partVars.dTWStartPrev, partVars.dTWStart, partVars.dTWEnd);
		}
		else
		{
			// finish simulation of the partition
			break;
		}
	}
}

void CSimulator::SimulateUnits(const CCalculationSequence::SPartition& _partition, double _t1, double _t2)
{
	for (auto& model : _partition.models)
	{
		// current model
		m_unitName = model->GetName();

		// copy output streams to input streams and convert grids if necessary
		m_pFlowsheet->PrepareInputStreams(model, _t1, _t2);

		// initialize unit if not yet initialized
		if (!m_vInitialized[model->GetKey()])
		{
			InitializeUnit(*model, _t1);
			m_vInitialized[model->GetKey()] = true;
		}

		// check for stopping flag
		if (m_nCurrentStatus == ESimulatorState::TO_BE_STOPPED) break;

		// write log
		m_log.WriteInfo(StrConst::Sim_InfoUnitSimulation(m_unitName, model->GetModel()->GetUnitName(), _t1, _t2));

		// clean output streams
		for (auto& port : model->GetModel()->GetPortsManager().GetAllOutputPorts())
			port->GetStream()->RemoveTimePointsAfter(_t1);

		// simulate
		if (dynamic_cast<CDynamicUnit*>(model->GetModel()))	// for dynamic units
		{
			// simulate
			SimulateUnit(*model, _t1, _t2);
		}
		else	// for steady-state units
		{
			// get all time points in current window + _dEndTime
			std::vector<double> vTimePoints = model->GetModel()->GetAllTimePoints(_t1, _t2);
			if (vTimePoints.empty() || std::fabs(vTimePoints.back() - _t2) > 16 * std::numeric_limits<double>::epsilon())
				vTimePoints.push_back(_t2);
			if (vTimePoints.size() != 1 && vTimePoints.front() != 0.0)
				vTimePoints.erase(vTimePoints.begin()); // already calculated on previous time window

			// for each time point
			for (auto t : vTimePoints)
			{
				// check for stopping flag
				if (m_nCurrentStatus == ESimulatorState::TO_BE_STOPPED) break;
				// simulate
				SimulateUnit(*model, t);
			}
		}
	}
}

void CSimulator::SimulateUnit(CUnitContainer& _unit, double _t1, double _t2 /*= -1*/)
{
	auto* model = _unit.GetModel();

	m_logUpdater.SetModel(model);

	// simulate
	try {
		if(dynamic_cast<CDynamicUnit*>(model))
			model->Simulate(_t1, _t2);
		else
			model->Simulate(_t1);
	}
	catch (const std::logic_error& e) {
		RaiseError(e.what());
	}

	m_logUpdater.ReleaseModel();

	// check for errors
	if (model->HasError())
		RaiseError(model->PopErrorMessage());
}

void CSimulator::InitializeUnit(CUnitContainer& _unit, double _t)
{
	auto* model = _unit.GetModel();

	// write log
	m_log.WriteInfo(StrConst::Sim_InfoUnitInitialization(m_unitName, model->GetUnitName()));
	//CLogUpdater logUpdater{ &m_log, model };
	try {
		model->DoInitializeUnit();
	}
	catch (const std::logic_error& e) {
		RaiseError(e.what());
	}
	//logUpdater.Release();

	if (model->HasError())
		RaiseError(model->GetErrorMessage());

	// check unit parameters
	for (const CBaseUnitParameter* param : model->GetUnitParametersManager().GetParameters())
		if (!param->IsInBounds())
			m_log.WriteWarning(StrConst::Sim_WarningParamOutOfRange(model->GetUnitName(), m_unitName, param->GetName()));
}

bool CSimulator::CheckConvergence(const std::vector<CStream*>& _vStreams1, const std::vector<CStream*>& _vStreams2, double _t1, double _t2) const
{
	for (size_t i = 0; i < _vStreams1.size(); ++i)
	{
		if (!CompareStreams(*_vStreams1[i], *_vStreams2[i], _t1, _t2))
		{
			return false;
		}
	}
	return true;
}

bool CSimulator::CompareStreams(const CStream& _str1, const CStream& _str2, double _t1, double _t2) const
{
	// get all time points
	std::vector<double> timePoints = VectorsUnionSorted(_str1.GetTimePoints(_t1, _t2), _str2.GetTimePoints(_t1, _t2));
	if (timePoints.empty())
		return true;

	// remove the first time point as it was analyzed on the previous time window
	if (timePoints.front() != 0.0)
		timePoints.erase(timePoints.begin());

	// compare
	return std::all_of(timePoints.begin(), timePoints.end(), [&](double t)
	{
		return CStream::AreEqual(t, _str1, _str2);
	});
}

void CSimulator::RaiseError(const std::string& _sError)
{
	m_log.WriteError(_sError);
	m_nCurrentStatus = ESimulatorState::TO_BE_STOPPED;
	m_hasError = true;
}

void CSimulator::ClearLogState()
{
	m_unitName.clear();
	m_log.Clear();
}

void CSimulator::ApplyExtrapolationMethod(const std::vector<CStream*>& _streams, double _t1, double _t2, double _tExtra) const
{
	switch (static_cast<EExtrapolationMethod>(m_pParams->extrapolationMethod))
	{
	case EExtrapolationMethod::LINEAR:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t1, _t2);						break;
	case EExtrapolationMethod::SPLINE:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t1, (_t2 + _t1) / 2, _t2);	break;
	case EExtrapolationMethod::NEAREST:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t2);							break;
	}
}

void CSimulator::SetupConvergenceMethod()
{
	m_bSteffensenTrigger = true;
}

void CSimulator::ApplyConvergenceMethod(const std::vector<CStream*>& _s3, std::vector<CStream*>& _s2, std::vector<CStream*>& _s1, double _t1, double _t2)
{
	if (static_cast<EConvergenceMethod>(m_pParams->convergenceMethod) == EConvergenceMethod::DIRECT_SUBSTITUTION && m_pParams->relaxationParam == 1.)
		return;
	if (static_cast<EConvergenceMethod>(m_pParams->convergenceMethod) == EConvergenceMethod::STEFFENSEN)
	{
		m_bSteffensenTrigger = !m_bSteffensenTrigger;
		if (m_bSteffensenTrigger)
			return;
	}

	for (size_t i = 0; i < _s3.size(); ++i)
		for (auto time : _s3[i]->GetTimePoints(_t1, _t2))
		{
			// TODO: go over all defined properties
			_s3[i]->SetOverallProperty(time, EOverall::OVERALL_MASS, PredictValues(_s3[i]->GetOverallProperty(time, EOverall::OVERALL_MASS), _s2[i]->GetOverallProperty(time, EOverall::OVERALL_MASS), _s1[i]->GetOverallProperty(time, EOverall::OVERALL_MASS)));
			_s3[i]->SetOverallProperty(time, EOverall::OVERALL_TEMPERATURE, PredictValues(_s3[i]->GetOverallProperty(time, EOverall::OVERALL_TEMPERATURE), _s2[i]->GetOverallProperty(time, EOverall::OVERALL_TEMPERATURE), _s1[i]->GetOverallProperty(time, EOverall::OVERALL_TEMPERATURE)));
			_s3[i]->SetOverallProperty(time, EOverall::OVERALL_PRESSURE, PredictValues(_s3[i]->GetOverallProperty(time, EOverall::OVERALL_PRESSURE), _s2[i]->GetOverallProperty(time, EOverall::OVERALL_PRESSURE), _s1[i]->GetOverallProperty(time, EOverall::OVERALL_PRESSURE)));
			for (const auto& phase : m_pFlowsheet->GetPhases())
			{
				_s3[i]->SetPhaseFraction(time, phase.state, PredictValues(_s3[i]->GetPhaseFraction(time, phase.state), _s2[i]->GetPhaseFraction(time, phase.state), _s1[i]->GetPhaseFraction(time, phase.state)));
				if (phase.state == EPhase::SOLID) // all distributed parameters
					_s3[i]->SetDistribution(time, PredictValues(_s3[i]->GetDistribution(time, _s3[i]->GetGrid().GetDimensionsTypes()), _s2[i]->GetDistribution(time, _s2[i]->GetGrid().GetDimensionsTypes()), _s1[i]->GetDistribution(time, _s1[i]->GetGrid().GetDimensionsTypes())));
				else				// only distribution by compounds
					_s3[i]->SetCompoundsFractions(time, phase.state, PredictValues(_s3[i]->GetCompoundsFractions(time, phase.state), _s2[i]->GetCompoundsFractions(time, phase.state), _s1[i]->GetCompoundsFractions(time, phase.state)));
			}
		}
}

double CSimulator::PredictValues(double _d3, double _d2, double _d1) const
{
	double res;
	PredictValues(1, &_d3, &_d2, &_d1, &res);
	return res;

}

std::vector<double> CSimulator::PredictValues(const std::vector<double>& _v3, const std::vector<double>& _v2, const std::vector<double>& _v1) const
{
	std::vector<double> res(_v3.size());
	PredictValues(_v3.size(), _v3.data(), _v2.data(), _v1.data(), res.data());
	return res;
}

CDenseMDMatrix CSimulator::PredictValues(const CDenseMDMatrix& _m3, const CDenseMDMatrix& _m2, const CDenseMDMatrix& _m1) const
{
	CDenseMDMatrix res(_m3.GetDimensions(), _m3.GetClasses());
	PredictValues(_m3.GetDataLength(), _m3.GetDataPtr(), _m2.GetDataPtr(), _m1.GetDataPtr(), res.GetDataPtr());
	return res;
}

void CSimulator::PredictValues(size_t _len, const double* _v3, const double* _v2, const double* _v1, double* _res) const
{
	switch (static_cast<EConvergenceMethod>(m_pParams->convergenceMethod))
	{
	case EConvergenceMethod::DIRECT_SUBSTITUTION:
		for (size_t i = 0; i < _len; ++i)
			_res[i] = PredictRelaxation(_v3[i], _v2[i]);
		break;
	case EConvergenceMethod::WEGSTEIN:
		for (size_t i = 0; i < _len; ++i)
			_res[i] = PredictWegstein(_v3[i], _v2[i], _v2[i], _v1[i]);
		break;
	case EConvergenceMethod::STEFFENSEN:
		for (size_t i = 0; i < _len; ++i)
			_res[i] = PredictSteffensen(_v3[i], _v2[i], _v1[i]);
		break;
	}
}

double CSimulator::PredictRelaxation(double F2, double F1) const
{
	return (1 - m_pParams->relaxationParam)*F1 + m_pParams->relaxationParam*F2;
}

double CSimulator::PredictWegstein(double F2, double F1, double X2, double X1) const
{
	if (std::fabs(X2 - X1) <= (std::fabs(X2)*m_pParams->relTol + m_pParams->absTol))
		return F2;
	const double s = ( F2 - F1 ) / ( X2 - X1 );
	double q = s / ( s - 1 );
	if (q < -5)
		q = -5;
	else if (q > m_pParams->wegsteinAccelParam)
		q = m_pParams->wegsteinAccelParam;
	return q*X2 + (1 - q)*F2;
}

double CSimulator::PredictSteffensen(double F3, double F2, double F1) const
{
	const double tmp = F3 - 2 * F2 + F1;
	if (std::fabs(tmp) <= (std::fabs(tmp)*m_pParams->relTol + m_pParams->absTol))
		return F3;
	return F1 - std::pow((F2 - F1), 2.) / tmp;
}

void CSimulator::ReduceData(const CCalculationSequence::SPartition& _partition, double _t1, double _t2) const
{
	if (m_pParams->saveTimeStep > 0.)
	{
		const double dStart = std::max(std::min(_t1, _t2 - 2 * m_pParams->saveTimeStep), 0.0);
		for (auto* model : _partition.models)
			if (model->GetModel()->GetStreamsManager().GetFeedsInit().empty()) // TODO: proper check for feed unit
			{
				for (auto& p : model->GetModel()->GetPortsManager().GetAllInputPorts())
					p->GetStream()->ReduceTimePoints(dStart, _t2, m_pParams->saveTimeStep);
				if (m_pParams->saveTimeStepFlagHoldups)
					model->GetModel()->ReduceTimePoints(dStart, _t2, m_pParams->saveTimeStep);
			}
	}
}
