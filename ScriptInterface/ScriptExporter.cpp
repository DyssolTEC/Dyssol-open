/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptExporter.h"
#include "ScriptJob.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "MaterialsDatabase.h"
#include <fstream>
#include <sstream>

using namespace ScriptInterface;
namespace fs = std::filesystem;

CScriptExporter::CScriptExporter(const CFlowsheet& _flowsheet, const CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB)
	: m_flowsheet{ _flowsheet }
	, m_modelsManager{ _modelsManager }
	, m_materialsDB{ _materialsDB }
{
}

bool CScriptExporter::Export(const fs::path& _scriptFile, const fs::path& _flowsheetFile)
{
	std::ofstream file(_scriptFile);
	if (!file) return false;

	// fill job
	CScriptJob job;
	for (const auto& e : allScriptArguments)
	{
		// use switch to let the compiler catch missing script entries
		switch (e.key)
		{
		case EScriptKeys::JOB:
		{
			job.AddEntry(e.keyStr);
			break;
		}
		case EScriptKeys::SOURCE_FILE:
		{
			auto entry = _flowsheetFile;
			job.AddEntry(e.keyStr)->value = fs::absolute(entry.make_preferred());
			break;
		}
		case EScriptKeys::RESULT_FILE:
		{
			auto entry = _flowsheetFile.parent_path() /= _flowsheetFile.stem() += fs::path{ "_res" } += _flowsheetFile.extension();
			job.AddEntry(e.keyStr)->value = fs::absolute(entry.make_preferred());
			break;
		}
		case EScriptKeys::MATERIALS_DATABASE:
		{
			auto entry = m_materialsDB.GetFileName();
			job.AddEntry(e.keyStr)->value = fs::absolute(entry.make_preferred());
			break;
		}
		case EScriptKeys::MODELS_PATH:
		{
			for (const auto& entry : m_modelsManager.GetAllActiveDirPaths())
				job.AddEntry(e.keyStr)->value = fs::absolute(entry);
			break;
		}
		case EScriptKeys::SIMULATION_TIME:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->endSimulationTime;
			break;
		}
		case EScriptKeys::RELATIVE_TOLERANCE:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->relTol;
			break;
		}
		case EScriptKeys::ABSOLUTE_TOLERANCE:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->absTol;
			break;
		}
		case EScriptKeys::MINIMAL_FRACTION:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->minFraction;
			break;
		}
		case EScriptKeys::SAVE_TIME_STEP_HINT:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->saveTimeStep;
			break;
		}
		case EScriptKeys::SAVE_FLAG_FOR_HOLDUPS:
		{
			job.AddEntry(e.keyStr)->value = (bool)m_flowsheet.GetParameters()->saveTimeStepFlagHoldups;
			break;
		}
		case EScriptKeys::THERMO_TEMPERATURE_MIN:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->enthalpyMinT;
			break;
		}
		case EScriptKeys::THERMO_TEMPERATURE_MAX:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->enthalpyMaxT;
			break;
		}
		case EScriptKeys::THERMO_TEMPERATURE_INTERVALS:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->enthalpyInt;
			break;
		}
		case EScriptKeys::INIT_TIME_WINDOW:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->initTimeWindow;
			break;
		}
		case EScriptKeys::MIN_TIME_WINDOW:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->minTimeWindow;
			break;
		}
		case EScriptKeys::MAX_TIME_WINDOW:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->maxTimeWindow;
			break;
		}
		case EScriptKeys::MAX_ITERATIONS_NUMBER:
		{
			job.AddEntry(e.keyStr)->value = (uint64_t)m_flowsheet.GetParameters()->maxItersNumber;
			break;
		}
		case EScriptKeys::WINDOW_CHANGE_RATE:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->magnificationRatio;
			break;
		}
		case EScriptKeys::ITERATIONS_UPPER_LIMIT:
		{
			job.AddEntry(e.keyStr)->value = (uint64_t)m_flowsheet.GetParameters()->itersUpperLimit;
			break;
		}
		case EScriptKeys::ITERATIONS_LOWER_LIMIT:
		{
			job.AddEntry(e.keyStr)->value = (uint64_t)m_flowsheet.GetParameters()->itersLowerLimit;
			break;
		}
		case EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST:
		{
			job.AddEntry(e.keyStr)->value = (uint64_t)m_flowsheet.GetParameters()->iters1stUpperLimit;
			break;
		}
		case EScriptKeys::CONVERGENCE_METHOD:
		{
			job.AddEntry(e.keyStr)->value = SNamedEnum{ static_cast<EConvergenceMethod>(m_flowsheet.GetParameters()->convergenceMethod) };
			break;
		}
		case EScriptKeys::RELAXATION_PARAMETER:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->relaxationParam;
			break;
		}
		case EScriptKeys::ACCELERATION_LIMIT:
		{
			job.AddEntry(e.keyStr)->value = m_flowsheet.GetParameters()->wegsteinAccelParam;
			break;
		}
		case EScriptKeys::EXTRAPOLATION_METHOD:
		{
			job.AddEntry(e.keyStr)->value = SNamedEnum{ static_cast<EExtrapolationMethod>(m_flowsheet.GetParameters()->extrapolationMethod) };
			break;
		}
		case EScriptKeys::COMPOUNDS:
		{
			job.AddEntry(e.keyStr)->value = m_materialsDB.GetCompoundsNames(m_flowsheet.GetCompounds());
			break;
		}
		case EScriptKeys::PHASES:
		{
			SPhasesSE phases;
			for (const auto& p : m_flowsheet.GetPhases())
			{
				phases.names.push_back(p.name);
				phases.types.emplace_back(p.state);
			}
			job.AddEntry(e.keyStr)->value = phases;
			break;
		}
		case EScriptKeys::KEEP_EXISTING_GRIDS_VALUES:
		{
			job.AddEntry(e.keyStr)->value = false;
			break;
		}
		case EScriptKeys::DISTRIBUTION_GRID:
		{
			const auto AddGrid = [&](const CGridDimension& _d, const std::string& _unit)
			{
				SGridDimensionSE entry;
				entry.unit.name = _unit;
				entry.distrType = SNamedEnum{ _d.DimensionType() };
				entry.entryType = SNamedEnum{ _d.GridType() };
				entry.function = SNamedEnum{ EDistrFunction::Manual };
				entry.psdMeans = SNamedEnum{ EPSDGridType::DIAMETER };
				entry.classes = _d.ClassesNumber();
				if (_d.GridType() == EGridEntry::GRID_NUMERIC)
					entry.valuesNum = dynamic_cast<const CGridDimensionNumeric&>(_d).Grid();
				else if (_d.GridType() == EGridEntry::GRID_SYMBOLIC)
					entry.valuesSym = dynamic_cast<const CGridDimensionSymbolic&>(_d).Grid();
				job.AddEntry(e.keyStr)->value = entry;
			};

			for (const auto& d : m_flowsheet.GetGrid().GetGridDimensions())
				if (d->DimensionType() != DISTR_COMPOUNDS)
					AddGrid(*d, "GLOBAL");
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel() || u->GetModel()->GetGrid() == m_flowsheet.GetGrid()) continue;
				for (const auto& d : u->GetModel()->GetGrid().GetGridDimensions())
					if (d->DimensionType() != DISTR_COMPOUNDS)
						AddGrid(*d, u->GetName());
			}
			break;
		}
		case EScriptKeys::KEEP_EXISTING_UNITS:
		{
			job.AddEntry(e.keyStr)->value = false;
			break;
		}
		case EScriptKeys::UNIT:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				job.AddEntry(e.keyStr)->value = std::vector<std::string>{ u->GetName(), u->GetModel()->GetUnitName() };
			}
			break;
		}
		case EScriptKeys::STREAM:
		{
			for (const auto& c : m_flowsheet.GenerateConnectionsDescription())
			{
				SStreamSE entry;
				entry.name = m_flowsheet.GetStream(c.stream)->GetName();
				entry.unitO.name = m_flowsheet.GetUnit(c.unitO)->GetName();
				entry.unitI.name = m_flowsheet.GetUnit(c.unitI)->GetName();
				entry.portO.name = c.portO;
				entry.portI.name = c.portI;
				job.AddEntry(e.keyStr)->value = entry;
			}
			break;
		}
		case EScriptKeys::UNIT_PARAMETER:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				for (const auto& up : u->GetModel()->GetUnitParametersManager().GetParameters())
				{
					SUnitParameterSE entry;
					entry.unit.name = u->GetName();
					entry.param.name = up->GetName();
					std::stringstream ss;
					up->ValueToStream(ss);
					entry.values = ss.str();
					job.AddEntry(e.keyStr)->value = entry;
				}
			}
			break;
		}
		case EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES:
		{
			job.AddEntry(e.keyStr)->value = false;
			break;
		}
		case EScriptKeys::HOLDUP_OVERALL:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				for (const auto& s : u->GetModel()->GetStreamsManager().GetAllInit())
				{
					SHoldupDependentSE entry;
					entry.unit.name = u->GetName();
					entry.holdup.name = s->GetName();
					for (const auto& t : s->GetAllTimePoints())
					{
						entry.values.push_back(t);
						for (const auto& p : m_flowsheet.GetOveralProperties())
							entry.values.push_back(s->GetOverallProperty(t, p.type));
					}
					job.AddEntry(e.keyStr)->value = entry;
				}
			}
			break;
		}
		case EScriptKeys::HOLDUP_PHASES:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				for (const auto& s : u->GetModel()->GetStreamsManager().GetAllInit())
				{
					SHoldupDependentSE entry;
					entry.unit.name = u->GetName();
					entry.holdup.name = s->GetName();
					for (const auto& t : s->GetAllTimePoints())
					{
						entry.values.push_back(t);
						for (const auto& p : m_flowsheet.GetPhases())
							entry.values.push_back(s->GetPhaseFraction(t, p.state));
					}
					job.AddEntry(e.keyStr)->value = entry;
				}
			}
			break;
		}
		case EScriptKeys::HOLDUP_COMPOUNDS:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				for (const auto& s : u->GetModel()->GetStreamsManager().GetAllInit())
					for (const auto& p : m_flowsheet.GetPhases())
					{
						SHoldupCompoundsSE entry;
						entry.unit.name = u->GetName();
						entry.holdup.name = s->GetName();
						entry.phase = SNamedEnum{ p.state };
						for (const auto& t : s->GetAllTimePoints())
						{
							entry.values.push_back(t);
							for (const auto& c : s->GetAllCompounds())
								entry.values.push_back(s->GetCompoundFraction(t, c, p.state));
						}
						job.AddEntry(e.keyStr)->value = entry;
					}
			}
			break;
		}
		case EScriptKeys::HOLDUP_DISTRIBUTION:
		{
			for (const auto& u : m_flowsheet.GetAllUnits())
			{
				if (!u->GetModel()) continue;
				for (const auto& s : u->GetModel()->GetStreamsManager().GetAllInit())
					for (const auto& d : s->GetGrid().GetDimensionsTypes())
					{
						if (d == DISTR_COMPOUNDS) continue;
						for (const auto& c : s->GetAllCompounds())
						{
							SHoldupDistributionSE entry;
							entry.unit.name = u->GetName();
							entry.holdup.name = s->GetName();
							entry.distrType = SNamedEnum{ d };
							entry.compound = m_materialsDB.GetCompound(c) ? m_materialsDB.GetCompound(c)->GetName() : c;
							entry.psdType = SNamedEnum{ EPSDTypes::PSD_MassFrac };
							entry.psdMeans = SNamedEnum{ EPSDGridType::DIAMETER };
							entry.function = SNamedEnum{ EGridFunction::GRID_FUN_MANUAL };
							for (const auto& t : s->GetAllTimePoints())
							{
								entry.values.push_back(t);
								for (const auto& v : s->GetDistribution(t, d, c))
									entry.values.push_back(v);
							}
							job.AddEntry(e.keyStr)->value = entry;
						}
					}
			}
			break;
		}
		case EScriptKeys::EXPORT_FILE:
		case EScriptKeys::EXPORT_PRECISION:
		case EScriptKeys::EXPORT_FIXED_POINT:
		case EScriptKeys::EXPORT_SIGNIFICANCE_LIMIT:
		case EScriptKeys::EXPORT_ONLY:
		case EScriptKeys::EXPORT_STREAM_MASS:
		case EScriptKeys::EXPORT_STREAM_TEMPERATURE:
		case EScriptKeys::EXPORT_STREAM_PRESSURE:
		case EScriptKeys::EXPORT_STREAM_OVERALLS:
		case EScriptKeys::EXPORT_STREAM_PHASES_FRACTIONS:
		case EScriptKeys::EXPORT_STREAM_COMPOUNDS_FRACTIONS:
		case EScriptKeys::EXPORT_STREAM_PSD:
		case EScriptKeys::EXPORT_STREAM_DISTRIBUTIONS:
		case EScriptKeys::EXPORT_HOLDUP_MASS:
		case EScriptKeys::EXPORT_HOLDUP_TEMPERATURE:
		case EScriptKeys::EXPORT_HOLDUP_PRESSURE:
		case EScriptKeys::EXPORT_HOLDUP_OVERALLS:
		case EScriptKeys::EXPORT_HOLDUP_PHASES_FRACTIONS:
		case EScriptKeys::EXPORT_HOLDUP_COMPOUNDS_FRACTIONS:
		case EScriptKeys::EXPORT_HOLDUP_PSD:
		case EScriptKeys::EXPORT_HOLDUP_DISTRIBUTIONS:
		case EScriptKeys::EXPORT_UNIT_STATE_VARIABLE:
		case EScriptKeys::EXPORT_UNIT_PLOT:
		case EScriptKeys::EXPORT_FLOWSHEET_GRAPH:
			break;
		}
	}

	// export the job to the file
	for (const auto& e : job.GetAllEntries())
		WriteScriptEntry(*e, file);

	file.close();

	return true;
}
