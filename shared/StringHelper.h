// StringHelper.h
// Class definition file

/** \file
  * \todo Add Doxygen information
  */

// Just include once
#ifndef STRINGHELPER_H
#define STRINGHELPER_H

// Qt includes
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

// Class definition
class StringHelper :
    public QObject
{
    Q_OBJECT
    
    
    
    // ============================================================== Lifecycle
private:
	// Default constructor (never to be called from outside)
	StringHelper();

public:
	// Destructor
	~StringHelper();



    // ===================================================== Special Characters
public:
    // "..." as a single UTF-8 character
    static QChar CharEllipse;



    // ================================================================ Sorting
public:

    // Sort a set of Keywords or other things
    static QList < int > SortHash(const QHash < int, QString > mcHash);
    static QList < int > SortHash_Reverse(const QHash < int, QString > mcHash);
    static QList < int > SortHash(const QHash < int, double > mcHash);
    static QList < int > SortHash(const QHash < int, int > mcHash);
    static QList < int > SortHash(const QHash < int, QDateTime > mcHash);
    static QList < QString > SortHash(const QHash < QString, QString > mcHash);
    static QList < QString > SortHash(const QHash < QString, int > mcHash);

    // Compare Strings but keep ID with it
    static bool LessThan_IntString(const QPair < int, QString > mcLeft,
        const QPair < int, QString > mcRight);
    static bool GreaterThan_IntString(const QPair < int, QString > mcLeft,
        const QPair < int, QString > mcRight);

    // Compare Strings but keep ID with it
    static bool LessThan_StringString(const QPair < QString, QString > mcLeft, 
        const QPair < QString, QString > mcRight);

    // Compare Strings but keep ID with it
    static bool LessThan_StringInt(const QPair < QString, int > mcLeft, 
        const QPair < QString, int > mcRight);

    // Compare ints but keep ID with it
    static bool LessThan_IntInt(const QPair < int, int > mcLeft, 
        const QPair < int, int > mcRight);

    // Compare doubles but keep ID with it
    static bool LessThan_IntDouble(const QPair < int, double > mcLeft, 
        const QPair < int, double > mcRight);

    // Compare dates but keep ID with it
    static bool LessThan_IntDate(const QPair < int, QString > mcLeft, 
        const QPair < int, QString > mcRight);

    // Compare Strings with natural order
    static bool LessThan_Natural(const QString mcLeft,
        const QString mcRight);
    static bool GreaterThan_Natural(const QString mcLeft,
        const QString mcRight);

    // Compare shutter times
    static bool LessThan_ShutterTime(const QString mcLeft, 
        const QString mcRight);
    static bool GreaterThan_ShutterTime(const QString mcLeft, 
        const QString mcRight);

    // Compare biases
    static bool LessThan_Bias(const QString mcLeft, 
        const QString mcRight);
    static bool GreaterThan_Bias(const QString mcLeft, 
        const QString mcRight);
    
    // Compare F stops
    static bool LessThan_FStop(const QString mcLeft, 
        const QString mcRight);
    static bool GreaterThan_FStop(const QString mcLeft, 
        const QString mcRight);
    
    
    
    // =================================================================== Misc
public:
    // Split filename into directory and filename.
    static QPair < QString, QString > SplitFilename(
        const QString mcFilename);

    // Parent directory of a given directory
    static QString ParentDirectory(const QString & mcrDirectory);

    // Sort filenames
    static QStringList SortFilenames(const QStringList & mcrFilenames);

    // Split text into words, taking quotes into account
    // (a "b c") => ("a", "b c")
    static QStringList SplitQuotedText(const QString mcText);
    
    // Convert file size
    static QString ConvertFileSize(const qint64 mcFileSize,
        const QString mcTargetScale = "automatic");

    // Convert number
    static QString ConvertNumber(const qint64 mcNumber);

    // Get portion bounded by brackets "([{" or quotes till the last closes.
    static QString GetBracketedPortion(QString & mrText);
    
    // Natural language lists
    static QString NaturalLanguageList(const QStringList mcList);
    
    // Decode encoded string, convert charset
    static QByteArray DecodeText(const QByteArray mcBody,
        const QString mcCharset, const QString mcTransferEncoding);
    
    // Guess charset from text
    static QString GuessCharset(const QByteArray mcText);
    
    // Escape non-ASCII characters (usually for debugging purposes)
    static QString EscapeNonAscii(const QByteArray mcText);
    
    // Convert ISO-8859-1 binary representation to UTF-8
    static QByteArray ConvertISO8859_1ToUTF8(const QByteArray mcText);
    
    // Convert ISO-8859-2 binary representation to ISO-8859-1
    static QByteArray ConvertISO8859_2ToISO8859_1(const QByteArray mcText);
    
    // Convert ISO-8859-2 binary representation to UTF-8
    static QByteArray ConvertISO8859_2ToUTF8(const QByteArray mcText);

    // Convert ISO-8859-15 binary representation to ISO-8859-1
    static QByteArray ConvertISO8859_15ToISO8859_1(const QByteArray mcText);
    
    // Convert ISO-8859-15 binary representation to UTF-8
    static QByteArray ConvertISO8859_15ToUTF8(const QByteArray mcText);
    
    // Convert Roman-8 (binary, unquoted representation) to ISO-8859-1
    static QByteArray ConvertRoman8ToISO8859_1(const QByteArray mcText);
    
    // Convert UTF-8 binary representation to ISO-8859-1
    static QByteArray ConvertUTF8ToISO8859_1(const QByteArray mcText);

    // Convert Windows-1252 binary representation to ISO-8859-1
    static QByteArray ConvertWindows1252ToISO8859_1(const QByteArray mcText);
    
    // Convert Windows-1252 binary representation to UTF-8
    static QByteArray ConvertWindows1252ToUTF8(const QByteArray mcText);
    
    // Mark search text
    static QString MarkSearchword(const QString mcText, 
        const QString mcSearchText, const QString mcColor = "FF0000",
        const QString mcBgColor = "FFFF00");
    
    // Month to name
    static QString MonthName(const int mcNumber);

    // Strip HTML tags from a string
    static QString StripHTMLTags(QString mString);

    // Check proper nesting (HTML)
    static QPair < bool, QString > CheckProperHTMLNesting(QString mHTML);

    // Convert some characters to HTML so the text can be put into XML
    static QString EncodeToHTML(QString mString);

    // Convert back from the previous function
    static QString DecodeFromHTML(QString mString);

    // Convert some characters to % (for text inclusion in XML)
    static QString PercentEncode(QString mString);

    // Convert some characters from % (for text inclusion in XML)
    static QString PercentDecode(QString mString);

    // Convert to CSV
    static QString EncodeToCSV(QString mString);

    // Convert UTF-8 to ASCII by stripping all non-existing characters
    static QString StripNonASCII(QString mString);

    // Convert long double to QString
    static QString ToString(const long double mcValue);

    // Convert QString to long double
    static long double ToLongDouble(const QString mcTextValue);

    // Convert integer into "first", "21st" etc.
    static QString ToNth(const int mcN);

    // Return unique elements of a list
    static QList < QString > UniqueElements(const QList < QString > mcValues);



    // ========================================================== Format stuff
public:
    // Check if a date has a valid format
    static bool IsValidDate(const QString & mcrDate);

    // Check if a date/time has a valid format
    static bool IsValidDateTime(const QString & mcrDateTime);



    // ====================================================== Photography stuff
public:
    // Convert exposure time to double
    static double ConvertExposureTimeToDouble(const QString mcExposureTime);

    // Convert f stop to double
    static double ConvertFStopToDouble(const QString mcFStop);
};

#endif
