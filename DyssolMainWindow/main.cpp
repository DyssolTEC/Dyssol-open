/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Dyssol.h"
#include "FileSystem.h"
#include "DyssolStringConstants.h"
#include "DyssolSystemDefines.h"
#include <QDir>
#include <QDate>

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
			int i = 0;
		}
	}
	while (nExitCode == Dyssol::EXIT_CODE_REBOOT);

	return 0;
}
