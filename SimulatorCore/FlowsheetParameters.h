/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include "H5Handler.h"

class CFlowsheetParameters
{
private:
	static const unsigned m_cnSaveVersion;

	/// Allows read-only public access outside the class, and full private access
	template <class T>
	class proxy
	{
		friend class CFlowsheetParameters;
	private:
		T data;
		T operator=(const T& arg) { data = arg; return data; }
	public:
		operator const T&() const { return data; }
	};

public:
	CFlowsheetParameters();

	void Initialize();

	/// Save parameters to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _sPath);
	/// Load parameters from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _sPath);

	// == Tolerances
	proxy<double> absTol;					// absolute tolerance
	void AbsTol(double val);
	proxy<double> relTol;					// relative tolerance
	void RelTol(double val);

	// == Fractions
	proxy<double> minFraction;				// minimal fraction that is taken into account in MDMatrix
	void MinFraction(double val);

	// == Time windows
	proxy<double> initTimeWindow;			// initial time window
	void InitTimeWindow(double val);
	proxy<double> minTimeWindow;			// minimal allowed time window
	void MinTimeWindow(double val);
	proxy<double> maxTimeWindow;			// maximal allowed time window
	void MaxTimeWindow(double val);
	proxy<unsigned> maxItersNumber;			// maximal allowed number of iteration for one time window
	void MaxItersNumber(unsigned val);
	proxy<unsigned> itersUpperLimit;		// upper limit of iterations after witch time window will be decreased
	void ItersUpperLimit(unsigned val);
	proxy<unsigned> itersLowerLimit;		// lower limit of iterations after witch time window will be increased
	void ItersLowerLimit(unsigned val);
	proxy<unsigned> iters1stUpperLimit;		// upper limit of iterations for the first time window after witch the time window will be decreased
	void Iters1stUpperLimit(unsigned val);
	proxy<double> magnificationRatio;		// factor for increasing of time window size
	void MagnificationRatio(double val);

	// == Convergence methods
	proxy<EConvMethod> convergenceMethod;	// method for prediction of tear streams' values
	void ConvergenceMethod(EConvMethod val);
	proxy<double> wegsteinAccelParam;		// limit of acceleration parameter of Wegstein's convergence method
	void WegsteinAccelParam(double val);
	proxy<double> relaxationParam;			// relaxation parameter of Direct substitution with relaxation convergence method
	void RelaxationParam(double val);

	// == Extrapolation methods
	proxy<EExtrapMethod> extrapolationMethod;	// method for data extrapolation on each next time window
	void ExtrapolationMethod(EExtrapMethod val);

	// == Compression
	proxy<double> saveTimeStep;				// saving time step hint
	void SaveTimeStep(double val);
	proxy<bool> saveTimeStepFlagHoldups;	// whether to apply saving time step also to holdups
	void SaveTimeStepFlagHoldups(bool val);

	// == Caching
	proxy<std::wstring> cachePath;
	void CachePath(std::wstring val);
	proxy<bool> cacheFlagStreams;
	void CacheFlagStreams(bool val);
	proxy<bool> cacheFlagHoldups;
	void CacheFlagHoldups(bool val);
	proxy<bool> cacheFlagInternal;
	void CacheFlagInternal(bool val);
	proxy<bool> cacheFlagStreamsAfterReload;
	void CacheFlagStreamsAfterReload(bool val);
	proxy<bool> cacheFlagHoldupsAfterReload;
	void CacheFlagHoldupsAfterReload(bool val);
	proxy<bool> cacheFlagInternalAfterReload;
	void CacheFlagInternalAfterReload(bool val);
	proxy<unsigned> cacheWindow;
	void CacheWindow(unsigned val);
	proxy<unsigned> cacheWindowAfterReload;
	void CacheWindowAfterReload(unsigned val);

	// == File saving
	proxy<bool> fileSingleFlag;		// true - single file, false - file is split on subfiles with MAX_FILE_SIZE size
	void FileSingleFlag(bool val);

	// == Initialization of tear streams
	proxy<bool> initializeTearStreamsAutoFlag;	// true - automatically calculate initialization values using previous calculations, false - user defined initial values
	void InitializeTearStreamsAutoFlag(bool val);
};

