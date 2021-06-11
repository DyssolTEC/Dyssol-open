/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Simulator.h"
#include "Flowsheet.h"
#include "ParametersHolder.h"
#include "Stream.h"
#include "Phase.h"
#include "UnitContainer.h"
#include "DynamicUnit.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#include "DistributionsGrid.h"
#include "DyssolUtilities.h"

CSimulator::CSimulator()
{
	ClearLogState();
	m_pFlowsheet = nullptr;
	m_pParams = nullptr;
	m_nCurrentStatus = ESimulatorStatus::SIMULATOR_IDLE;
}

void CSimulator::SetFlowsheet(CFlowsheet* _pFlowsheet)
{
	m_pFlowsheet = _pFlowsheet;
	m_pSequence = m_pFlowsheet->GetCalculationSequence();
	m_pParams = m_pFlowsheet->GetParameters();
}

void CSimulator::SetCurrentStatus(ESimulatorStatus _nStatus)
{
	m_nCurrentStatus = _nStatus;
}

ESimulatorStatus CSimulator::GetCurrentStatus() const
{
	return m_nCurrentStatus;
}

void CSimulator::Simulate()
{
	m_nCurrentStatus = ESimulatorStatus::SIMULATOR_RUNNED;

	// Prepare
	SetupConvergenceMethod();
	ClearLogState();

	// clear previous results
	m_pFlowsheet->ClearSimulationResults();

	// Clear initialization flags
	m_vInitialized.clear();
	for (const auto& partition : m_pSequence->Partitions())
		for (const auto& model : partition.models)
			m_vInitialized[model->GetKey()] = false;

	// run logger updater
	m_logUpdater.Run();

	// set initial values to tear streams
	m_pFlowsheet->GetCalculationSequence()->InitializeTearStreams(m_pParams->initTimeWindow);

	// Simulate all units
	for (const auto& partition : m_pSequence->Partitions())
	{
		if (partition.tearStreams.empty())	// step without cycles
		{
			SimulateUnits(partition, 0, m_pParams->endSimulationTime);		// simulation on time interval itself
			ReduceData(partition, 0, m_pParams->endSimulationTime);
		}
		else															// step with recycles
			SimulateUnitsWithRecycles(partition);								// waveform relaxation on time interval

		if (m_nCurrentStatus == ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED) break;

		// Finalize all units within partition
		for (auto& model : partition.models)
		{
			m_log.WriteInfo(StrConst::Sim_InfoUnitFinalization(model->GetName(), model->GetModel()->GetUnitName()));
			model->GetModel()->DoFinalizeUnit();
		}
	}

	// stop logger updater
	m_logUpdater.Stop();

	// Save new initial values of tear streams
	if(m_pParams->initializeTearStreamsAutoFlag)
	{
		m_log.WriteInfo(StrConst::Sim_InfoSaveInitTearStreams);
		m_pFlowsheet->GetCalculationSequence()->UpdateInitialStreams(m_pParams->initTimeWindow);
	}

	m_nCurrentStatus = ESimulatorStatus::SIMULATOR_IDLE;
}

void CSimulator::SimulateUnitsWithRecycles(const CCalculationSequence::SPartition& _partition)
{
	const std::vector<CStream*>& vRecycles = _partition.tearStreams;

	// check whether initial values were set to recycle streams
	bool bTearStreamsFromInit = false; // will be true if data from m_vvInitTearStreams were used to initialize tear streams
	for (auto recycle : vRecycles)
		bTearStreamsFromInit |= !recycle->GetTimePoints(0, m_pParams->initTimeWindow).empty();

	// initialize simulation's parameters
	m_iTWIterationFull = 0;
	m_iTWIterationCurr = 0;
	m_iWindowNumber = 0;
	m_dTWStart = 0;
	m_dTWLength = m_pParams->initTimeWindow;
	m_dTWEnd = m_dTWStart + m_dTWLength;
	double dTWStartPrev = 0;					// start time of the previous time window

	// create and initialize structure of buffer streams
	std::vector<CStream*> vRecyclesPrev(vRecycles.size());			// previous state of recycles
	std::vector<CStream*> vRecyclesPrevPrev(vRecycles.size());		// pre-previous state of recycles
	for (size_t i = 0; i < vRecycles.size(); ++i)
	{
		vRecyclesPrev[i] = new CStream(*vRecycles[i]);
		vRecyclesPrev[i]->RemoveAllTimePoints();
		vRecyclesPrevPrev[i] = new CStream(*vRecycles[i]);
		vRecyclesPrevPrev[i]->RemoveAllTimePoints();
	}

	// main calculation sequence
	while (m_dTWStart < m_pParams->endSimulationTime)
	{
		if (m_dTWLength < m_pParams->minTimeWindow)
			RaiseError(StrConst::Sim_ErrMinTWLength);
		if (m_iTWIterationFull == m_pParams->maxItersNumber)
			RaiseError(StrConst::Sim_ErrMaxTWIterations);
		if (m_nCurrentStatus == ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED)
			break;

		// adjust time window
		if (m_dTWEnd > m_pParams->endSimulationTime)
			m_dTWEnd = m_pParams->endSimulationTime;

		// write log
		m_log.WriteInfo(StrConst::Sim_InfoRecycleStreamCalculating(m_iWindowNumber, m_iTWIterationFull, m_dTWStart, m_dTWEnd), true);

		// save copies of streams
		for (size_t j = 0; j < vRecycles.size(); ++j)
		{
			vRecyclesPrevPrev[j]->CopyFromStream(dTWStartPrev, m_dTWEnd, vRecyclesPrev[j]);
			vRecyclesPrev[j]->CopyFromStream(dTWStartPrev, m_dTWEnd, vRecycles[j]);
		}

		// simulation itself
		SimulateUnits(_partition, m_dTWStart, m_dTWEnd);

		m_iTWIterationFull++;
		m_iTWIterationCurr++;

		// check convergence
		if (!CheckConvergence(vRecycles, vRecyclesPrev, m_dTWStart, m_dTWEnd))
		{
			// cannot converge with automatic defined initial conditions in tear streams. set defaults and try again
			if (m_dTWStart == 0 && m_iTWIterationCurr > m_pParams->iters1stUpperLimit && m_pParams->initializeTearStreamsAutoFlag && bTearStreamsFromInit)
			{
				m_log.WriteInfo(StrConst::Sim_InfoFalseInitTearStreams, true);					// warn the user
				for (auto& stream : vRecycles)			stream->RemoveAllTimePoints();			// clear recycle streams
				for (auto& stream : vRecyclesPrev)		stream->RemoveAllTimePoints();			// clear previous state of recycles
				for (auto& stream : vRecyclesPrevPrev)	stream->RemoveAllTimePoints();			// clear pre-previous state of recycles
				bTearStreamsFromInit = false;													// turn off the control flag to prevent a repeated reset
				m_iTWIterationFull = 0;															// reset iteration number
				m_iTWIterationCurr = 0;															// reset iteration number
				continue;																		// repeat calculations
			}

			// apply chosen convergence method
			if (m_iTWIterationFull > 2)
				ApplyConvergenceMethod(vRecycles, vRecyclesPrev, vRecyclesPrevPrev, m_dTWStart, m_dTWEnd);

			// reduce time window if necessary
			if (((m_dTWStart == 0) && (m_iTWIterationCurr > m_pParams->iters1stUpperLimit)) ||	// for the first window
				((m_dTWStart != 0) && (m_iTWIterationCurr > m_pParams->itersUpperLimit)))		// for other windows
			{
				m_dTWLength /= m_pParams->magnificationRatio;
				m_dTWEnd = m_dTWStart + m_dTWLength;
				m_iTWIterationCurr = 0;
			}

			continue; // repeat calculations on time window
		}

		// first time window && converged in the first iteration -> proper initial values -> no parameters have been changed from the previous run ->
		// use old initial values instead of calculated ones to maintain the consistency of the results in consecutive simulations of the same flowsheet
		if (m_dTWStart == 0 && m_iTWIterationFull == 1)
			for (size_t i = 0; i < vRecycles.size(); ++i)
				vRecycles[i]->CopyFromStream(dTWStartPrev, m_dTWEnd, vRecyclesPrev[i]);

		// save units state
		for (auto& model : _partition.models)
			model->GetModel()->DoSaveStateUnit(m_dTWStart, m_dTWEnd);

		if (m_dTWEnd < m_pParams->endSimulationTime)
		{
			// recalculate time window if necessary
			if (m_iTWIterationCurr < m_pParams->itersLowerLimit)
				m_dTWLength *= m_pParams->magnificationRatio;	// increase time window
			else if (m_iTWIterationCurr > m_pParams->itersUpperLimit)
				m_dTWLength /= m_pParams->magnificationRatio;	// decrease time window
			if (m_dTWLength > m_pParams->maxTimeWindow)
				m_dTWLength = m_pParams->maxTimeWindow;			// set maximum time window

			// setup simulation's parameters and move to the next time window
			m_iTWIterationCurr = 0;
			m_iTWIterationFull = 0;
			m_iWindowNumber++;
			dTWStartPrev = m_dTWStart;
			m_dTWStart = m_dTWEnd;
			m_dTWEnd += m_dTWLength;

			// make prediction
			ApplyExtrapolationMethod(vRecycles, dTWStartPrev, m_dTWStart, m_dTWEnd);

			// remove excessive data
			ReduceData(_partition, dTWStartPrev, m_dTWStart);
		}
		else
		{
			// remove excessive data
			ReduceData(_partition, m_dTWStart, m_dTWEnd);

			// finish simulation of the partition
			break;
		}
	}

	// remove buffer streams
	for (size_t i = 0; i < vRecycles.size(); ++i)
	{
		delete vRecyclesPrev[i];
		delete vRecyclesPrevPrev[i];
	}
	vRecyclesPrev.clear();
	vRecyclesPrevPrev.clear();
}

void CSimulator::SimulateUnits(const CCalculationSequence::SPartition& _partition, double _t1, double _t2)
{
	for (auto& model : _partition.models)
	{
		// current model
		m_unitName = model->GetName();

		// initialize unit if not yet initialized
		if (!m_vInitialized[model->GetKey()])
		{
			InitializeUnit(*model, _t1);
			m_vInitialized[model->GetKey()] = true;
		}

		// check for stopping flag
		if (m_nCurrentStatus == ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED) break;

		// write log
		m_log.WriteInfo(StrConst::Sim_InfoUnitSimulation(m_unitName, model->GetModel()->GetUnitName(), _t1, _t2));

		// load previous state
		model->GetModel()->DoLoadStateUnit();

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
			if (std::fabs(vTimePoints.back() - _t2) > 16 * std::numeric_limits<double>::epsilon())
				vTimePoints.push_back(_t2);
			if (vTimePoints.front() != 0)
				vTimePoints.erase(vTimePoints.begin()); // already calculated on previous time window

			// for each time point
			for (auto t : vTimePoints)
			{
				// check for stopping flag
				if (m_nCurrentStatus == ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED) break;
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
		if (!CompareStreams(*_vStreams1[i], *_vStreams2[i], _t1, _t2))
			return false;
	return true;
}

bool CSimulator::CompareStreams(const CStream& _str1, const CStream& _str2, double _t1, double _t2) const
{
	// get all time points
	std::vector<double> vTimePoints = VectorsUnionSorted(_str1.GetTimePoints(_t1, _t2), _str2.GetTimePoints(_t1, _t2));
	if (vTimePoints.empty())
		return true;

	// remove the first time point as it was analyzed on the previous time window
	if (vTimePoints.front() != 0)
		vTimePoints.erase(vTimePoints.begin());

	// compare
	for (auto t : vTimePoints)
		if (!CBaseStream::AreEqual(t, _str1, _str2))
			return false;

	return true;
}

void CSimulator::RaiseError(const std::string& _sError)
{
	m_log.WriteError(_sError);
	m_nCurrentStatus = ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED;
}

void CSimulator::ClearLogState()
{
	m_dTWStart = 0;
	m_dTWEnd = 0;
	m_iTWIterationFull = 0;
	m_iWindowNumber = 0;
	m_unitName.clear();
	m_log.Clear();
}

void CSimulator::ApplyExtrapolationMethod(const std::vector<CStream*>& _streams, double _t1, double _t2, double _tExtra) const
{
	if (_t2 >= m_pParams->endSimulationTime) return;
	switch (static_cast<EExtrapMethod>(m_pParams->extrapolationMethod))
	{
	case EExtrapMethod::EM_LINEAR:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t1, _t2);						break;
	case EExtrapMethod::EM_SPLINE:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t1, (_t2 + _t1) / 2, _t2);	break;
	case EExtrapMethod::EM_NEAREST:	for (auto& str : _streams) str->Extrapolate(_tExtra, _t2);							break;
	}
}

void CSimulator::SetupConvergenceMethod()
{
	m_bSteffensenTrigger = true;
	m_vDims = m_pFlowsheet->GetDistributionsGrid()->GetDistrTypes();
	m_mMDnew.SetDimensions(vector_cast<unsigned>(m_vDims), m_pFlowsheet->GetDistributionsGrid()->GetClasses());
	m_vMTPnew.resize(3);
	m_vPFnew.resize(m_pFlowsheet->GetPhasesNumber());
}

void CSimulator::ApplyConvergenceMethod(const std::vector<CStream*>& _s3, std::vector<CStream*>& _s2, std::vector<CStream*>& _s1, double _t1, double _t2)
{
	if ((m_pParams->convergenceMethod == CM_DIRECT_SUBSTITUTION) && (m_pParams->relaxationParam == 1.))
		return;
	if (m_pParams->convergenceMethod == CM_STEFFENSEN)
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
					_s3[i]->SetDistribution(time, PredictValues(_s3[i]->GetDistribution(time, m_pFlowsheet->GetDistributionsGrid()->GetDistrTypes()), _s2[i]->GetDistribution(time, m_pFlowsheet->GetDistributionsGrid()->GetDistrTypes()), _s1[i]->GetDistribution(time, m_pFlowsheet->GetDistributionsGrid()->GetDistrTypes())));
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
	switch (static_cast<EConvMethod>(m_pParams->convergenceMethod))
	{
	case CM_DIRECT_SUBSTITUTION:
		for (size_t i = 0; i < _len; ++i)
			_res[i] = PredictRelaxation(_v3[i], _v2[i]);
		break;
	case CM_WEGSTEIN:
		for (size_t i = 0; i < _len; ++i)
			_res[i] = PredictWegstein(_v3[i], _v2[i], _v2[i], _v1[i]);
		break;
	case CM_STEFFENSEN:
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
				for (auto& p : model->GetModel()->GetPortsManager().GetAllOutputPorts())
					p->GetStream()->ReduceTimePoints(dStart, _t2, m_pParams->saveTimeStep);
				if (m_pParams->saveTimeStepFlagHoldups)
					model->GetModel()->ReduceTimePoints(dStart, _t2, m_pParams->saveTimeStep);
			}
	}
}

EPhase CSimulator::PhaseSOA2EPhase(unsigned _soa)
{
	switch (_soa)
	{
	case SOA_SOLID:		return EPhase::SOLID;
	case SOA_LIQUID:	return EPhase::LIQUID;
	case SOA_VAPOR:		return EPhase::VAPOR;
	default:			return EPhase::UNDEFINED;
	}
}