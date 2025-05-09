// CallTracer.h
// Class definition

/** \class CallTracer
  * Used for keeping track of methods and functions being called while the
  * program is running, to be used as a call stack for debugging purposes.
  *
  * This class is at this point not designed to be thread-safe, but it should
  * be rather straight forward, and it's on the list of future improvements.
  *
  * Users of this class would use it mostly through the macros defined below,
  * using \link CALL_IN()\endlink as the first thing when entering a function,
  * and using \link CALL_OUT()\endlink as the last thing. These macros always
  * come in pairs, and any exit point of a function needs to invoke
  * \link CALL_OUT()\endlink.
  *
  * Within \link CALL_IN()\endlink, use the \link CALL_CLASS()\endlink and
  * \link CALL_METHOD()\endlink macros to get representations of the current
  * class
  * name and method name within the class. The \link CALL_SHOW()\endlink macro
  * is very
  * useful to obtain human readable representation of the content of a call
  * parameter. It's a heavily overloaded function.
  *
  * Finally, \link CALL_STACK()\endlink allows you to dump the current call
  * stack.
  *
  * This class also counts calls to methods/functions and the methods/functions
  * calling them; see \link ShowUsage()\endlink and
  * \link ShowCallOriginators()\endlink functions.
  *
  * Functionality can be turned off by setting \c DEPLOY to \c true in
  * Deploy.h
  */

#ifndef CALLTRACER_H
#define CALLTRACER_H

// Qt includes
#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QString>


// The following a luckily documented in
// https://lists.qt-project.org/pipermail/interest/2015-January/014617.html

// Only relevant if XML module is used
#ifdef QT_XML_LIB
#include <QDomDocument>
#endif

// Only relevant if SQL module is used
#ifdef QT_SQL_LIB
#include <QSqlQuery>
#endif

// Macros to use
#include "Deploy.h"
#if DEPLOY
    #define CALL_CLASS QString()
    #define CALL_METHOD QString()
    #define CALL_IN(p) {}
    #define CALL_OUT(p) {}
    #define CALL_STACK() QString()
    #define CALL_SHOW(p) QString()
    #define CALL_TIMESTAMP QString()
#else
    /** \brief Generates class name from filename
     */
    #define CALL_CLASS CallTracer::ClassName(__FILE__)

    /** \brief Generates name of the method where ever it is called
     */
    #define CALL_METHOD (CALL_CLASS + "::" + __func__)

    /** \brief Saves information when entering a function or method
     */
    #define CALL_IN(p) CallTracer::EnterFunction(__FILE__, __func__, p)

    /** \brief Saves information when exiting a function or method
     */
    #define CALL_OUT(p) CallTracer::ExitFunction(__FILE__, __func__, __LINE__, p)

    /** \brief The entire call stack or call history
     */
    #define CALL_STACK() CallTracer::GetCallTrace()

    /** \brief Converts variable to human readable form
     */
    #define CALL_SHOW(p) CallTracer::Show(p)

    /** \brief Converts variable to human readable form
     */
    #define CALL_SHOW_FULL(p) CallTracer::Show(p, false)

    /** \brief Date and time for time stamping
     */
    #define CALL_TIMESTAMP \
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
#endif

// Define class
class CallTracer
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
private:
    /** \brief Constructor - this class will never be instaciated.
      */
    // Constructor
    CallTracer();

public:
    /** \brief Destructor
      */
    // Destructor
    ~CallTracer();



    // ============================================================= Call Stack
public:
    /** \brief Forget the entire call history up to this point.
      * Useful while recording the entire call history and you're only
      * interested from a particular point onward.
      */
    static void ResetHistory();

    /** \brief Start or stop keeping the entire call history.
      * By default, only the call stack is maintained, not the full history.
      * \param mcKeepHistory \c true if you want to keep the full history,
      * \c false if you don't.
      */
    static void SetKeepAllHistory(const bool mcKeepHistory);

    /** \brief Records when a function is entered.
      * \param mcFilename Name of the source code file; \c __FILE__
      * is a good choice here
      * \param mcFunction Name of the function/method being entered. Ususally,
      * use \c __func__.
      * \param mcParameters List of parameters and their values. Best practice
      * if to use \c CALL_SHOW() to show the parameter value
      */
    static void EnterFunction(const QString mcFilename,
        const QString mcFunction, const QString mcParameters);

    /** \brief Records when a function is exited.
      * \param mcFilename Name of the source code file; \c __FILE__
      * macro is a good choice here
      * \param mcFunction Name of the function/method being entered. Ususally,
      * use \c __func__ macro.
      * \param mcLine Line in the source file for pinpointing location of the
      * exit point. Usually, use \c __LINE__ macro.
      * \param mcReason The reason why the function/method was left. This is
      * helpful to track error handling. Use an empty string for normal exit.
      */
    static void ExitFunction(const QString mcFilename,
        const QString mcFunction, const int mcLine, const QString mcReason);

    /** \brief Returns the call trace.
      * Depending on your choices in \link SetKeepAllHistory()\endlink, this
      * is either the call stack or the full history.
      * \returns A single string with the call trace.
      */
    static QString GetCallTrace();

    /** \brief Extract the class name from a (source) filename
      * \param mcFilename The name of the source file to be used to extract
      * the class name; we assume were that the name of the file (plus
      * extensions .h and .cpp) is the same as the class. It's a good idea
      * to use the \c __FILE__ macro as the argument, or to use
      * the \c CALL_CLASS macro.
      */
    static QString ClassName(const QString mcFilename);

private:
    /** \brief Time stamp when a function/method was called.
      */
    static QList < QString > m_CallStack_Time;
    /** \brief Name of the method that had been called.
      */
    static QList < QString > m_CallStack_Method;
    /** \brief Text (usually the list of arguments and their content) for the
      * call of the function/method
      */
    static QList < QString > m_CallStack_Text;

    /** \brief Flag indicating if we want to keep the full call history or just
      * the call stack.
      * Set it with \link SetKeepAllHistory()\endlink.
      */
    static bool m_KeepAllHistory;



    // =========================================================== Method usage
public:
    /** \brief Reset usage statistics
      * \param mcClass indicates the class for which statistics should be reset
      * \param mcMethod indicates the method for which statistics should be
      * reset
      */
    static void ResetUsage(const QString mcClass = QString(),
        const QString mcMethod = QString());

    /** \brief Show usage statistics
      * \param mcClass indicates the class for which statistics should be shown
      * \param mcMethod indicates the method for which statistics should be
      * show
      */
    static void ShowUsage(const QString mcClass = QString(),
        const QString mcMethod = QString());

    /** \brief Show who called a particular method
      * \param mcClass indicates the class
      * \param mcMethod indicates the method
      */
    static void ShowCallOriginators(const QString mcClass,
        const QString mcMethod);

private:
    /** \brief Counts what class and method is called how often
      * Maps class + method to call count.
      */
    static QHash < QString, QHash < QString, int > > m_CallCount;
    /** \brief Counts what method is called by what other method, and how often
      * maps called method + calling method to call count.
      */
    static QHash < QString, QHash < QString, int > > m_OriginatorCount;

public:
    /** \brief Set verbosity of operations
      * \param mcNewVerbosity new value; \c true for verbose operations,
      * \c false for quiet operations.
      */
    static void SetVerbosity(const bool mcNewVerbosity);
private:
    /** \brief Verbosity
      */
    static bool m_IsVerbose;



    // ============================================================ Convenience
public:
    // Show different objects
    /** \brief Show bool value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const bool mcValue);

    /** \brief Show int value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const int mcValue);

    /** \brief Show double value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const double mcValue);

    /** \brief Show long double value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const long double mcValue);

    /** \brief Show QByteArray value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QByteArray & mcrValue);

    /** \brief Show QColor value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QColor & mcrValue);

    /** \brief Show QDate value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QDate & mcrValue);

    /** \brief Show QDateTime value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QDateTime & mcrValue);

#ifdef QT_XML_LIB
    /** \brief Show QDomDocument value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QDomDocument & mcrValue);

    /** \brief Show QDomElement value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QDomElement & mcrValue);

    /** \brief Show QDomNode value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QDomNode & mcrValue);
#endif

    /** \brief Show QHash < double, double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < double, double > & mcrValue);

    /** \brief Show QHash < int, double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < int, double > & mcrValue);

    /** \brief Show QHash < int, int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < int, int > & mcrValue);

    /** \brief Show QHash < int , QDateTime > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < int, QDateTime > & mcrValue);

    /** \brief Show QHash < int, QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < int, QString > & mcrValue);

    /** \brief Show QHash < QString, int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < QString, int > & mcrValue);

    /** \brief Show QHash < QString, QList < double > > value in human
      * readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < QString, QList < double > > & mcrValue);

    /** \brief Show QHash < QString, QList < QString > > value in human
      * readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < QString, QList < QString > > & mcrValue);

    /** \brief Show QHash < QString, QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QHash < QString, QString > & mcrValue);

    /** \brief Show QImage value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QImage & mcrValue);

    /** \brief Show int64 value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const qint64 mcValue);

    /** \brief Show JSON array in human readable form.
      * \param mcrValue The value to show
      * \param mcTruncate If the text will be truncated to 20 characters
      * or shown in its entirety
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QJsonArray & mcrValue,
        const bool mcTruncate = true);

    /** \brief Show JSON object in human readable form.
      * \param mcrValue The value to show
      * \param mcTruncate If the text will be truncated to 20 characters
      * or shown in its entirety
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QJsonObject & mcrValue,
        const bool mcTruncate = true);

    /** \brief Show JSON object in human readable form.
      * \param mcrValue The value to show
      * \param mcTruncate If the text will be truncated to 20 characters
      * or shown in its entirety
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QJsonValue & mcrValue,
        const bool mcTruncate = true);

    /** \brief Show QList < int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QList < int > & mcrValue);

    /** \brief Show QList < qint64 > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QList < qint64 > & mcrValue);

    /** \brief Show QList < QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QList < QString > & mcrValue);

    /** \brief Show QList < QStringList > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QList < QStringList > & mcrValue);

    /** \brief Show QPair < double, double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < double, double > & mcrValue);

    /** \brief Show QPair < int, double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < int, double > & mcrValue);

    /** \brief Show QPair < int, int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < int, int > & mcrValue);

    /** \brief Show QPair < int, QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < int, QString > & mcrValue);

    /** \brief Show QPair < QString, int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < QString, int > & mcrValue);

    /** \brief Show QPair < QString, QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPair < QString, QString > & mcrValue);

    /** \brief Show QPixmap value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QPixmap & mcrValue);

    /** \brief Show QSet < double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QSet < double > & mcrValue);

    /** \brief Show QSet < int > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QSet < int > & mcrValue);

    /** \brief Show QSet < QString > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QSet < QString > & mcrValue);

#ifdef QT_SQL_LIB
    /** \brief Show QSqlQuery value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QSqlQuery & mcrValue);
#endif

    /** \brief Show QString value in human readable form.
      * \param mcValue The value to show
      * \param mcTruncate If the text will be truncated to 20 characters
      * or shown in its entirety
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QString & mcrValue,
        const bool mcTruncate = true);

    /** \brief Show QUrl value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QUrl & mcrValue);

    /** \brief Show QVector < double > value in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const QVector < double > & mcrValue);

    /** \brief Show address of generic pointer in human readable form.
      * \param mcValue The value to show
      * \returns a QString with a human readable representation of the value
      */
    static QString Show(const void * mcpValue);
};

#endif
