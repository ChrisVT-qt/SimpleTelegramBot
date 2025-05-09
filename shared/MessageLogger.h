// MessageLogger.h
// Class definition file

/** \file
  * \todo Add Doxygen information
  */

// Just include once
#ifndef MESSAGELOGGER_H
#define MESSAGELOGGER_H

// Qt includes
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

// Class definition
class MessageLogger
    : public QObject
{
    Q_OBJECT
    
    
    
    // ============================================================== Lifecycle
private:
	// Default constructor
	MessageLogger();

public:
    // Get instance (singleton)
    static MessageLogger * Instance();
    
private:
    // Instance
    static MessageLogger * m_Instance;
    
public:
	// Destructor
	virtual ~MessageLogger();
    
        
    
    // ================================================================ Logging
public:
    // Add error line
    static void Error(const QString mcMethod, const QString mcReason);
    static void Error(const QString mcMethod, const QString mcNoRepeatTag,
        const QString mcReason);

    // Add message line
    static void Message(const QString mcMethod, const QString mcReason);
    static void Message(const QString mcMethod, const QString mcNoRepeatTag,
        const QString mcReason);
    
    // Add debug line
    static void Debug(const QString mcMethod, const QString mcReason);

    // Table
    static void Table(const QList < QString > mcTitles,
        const QList < QString > mcValues);

    // Add message line
    static void Print(const QString mcMessage);

private:
    // Remember what warning has already been shown
    static QSet < QString > m_NoRepeatTags;
};

#endif
