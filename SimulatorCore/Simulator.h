/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Flowsheet.h"
#include "SimulatorLog.h"
#include "DenseMDMatrix.h"

enum class ESimulatorStatus
{
	SIMULATOR_IDLE              = 0,
	SIMULATOR_RUNNED            = 1,
	SIMULATOR_SHOULD_BE_STOPPED = 2
};

class CSimulator
{
	friend class CSimulatorTab;

private:
	CFlowsheet* m_pFlowsheet;
	const CCalculationSequence* m_pSequence; // Calculation sequence.
	CFlowsheetParameters* m_pParams;
	ESimulatorStatus m_nCurrentStatus;
	std::map<std::string, bool> m_vInitialized;

	/// Data for logging
	CSimulatorLog m_log;			// Log itself.
	double m_dTWStart;				// Current time window start.
	double m_dTWEnd;				// Current time window end.
	double m_dTWLength;				// Current time window length.
	unsigned m_iTWIterationFull;	// Iteration number within a current time window [m_dTWStart .. ]. Does not reset if the size of current TW is reduced.
	unsigned m_iTWIterationCurr;	// Iteration number within a current time window [m_dTWStart .. m_dTWEnd]. Reset if the size of current TW is reduced.
	unsigned m_iWindowNumber;		// Current time window within a partition.
	std::string m_sUnitName;		// Name of the currently calculated unit.

	//// parameters of convergence methods
	bool m_bSteffensenTrigger;
	std::vector<EDistrTypes> m_vDims;
	CDenseMDMatrix m_mMDnew;
	std::vector<double> m_vMTPnew;
	std::vector<double> m_vPFnew;

public:
	CSimulator();

	/// Sets pointer to a flowsheet.
	void SetFlowsheet(CFlowsheet* _pFlowsheet);

	// Change status of simulator.
	void SetCurrentStatus(ESimulatorStatus _nStatus);
	// Returns current status of the simulator.
	ESimulatorStatus GetCurrentStatus() const;

	/// Perform simulation.
	void Simulate();

private:
	/// Performs simulation of a given partition with waveform relaxation method.
	void SimulateUnitsWithRecycles(const CCalculationSequence::SPartition& _partition);
	/// Simulate all units of a given partition on specified time interval.
	void SimulateUnits(const CCalculationSequence::SPartition& _partition, double _t1, double _t2);
	/// Simulate specified steady-state or dynamic unit on a given time or interval.
	void SimulateUnit(CBaseModel& _model, double _t1, double _t2 = -1);
	/// Initialize the specified steady-state or dynamic unit at the given time.
	void InitializeUnit(CBaseModel& _model, double _t);

	/// Checks convergence comparing all values from _vStreams1 and _vStreams2 in pairs on the specified time interval. The length of _vStreams1 and _vStreams2 must be the same.
	bool CheckConvergence(const std::vector<CStream*>& _vStreams1, const std::vector<CStream*>& _vStreams2, double _t1, double _t2) const;
	/// Compares all values of two streams on a specified time interval. Returns true if streams are equal to within tolerance.
	bool CompareStreams(const CStream& _str1, const CStream& _str2, double _t1, double _t2) const;

	/// Sets error's description into log, stops simulation.
	void RaiseError(const std::string& _sError);
	/// clears log information about current state (TimeStart, TimeEnd, WindowNumber, etc.)
	void ClearLogState();

	/// Calculates and sets estimated values to initialize tear _streams up to the _tExtra time point, applying selected extrapolation method on the time interval [_t1, _t2].
	void ApplyExtrapolationMethod(const std::vector<CStream*>& _streams, double _t1, double _t2, double _tExtra) const;

	/// Setup chosen convergence method.
	void SetupConvergenceMethod();
	/// Applies selected convergence method to calculate new values _s3 using previous values _s2 and _s1 on the specified time interval.
	void ApplyConvergenceMethod(const std::vector<CStream*>& _s3, std::vector<CStream*>& _s2, std::vector<CStream*>& _s1, double _t1, double _t2);
	double PredictValues(double _d3, double _d2, double _d1) const;
	std::vector<double> PredictValues( const std::vector<double>& _v3, const std::vector<double>& _v2, const std::vector<double>& _v1) const;
	CDenseMDMatrix PredictValues(const CDenseMDMatrix& _m3, const CDenseMDMatrix& _m2, const CDenseMDMatrix& _m1) const;
	void PredictValues(size_t _len, const double* _v3, const double* _v2, const double* _v1, double* _res) const;
	double PredictRelaxation(double F2, double F1) const;
	double PredictWegstein(double F2, double F1, double X2, double X1) const;
	double PredictSteffensen(double F3, double F2, double F1) const;

	// Removes excessive data from streams of the selected partition on the time interval.
	void ReduceData(const CCalculationSequence::SPartition& _partition, double _t1, double _t2) const;

	static EPhase PhaseSOA2EPhase(unsigned _soa);
};
