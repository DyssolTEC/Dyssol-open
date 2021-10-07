/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ArgumentsParser.h"
#include "ScriptParser.h"
#include "ScriptRunner.h"
#include "ThreadPool.h"
#include "DyssolSystemDefines.h"

// Prints information about command line arguments.
void PrintArgumentsInfo(const CArgumentsParser& _parser)
{
	std::cout << "Usage: DyssolC --key[=value] [--key[=value]] [...]" << std::endl;
	for (const auto& k : _parser.AllAllowedKeys())
	{
		for (const auto& s : k.keysS)
			std::cout << "-" << s << " ";
		std::cout << '\t';
		for (const auto& l : k.keysL)
			std::cout << "--" << l << " ";
		std::cout << k.description << std::endl;
	}
}

// Prints information about the current version.
void PrintVersionInfo()
{
	std::cout << "Version: " << CURRENT_VERSION_STR << std::endl;
}

// Prints information about available models.
void PrintModelsInfo(const std::vector<std::filesystem::path>& _modelPaths)
{
	// Helper function to print a formatted entry justified left and filled with spaces until the given length.
	constexpr auto PrintUnitEntry = [](const auto& _entry, std::streamsize _width)
	{
		std::cout << std::left << std::setw(_width) << std::setfill(' ') << _entry;
	};

	// create models manager
	CModelsManager manager;
	manager.AddDir(L".");				// add current directory as a path to units/solvers
	for (const auto& dir : _modelPaths)	// add other directories as a path to units/solvers
		manager.AddDir(dir);

	// get available models
	const auto units   = manager.GetAvailableUnits();
	const auto solvers = manager.GetAvailableSolvers();

	// print info
	constexpr size_t nameLen = 50;
	constexpr size_t typeLen = 30;
	constexpr size_t authLen = 50;
	PrintUnitEntry("Name"  , nameLen);
	PrintUnitEntry("Type"  , typeLen);
	PrintUnitEntry("Author", authLen);
	std::cout << std::endl;
	for (const auto& m : units)
	{
		PrintUnitEntry(m.name, nameLen);
		PrintUnitEntry(m.isDynamic ? StrConst::MMT_Dynamic : StrConst::MMT_SteadyState, typeLen);
		PrintUnitEntry(m.author, authLen);
		std::cout << std::endl;
	}
	for (const auto& m : solvers)
	{
		PrintUnitEntry(m.name, nameLen);
		PrintUnitEntry(std::vector<std::string>{ SOLVERS_TYPE_NAMES }[static_cast<unsigned>(m.solverType)] + " " + StrConst::MMT_Solver, typeLen);
		PrintUnitEntry(m.author, authLen);
		std::cout << std::endl;
	}
}

void RunDyssol(const std::filesystem::path& _script)
{
	InitializeThreadPool();

	std::cout << "Parsing script file: \n\t" << _script.string() << std::endl;

	const CScriptParser parser{ _script };
	std::cout << "Jobs found: \n\t" << parser.JobsCount() << std::endl;

	CScriptRunner runner;
	size_t counter = 0;
	for (const auto& job : parser.Jobs())
	{
		std::cout << " ===== Starting job: " << counter++ + 1 << " ===== " << std::endl;
		runner.RunJob(*job);
	}
}

int main(int argc, const char* argv[])
{
	try
	{
		// possible keys with aliases and descriptions
		const std::vector<CArgumentsParser::SKey> keys{
			{ { "script"      }, { "s"  }, { "path to script file"                          } },
			{ { "version"     }, { "v"  }, { "print information about current version"      } },
			{ { "models"      }, { "m"  }, { "print information about available models"     } },
			{ { "models_path" }, { "mp" }, { "additional path to look for available models" } },
			{ { "help"        }, { "h"  }, { "give this help list"                          } },
		};

		const CArgumentsParser parser(argc, argv, keys);

		if (parser.TokensCount() == 0 || parser.HasKey("h"))
		{
			PrintArgumentsInfo(parser);
			return 1;
		}

		if (parser.HasKey("v"))
			PrintVersionInfo();
		if (parser.HasKey("m"))
		{
			std::vector<std::filesystem::path> fsPaths;
			for (const auto& p : parser.GetValues("mp"))
				fsPaths.emplace_back(p);
			PrintModelsInfo(fsPaths);
		}
		if (parser.HasKey("s"))
			RunDyssol(parser.GetValue("s"));
	}
	catch (const std::exception& e)
	{
		std::cout << "Unhandled exception caught: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Unknown unhandled exception caught" << std::endl;
	}
	return 0;
}
