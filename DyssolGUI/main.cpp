/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Dyssol.h"
#include <QApplication>
#include <QStyleFactory>

#ifdef _MSC_VER
// Print message to the VS Output window.
void VSDebugOutput(const std::string& _message)
{
	OutputDebugStringA(_message.c_str());
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
#ifdef _MSC_VER
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

#ifdef _MSC_VER
		// force Qt6 to look in ./styles folder for style libraries
		QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/styles");

		const QStringList available = QStyleFactory::keys();
		if (available.contains("windowsvista", Qt::CaseInsensitive))
			QApplication::setStyle("windowsvista");
		else if (available.contains("fusion", Qt::CaseInsensitive))
			QApplication::setStyle("fusion");
		else if (available.contains("windows11", Qt::CaseInsensitive))
			QApplication::setStyle("windows11");
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
		qRegisterMetaTypeStreamOperators<QList<bool>>("QList<bool>");
#endif

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
