// SimpleTelegramBot - a software organizing everyday tasks
// Copyright (C) 2025 Chris von Toerne
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contact the author by email: christian.vontoerne@gmail.com

// Application.cpp
// Class definition

// Project includes
#include "Application.h"
#include "CallTracer.h"
#include "MessageLogger.h"



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
Application::Application(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    QStringList arg_values;
    for (int index = 0; index < argc; index++)
    {
        arg_values << argv[index];
    }
    CALL_IN(QString("argc=%1, argv=%2")
        .arg(CALL_SHOW(argc),
             CALL_SHOW(arg_values)));

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
Application::~Application()
{
    CALL_IN("");

    // Nothing to do.
    CALL_OUT("");
}

   

///////////////////////////////////////////////////////////////////////////////
// Instanciator
Application * Application::Instance(int & argc, char ** argv)
{
    QStringList arg_values;
    for (int index = 0; index < argc; index++)
    {
        arg_values << argv[index];
    }
    CALL_IN(QString("argc=%1, argv=%2")
        .arg(CALL_SHOW(argc),
             CALL_SHOW(arg_values)));

    if (!m_Instance)
    {
        m_Instance = new Application(argc, argv);
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
Application * Application::Instance()
{
    CALL_IN("");

    if (!m_Instance)
    {
        // Error.
        const QString reason =
            tr("Trying to access uninitialized instance. Should not happen.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return nullptr;
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
Application * Application::m_Instance = nullptr;

