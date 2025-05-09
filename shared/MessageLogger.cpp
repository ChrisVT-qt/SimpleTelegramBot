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

// MessageLogger.cpp
// Class implementation file

// Project includes
#include "CallTracer.h"
#include "MessageLogger.h"

// Qt includes
#include <QDebug>



// MessageLogger class does not do CALL_IN/CALL_OUT



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Default constructor
MessageLogger::MessageLogger()
{
    // Nothing to do.
}



///////////////////////////////////////////////////////////////////////////////
// Get instance (singleton)
MessageLogger * MessageLogger::Instance()
{
    // Check if we have an instance
    if (!m_Instance)
    {
        // No. Create one.
        m_Instance = new MessageLogger();
    }

    // Return instance
    return m_Instance;
}



///////////////////////////////////////////////////////////////////////////////
// Instance
MessageLogger * MessageLogger::m_Instance = nullptr;



///////////////////////////////////////////////////////////////////////////////
// Destructor
MessageLogger::~MessageLogger()
{
    // Nothing to do.
}



// ==================================================================== Logging



///////////////////////////////////////////////////////////////////////////////
// Add error line
void MessageLogger::Error(const QString mcMethod, const QString mcReason)
{
    // Dump to console
    qDebug().noquote() << tr("ERROR: %1:\n\t%2")
        .arg(mcMethod,
             mcReason);
    qDebug().noquote() << tr("Callback stack:\n%1")
        .arg(CALL_STACK());
}



///////////////////////////////////////////////////////////////////////////////
// Add error line (avoid repetitions)
void MessageLogger::Error(const QString mcMethod, const QString mcNoRepeatTag,
    const QString mcReason)
{
    // Check if this is a repetition
    if (m_NoRepeatTags.contains(mcNoRepeatTag))
    {
        // Yup. Ignore.
        return;
    }
    
    // This will be a repetition next time around
    m_NoRepeatTags += mcNoRepeatTag;
    
    // Use normal method
    Error(mcMethod, mcReason);
}



///////////////////////////////////////////////////////////////////////////////
// Add message line
void MessageLogger::Message(const QString mcMethod, const QString mcReason)
{
    qDebug().noquote() << QString("%1:\n\t%2")
        .arg(mcMethod,
             mcReason);
}



///////////////////////////////////////////////////////////////////////////////
// Add message line
void MessageLogger::Message(const QString mcMethod,
    const QString mcNoRepeatTag, const QString mcReason)
{
    // Check if this is a repetition
    if (m_NoRepeatTags.contains(mcNoRepeatTag))
    {
        // Yup. Ignore.
        return;
    }
    
    // This will be a repetition next time around
    m_NoRepeatTags += mcNoRepeatTag;
    
    // Use normal method
    Message(mcMethod, mcReason);
}



///////////////////////////////////////////////////////////////////////////////
// Add debug line
void MessageLogger::Debug(const QString mcMethod, const QString mcReason)
{
    qDebug().noquote() << tr("DEBUG: %1: %2")
        .arg(mcMethod,
             mcReason);
}



///////////////////////////////////////////////////////////////////////////////
// Table
void MessageLogger::Table(const QList < QString > mcTitles,
    const QList < QString > mcValues)
{
    // Check parameters
    if (!mcTitles.isEmpty() &&
        mcTitles.size() != mcValues.size())
    {
        const QString reason = tr("Incompatible count in titles argument.");
        Error(CALL_METHOD, reason);
        return;
    }
    
    // Dump to console
    for (int idx = 0; idx < mcValues.size(); idx++)
    {
        if (mcTitles.size() > idx)
        {
            qDebug().noquote() << QString("%1: %2")
                .arg(mcTitles[idx],
                     mcValues[idx]);
        } else
        {
            qDebug().noquote() << QString("%1")
                .arg(mcValues[idx]);
        }
    }
    qDebug().noquote() << "";
}



///////////////////////////////////////////////////////////////////////////////
// Add message line
void MessageLogger::Print(const QString mcMessage)
{
    qDebug().noquote() << mcMessage;
}



///////////////////////////////////////////////////////////////////////////////
// Remember what warning has already been shown
QSet < QString > MessageLogger::m_NoRepeatTags = QSet < QString >();
