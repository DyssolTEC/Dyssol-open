/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Dyssol.h"
#include "FileSystem.h"

#ifdef _WIN32
// Print message to the VS Output window.
void VSDebugOutput(const std::string_view _message)
{
	std::ostringstream os;
	os << _message;
	OutputDebugStringA(os.str().c_str());
}
#endif

void HandleException(const std::exception_ptr& _exceptionPtr)
{
	try
	{
		if (_exceptionPtr)
			std::rethrow_exception(_exceptionPtr);
	}
	catch (const std::exception& e)
	{
		const std::string message = "Unknown unhandled exception caught: '" + std::string{ e.what() } + "'\n";
		std::cout << message;
#ifdef _WIN32
		VSDebugOutput(message);
#endif
	}
}

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(Resources);

	int nExitCode = 0;

	do
	{
		QApplication a(argc, argv);
		QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
		qRegisterMetaTypeStreamOperators<QList<bool>>("QList<bool>");
		QStringList args = a.arguments();

		for (int i = 0; i < args.size(); ++i)
			std::cout << args[i].toStdString() << std::endl;

		Dyssol w;

		std::exception_ptr exceptionPtr;
		try
		{
			w.InitializeConnections();
			w.show();
			if (args.size() == 2)
				w.OpenFromCommandLine(args[1]);
			else
				w.OpenDyssol();
			nExitCode = a.exec();
		}
		catch(...)
		{
			exceptionPtr = std::current_exception(); // capture current exception
		}
		HandleException(exceptionPtr);
	}
	while (nExitCode == Dyssol::EXIT_CODE_REBOOT);

	return 0;
}
