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

// DatabaseHelper.cpp
// Class definition

// Project includes
#include "CallTracer.h"
#include "DatabaseHelper.h"
#include "MessageLogger.h"

// Qt includes
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor (never called)
DatabaseHelper::DatabaseHelper()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
DatabaseHelper::~DatabaseHelper()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



// ======================================================================= Misc



///////////////////////////////////////////////////////////////////////////////
// Check if query caused an error
bool DatabaseHelper::HasSQLError(QSqlQuery & mrQuery, const QString mcFilename,
    const int mcLineNumber)
{
    CALL_IN(QString("mrQuery=%1, mcFilename=%2, mcLineNumber=%3")
        .arg(CALL_SHOW(mrQuery),
             CALL_SHOW(mcFilename),
             CALL_SHOW(mcLineNumber)));

    // Check for error
    if (mrQuery.lastError().type() != 0)
    {
        const QString reason =
            tr("SQL Error in %1 (line %2): SQL reported an error during "
                "execution of \"%3\": %4")
                .arg(mcFilename,
                     QString::number(mcLineNumber),
                     mrQuery.lastQuery(),
                     mrQuery.lastError().text());

        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return true;
    }

    // No error
    CALL_OUT("");
    return false;
}
