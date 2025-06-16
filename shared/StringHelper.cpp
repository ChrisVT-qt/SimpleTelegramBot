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

// StringHelper.cpp
// Class implementation file

// Project includes
#include "CallTracer.h"
#include "MessageLogger.h"
#include "StringHelper.h"

// Qt includes
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>

// System includes
#include <cmath>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Default constructor (never to be called from outside)
StringHelper::StringHelper()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
StringHelper::~StringHelper()
{
    CALL_IN("");

    // Nothing to do, either.

    CALL_OUT("");
}



// ========================================================= Special Characters



///////////////////////////////////////////////////////////////////////////////
// "..." as a single UTF-8 character
QChar StringHelper::CharEllipse = QChar(0x2026);



// ==================================================================== Sorting



///////////////////////////////////////////////////////////////////////////////
// Sort a set of strings
QList < int > StringHelper::SortHash(const QHash < int, QString > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < int, QString > > raw;
    for (auto id_iterator = mcHash.keyBegin();
         id_iterator != mcHash.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        raw << QPair < int, QString >(id, mcHash[id]);
    }

    // Sort
    std::stable_sort(raw.begin(), raw.end(), &LessThan_IntString);

    // Recover keys in the correct order
    QList < int > ret;
    for (int idx = 0; idx < raw.size(); idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort a set of strings
QList < int > StringHelper::SortHash_Reverse(
    const QHash < int, QString > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < int, QString > > raw;
    for (auto id_iterator = mcHash.keyBegin();
         id_iterator != mcHash.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        raw << QPair < int, QString >(id, mcHash[id]);
    }

    // Sort
    std::stable_sort(raw.begin(), raw.end(), &GreaterThan_IntString);

    // Recover keys in the correct order
    QList < int > ret;
    for (int idx = 0; idx < raw.size(); idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort a set of numbers
QList < int > StringHelper::SortHash(const QHash < int, double > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < int, double > > raw;
    for (auto id_iterator = mcHash.keyBegin();
         id_iterator != mcHash.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        raw << QPair < int, double >(id, mcHash[id]);
    }

    // Sort
    std::stable_sort(raw.begin(), raw.end(), &LessThan_IntDouble);

    // Recover keys in the correct order
    QList < int > ret;
    for (int idx = 0; idx < raw.size(); idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort a set of numbers
QList < int > StringHelper::SortHash(const QHash < int, int > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < int, int > > raw;
    for (auto id_iterator = mcHash.keyBegin();
         id_iterator != mcHash.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        raw << QPair < int, int >(id, mcHash[id]);
    }

    // Sort
    std::stable_sort(raw.begin(), raw.end(), &LessThan_IntInt);

    // Recover keys in the correct order
    QList < int > ret;
    for (int idx = 0; idx < raw.size(); idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort by date/time
QList < int > StringHelper::SortHash(const QHash < int, QDateTime > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    QHash < int, int > sort_hash;
    for (auto id_iterator = mcHash.keyBegin();
         id_iterator != mcHash.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        sort_hash[id] = mcHash[id].toSecsSinceEpoch();
    }
    const QList < int > sorted_keys = SortHash(sort_hash);

    CALL_OUT("");
    return sorted_keys;
}



///////////////////////////////////////////////////////////////////////////////
// Sort a set of Properties
QList < QString > StringHelper::SortHash(
    const QHash < QString, QString > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < QString, QString > > raw;
    for (auto key_iterator = mcHash.keyBegin();
         key_iterator != mcHash.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        raw << QPair < QString, QString >(key, mcHash[key]);
    }

    // Sort
    std::sort(raw.begin(), raw.end(), &LessThan_StringString);

    // Recover keys in the correct order
    QList < QString > ret;
    for (int idx = 0;
         idx < raw.size();
         idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort a set of Properties
QList < QString > StringHelper::SortHash(
    const QHash < QString, int > mcHash)
{
    CALL_IN(QString("mcHash=%1")
        .arg(CALL_SHOW(mcHash)));

    // Set up list of pairs
    QList < QPair < QString, int > > raw;
    for (auto key_iterator = mcHash.keyBegin();
         key_iterator != mcHash.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        raw << QPair < QString, int >(key, mcHash[key]);
    }

    // Sort
    std::sort(raw.begin(), raw.end(), &LessThan_StringInt);

    // Recover keys in the correct order
    QList < QString > ret;
    for (int idx = 0;
         idx < raw.size();
         idx++)
    {
        ret << raw[idx].first;
    }

    // Done.
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Sort and return indexes
QList < int > StringHelper::SortAndReturnIndex(const QStringList & mcrData)
{
    CALL_IN(QString("mcrData=%1")
        .arg(CALL_SHOW(mcrData)));

    QHash < int, QString > data;
    for (int index = 0;
         index < mcrData.size();
         index++)
    {
        data[index] = mcrData[index];
    }

    const QList < int > indices = SortHash(data);

    CALL_OUT("");
    return indices;
}



///////////////////////////////////////////////////////////////////////////////
// Compare Strings but keep ID with it
bool StringHelper::LessThan_IntString(const QPair < int, QString > mcLeft,
    const QPair < int, QString > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    const bool result = LessThan_Natural(mcLeft.second, mcRight.second);

    CALL_OUT("");
    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Compare Strings but keep ID with it
bool StringHelper::GreaterThan_IntString(const QPair < int, QString > mcLeft,
    const QPair < int, QString > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    const bool result = GreaterThan_Natural(mcLeft.second, mcRight.second);

    CALL_OUT("");
    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Compare Strings but keep ID with it
bool StringHelper::LessThan_StringString(
    const QPair < QString, QString > mcLeft,
    const QPair < QString, QString > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    const bool result = LessThan_Natural(mcLeft.second, mcRight.second);

    CALL_OUT("");
    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Compare ints but keep ID with it
bool StringHelper::LessThan_StringInt(
    const QPair < QString, int > mcLeft,
    const QPair < QString, int > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    CALL_OUT("");
    return (mcLeft.second < mcRight.second);
}



///////////////////////////////////////////////////////////////////////////////
// Compare ints but keep ID with it
bool StringHelper::LessThan_IntInt(const QPair < int, int > mcLeft,
    const QPair < int, int > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    CALL_OUT("");
    return (mcLeft.second < mcRight.second);
}



///////////////////////////////////////////////////////////////////////////////
// Compare doubles but keep ID with it
bool StringHelper::LessThan_IntDouble(const QPair < int, double > mcLeft,
    const QPair < int, double > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    CALL_OUT("");
    return (mcLeft.second < mcRight.second);
}



///////////////////////////////////////////////////////////////////////////////
// Compare dates but keep ID with it
bool StringHelper::LessThan_IntDate(const QPair < int, QString > mcLeft,
    const QPair < int, QString > mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // By definition, "empty" comes before anything that has a value.
    // (e.g. "" is before everything, and "2009" is before "2009-01-01")

    // Speed-up - this will address most cases
    // (both full date/time, or both empty)
    if (mcLeft.second.size() == mcRight.second.size())
    {
        CALL_OUT("");
        return (mcLeft.second < mcRight.second);
    }

    // Date format is YYYY-MM-DD hh:mm:ss. Dates may be partial.

    // Simple checks
    if (mcLeft.second.isEmpty())
    {
        // Right can't be empty per the above
        CALL_OUT("");
        return true;
    }
    if (mcRight.second.isEmpty())
    {
        // Left can't be empty per the above
        CALL_OUT("");
        return false;
    }

    // Break up date
    static const QRegularExpression split(
        "^([0-9]{4})(?:-(0[1-9]|1[0-2])(?:-(0[1-9]|[12][0-9]|3[01])"
        " ?(?:([01][0-9]|2[0-3])(?::([0-5][0-9])(?::([0-5][0-9])"
        ")?)?)?)?)?$");

    // Left
    QRegularExpressionMatch split_match = split.match(mcLeft.second);
    if (!split_match.hasMatch())
    {
        const QString reason =
            tr("\"%1\" does not match date regular expression. Ignored.")
                .arg(mcLeft.second);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }
    const QString left_year = split_match.captured(1);
    const QString left_month = split_match.captured(2);
    const QString left_day = split_match.captured(3);
    const QString left_hour = split_match.captured(4);
    const QString left_minute = split_match.captured(5);
    const QString left_second = split_match.captured(6);

    // Right
    split_match = split.match(mcRight.second);
    if (!split_match.hasMatch())
    {
        const QString reason =
            tr("\"%1\" does not match date regular expression. Ignored.")
                .arg(mcRight.second);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }
    const QString right_year = split_match.captured(1);
    const QString right_month = split_match.captured(2);
    const QString right_day = split_match.captured(3);
    const QString right_hour = split_match.captured(4);
    const QString right_minute = split_match.captured(5);
    const QString right_second = split_match.captured(6);

    // Do comparison
    // Year
    if (left_year.isEmpty())
    {
        CALL_OUT("");
        return !right_year.isEmpty();
    }
    if (right_year.isEmpty())
    {
        // Left year is not empty, and
        // anything <= "" is false
        CALL_OUT("");
        return false;
    }
    if (left_year != right_year)
    {
        CALL_OUT("");
        return (left_year < right_year);
    }

    // Month
    if (left_month.isEmpty())
    {
        CALL_OUT("");
        return !right_month.isEmpty();
    }
    if (right_month.isEmpty())
    {
        // Left month is not empty, and
        // anything < "" is false
        CALL_OUT("");
        return false;
    }
    if (left_month != right_month)
    {
        CALL_OUT("");
        return (left_month < right_month);
    }

    // Day
    if (left_day.isEmpty())
    {
        CALL_OUT("");
        return !right_day.isEmpty();
    }
    if (right_day.isEmpty())
    {
        // Left day is not empty, and
        // anything < "" is false
        CALL_OUT("");
        return false;
    }
    if (left_day != right_day)
    {
        CALL_OUT("");
        return (left_day < right_day);
    }

    // Hour
    if (left_hour.isEmpty())
    {
        CALL_OUT("");
        return !right_hour.isEmpty();
    }
    if (right_hour.isEmpty())
    {
        // Left hour is not empty, and
        // anything < "" is false
        CALL_OUT("");
        return false;
    }
    if (left_hour != right_hour)
    {
        CALL_OUT("");
        return (left_hour < right_hour);
    }

    // Minute
    if (left_minute.isEmpty())
    {
        CALL_OUT("");
        return !right_minute.isEmpty();
    }
    if (right_minute.isEmpty())
    {
        // Left minute is not empty, and
        // anything < "" is false
        CALL_OUT("");
        return false;
    }
    if (left_minute != right_minute)
    {
        CALL_OUT("");
        return (left_minute < right_minute);
    }

    // Second
    if (left_second.isEmpty())
    {
        CALL_OUT("");
        return !right_second.isEmpty();
    }
    if (right_second.isEmpty())
    {
        // Left second is not empty, and
        // anything <= "" is false
        CALL_OUT("");
        return false;
    }

    CALL_OUT("");
    return (left_second < right_second);
}



///////////////////////////////////////////////////////////////////////////////
// Compare Strings with natural order
bool StringHelper::LessThan_Natural(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Cache for speed-up
    static QHash < QString, QList < QString > > cache;

    // Quick trivial check
    QString left(mcLeft.toLower());
    QString right(mcRight.toLower());

    if (left == right)
    {
        CALL_OUT("");
        return false;
    }

    // Regular expressions don't like control characters
    left.replace("\n", " ");
    left.replace("\t", " ");
    right.replace("\n", " ");
    right.replace("\t", " ");

    // Regular expressions
    static const QRegularExpression split("^([^0-9]+)?([0-9]+)?([^0-9].*)?$");

    // Repeated splitting
    QString left_head, left_number;
    QString right_head, right_number;
    while (!left.isEmpty() && !right.isEmpty())
    {
        // Split left
        if (!cache.contains(left))
        {
            QRegularExpressionMatch split_match = split.match(left);
            if (!split_match.hasMatch())
            {
                const QString reason(tr("No match for: %1")
                    .arg(left));
                MessageLogger::Error(CALL_METHOD, reason);
            }
            cache[left] << split_match.captured(1).trimmed()
                << split_match.captured(2)
                << split_match.captured(3).trimmed();
        }
        left_head = cache[left][0];
        left_number = cache[left][1];
        left = cache[left][2];

        // Split right
        if (!cache.contains(right))
        {
            QRegularExpressionMatch split_match = split.match(right);
            if (!split_match.hasMatch())
            {
                const QString reason(tr("No match for: %1")
                    .arg(right));
                MessageLogger::Error(CALL_METHOD, reason);
            }
            cache[right] << split_match.captured(1).trimmed()
                << split_match.captured(2)
                << split_match.captured(3).trimmed();
        }
        right_head = cache[right][0];
        right_number = cache[right][1];
        right = cache[right][2];

        // Check head text part
        if (left_head < right_head)
        {
            CALL_OUT("");
            return true;
        }
        if (left_head > right_head)
        {
            CALL_OUT("");
            return false;
        }

        // Check number

        // Kill heading zeros
        QString left_no_zeros(left_number);
        while (left_no_zeros.left(1) == "0")
        {
            left_no_zeros = left_no_zeros.mid(1);
        }
        QString right_no_zeros(right_number);
        while (right_no_zeros.left(1) == "0")
        {
            right_no_zeros = right_no_zeros.mid(1);
        }
        if (left_no_zeros.size() < right_no_zeros.size())
        {
            CALL_OUT("");
            return true;
        }
        if (left_no_zeros.size() > right_no_zeros.size())
        {
            CALL_OUT("");
            return false;
        }

        // Number of digits is equal here.
        if (left_no_zeros < right_no_zeros)
        {
            CALL_OUT("");
            return true;
        }
        if (left_no_zeros > right_no_zeros)
        {
            CALL_OUT("");
            return false;
        }
    }

    // If we arrive here, one is empty
    if (left.isEmpty())
    {
        CALL_OUT("");
        return !right.isEmpty();
    } else
    {
        // Right is empty, so it is smaller.
        CALL_OUT("");
        return false;
    }

    // We never get here
}



///////////////////////////////////////////////////////////////////////////////
// Compare Strings with natural order
bool StringHelper::GreaterThan_Natural(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Just reverse order and use existing function
    const bool result = LessThan_Natural(mcRight, mcLeft);

    CALL_OUT("");
    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Compare shutter times
bool StringHelper::LessThan_ShutterTime(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Either number, or "1/Number", or empty

    // The user of this function has to make sure that the strings
    // are actually shutter times.

    // Empty is first.
    if (mcLeft.isEmpty())
    {
        CALL_OUT("");
        return true;
    }
    if (mcRight.isEmpty())
    {
        CALL_OUT("");
        return false;
    }

    // Calculate values
    double left_value = -1;
    bool left_is_inf = false;
    double right_value = -1;
    bool right_is_inf = false;
    if (mcLeft.startsWith("1/"))
    {
        double tmp = mcLeft.mid(2).toDouble();
        if (fabs(tmp) < 1e-15)
        {
            left_is_inf = true;
        } else
        {
            left_value = 1./tmp;
        }
    } else
    {
        left_value = mcLeft.toDouble();
    }
    if (mcRight.startsWith("1/"))
    {
        double tmp = mcRight.mid(2).toDouble();
        if (fabs(tmp) < 1e-15)
        {
            right_is_inf = true;
        } else
        {
            right_value = 1./tmp;
        }
    } else
    {
        right_value = mcRight.toDouble();
    }

    // Return comparison
    if (right_is_inf)
    {
        CALL_OUT("");
        return true;
    }
    if (left_is_inf)
    {
        CALL_OUT("");
        return false;
    }

    CALL_OUT("");
    return (left_value < right_value);
}



///////////////////////////////////////////////////////////////////////////////
// Compare shutter times
bool StringHelper::GreaterThan_ShutterTime(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Either number, or "1/Number", or empty

    // The user of this function has to make sure that the strings
    // are actually shutter times.

    // Empty is first.
    if (mcRight.isEmpty())
    {
        CALL_OUT("");
        return true;
    }
    if (mcLeft.isEmpty())
    {
        CALL_OUT("");
        return false;
    }

    // Calculate values
    double left_value = -1;
    bool left_is_inf = false;
    double right_value = -1;
    bool right_is_inf = false;
    if (mcLeft.startsWith("1/"))
    {
        double tmp = mcLeft.mid(2).toDouble();
        if (fabs(tmp) < 1e-15)
        {
            left_is_inf = true;
        } else
        {
            left_value = 1./tmp;
        }
    } else
    {
        left_value = mcLeft.toDouble();
    }
    if (mcRight.startsWith("1/"))
    {
        double tmp = mcRight.mid(2).toDouble();
        if (fabs(tmp) < 1e-15)
        {
            right_is_inf = true;
        } else
        {
            right_value = 1./tmp;
        }
    } else
    {
        right_value = mcRight.toDouble();
    }

    // Return comparison
    if (left_is_inf)
    {
        CALL_OUT("");
        return true;
    }
    if (right_is_inf)
    {
        CALL_OUT("");
        return false;
    }

    CALL_OUT("");
    return (left_value > right_value);
}



///////////////////////////////////////////////////////////////////////////////
// Compare biases
bool StringHelper::LessThan_Bias(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Either "0", "+number", or "-number".

    // The user of this function has to make sure that the strings
    // are actually shutter times.

    // Calculate values
    double left_value = 0;
    if (mcLeft.startsWith("+"))
    {
        left_value = mcLeft.mid(1).toDouble();
    } else if (mcLeft.startsWith("-"))
    {
        left_value = -mcLeft.mid(1).toDouble();
    } else
    {
        left_value = mcLeft.toDouble();
    }
    double right_value = 0;
    if (mcRight.startsWith("+"))
    {
        right_value = mcRight.mid(1).toDouble();
    } else if (mcRight.startsWith("-"))
    {
        right_value = -mcRight.mid(1).toDouble();
    } else
    {
        right_value = mcRight.toDouble();
    }

    // Comparison
    CALL_OUT("");
    return (left_value < right_value);
}



///////////////////////////////////////////////////////////////////////////////
// Compare biases
bool StringHelper::GreaterThan_Bias(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Either "0", "+number", or "-number".

    // The user of this function has to make sure that the strings
    // are actually shutter times.

    // Calculate values
    double left_value = 0;
    if (mcLeft.startsWith("+"))
    {
        left_value = mcLeft.mid(1).toDouble();
    } else if (mcLeft.startsWith("-"))
    {
        left_value = -mcLeft.mid(1).toDouble();
    } else
    {
        left_value = mcLeft.toDouble();
    }
    double right_value = 0;
    if (mcRight.startsWith("+"))
    {
        right_value = mcRight.mid(1).toDouble();
    } else if (mcRight.startsWith("-"))
    {
        right_value = -mcRight.mid(1).toDouble();
    } else
    {
        right_value = mcRight.toDouble();
    }

    // Comparison
    CALL_OUT("");
    return (left_value > right_value);
}



///////////////////////////////////////////////////////////////////////////////
// Compare F stops
bool StringHelper::LessThan_FStop(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Form is "f/xxx"

    // Empty is first.
    if (mcLeft.isEmpty() || !mcLeft.startsWith("f/"))
    {
        CALL_OUT("");
        return !mcRight.isEmpty() && mcRight.startsWith("f/");
    }
    if (mcRight.isEmpty() || !mcRight.startsWith("f/"))
    {
        CALL_OUT("");
        return false;
    }

    // Convert
    const double left = mcLeft.mid(2).toDouble();
    const double right = mcRight.mid(2).toDouble();

    CALL_OUT("");
    return (left < right);
}



///////////////////////////////////////////////////////////////////////////////
// Compare F stops
bool StringHelper::GreaterThan_FStop(const QString mcLeft,
    const QString mcRight)
{
    CALL_IN(QString("mcLeft=%1, mcRight=%2")
        .arg(CALL_SHOW(mcLeft),
             CALL_SHOW(mcRight)));

    // Form is "f/xxx"

    // Empty is first.
    if (mcRight.isEmpty() || !mcRight.startsWith("f/"))
    {
        CALL_OUT("");
        return !mcLeft.isEmpty() && mcLeft.startsWith("f/");
    }
    if (mcLeft.isEmpty() || !mcLeft.startsWith("f/"))
    {
        CALL_OUT("");
        return false;
    }

    // Convert
    const double left = mcLeft.mid(2).toDouble();
    const double right = mcRight.mid(2).toDouble();

    CALL_OUT("");
    return (left > right);
}



// ======================================================================= Misc



///////////////////////////////////////////////////////////////////////////////
// Split filename into directory and filename.
QPair < QString, QString > StringHelper::SplitFilename(
    const QString mcFilename)
{
    CALL_IN(QString("mcFilename=%1")
        .arg(CALL_SHOW(mcFilename)));

    // Split
    static const QRegularExpression split("^((.*[^/])/+)?([^/]*)?$");
    QRegularExpressionMatch split_match = split.match(mcFilename);

    CALL_OUT("");
    return QPair < QString, QString >(split_match.captured(2),
        split_match.captured(3));
}



///////////////////////////////////////////////////////////////////////////////
// Parent directory of a given directory
QString StringHelper::ParentDirectory(const QString & mcrDirectory)
{
    CALL_IN(QString("mcrDirectory=%1")
        .arg(CALL_SHOW(mcrDirectory)));

    QString parent_directory = mcrDirectory;
    QString suffix;
    if (parent_directory.endsWith("/"))
    {
        suffix = "/";
        parent_directory = parent_directory.left(parent_directory.size() - 1);
    }
    const int index = parent_directory.lastIndexOf("/");
    if (index == -1)
    {
        CALL_OUT("");
        return suffix;
    }
    parent_directory = parent_directory.left(index) + suffix;

    CALL_OUT("");
    return parent_directory;
}



///////////////////////////////////////////////////////////////////////////////
// Sort filenames
QStringList StringHelper::SortFilenames(const QStringList & mcrFilenames)
{
    CALL_IN(QString("mcrFilenames=%1")
        .arg(CALL_SHOW(mcrFilenames)));

    QHash < int, QString > sort;
    for (int index = 0;
         index < mcrFilenames.size();
         index++)
    {
        const QPair < QString, QString > split_filename =
            SplitFilename(mcrFilenames[index]);
        sort[index] = split_filename.second + " " + split_filename.first;
    }
    const QList < int > sorted_index = SortHash(sort);

    QStringList sorted_filenames;
    for (int index : sorted_index)
    {
        sorted_filenames << mcrFilenames[index];
    }

    CALL_OUT("");
    return sorted_filenames;
}



///////////////////////////////////////////////////////////////////////////////
// Split text into words, taking quotes into account
// (a "b c") => ("a", "b c")
QStringList StringHelper::SplitQuotedText(const QString mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // Get quoted expressions
    QStringList match_words;
    static const QRegularExpression format_quoted(
        "^([^\"]*)\"([^\"]*)\"(.*)$");
    QString remaining_text = mcText;
    QRegularExpressionMatch match = format_quoted.match(remaining_text);
    while (match.hasMatch())
    {
        // Exclude empty expressions
        if (!match.captured(2).isEmpty())
        {
            // Not trimming quoted expressions
            match_words << match.captured(2);
        }
        remaining_text = match.captured(1).trimmed();
        if (!remaining_text.isEmpty())
        {
            remaining_text += " ";
        }
        remaining_text += match.captured(3).trimmed();
        match = format_quoted.match(remaining_text);
    }
    // Remove any remaining unmatched quotes
    remaining_text.replace("\"", "");
    // Unquoted single words
    static const QRegularExpression format_space("^\\s*([^\\s]+)(.*)$");
    match = format_space.match(remaining_text);
    while (match.hasMatch())
    {
        match_words << match.captured(1);
        remaining_text = match.captured(2).trimmed();
        match = format_space.match(remaining_text);
    }
    if (!remaining_text.isEmpty())
    {
        match_words << remaining_text;
    }

    CALL_OUT("");
    return match_words;
}



///////////////////////////////////////////////////////////////////////////////
// Convert file size
QString StringHelper::ConvertFileSize(const qint64 mcFileSize,
    const QString mcTargetScale)
{
    CALL_IN(QString("mcFileSize=%1, mcTargetScale=%2")
        .arg(CALL_SHOW(mcFileSize),
             CALL_SHOW(mcTargetScale)));

    if (mcTargetScale == "byte" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<10))
    {
        CALL_OUT("");
        return QString::number(mcFileSize) + " " + tr("bytes");
    }

    if (mcTargetScale == "k" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<20))
    {
        CALL_OUT("");
        return QString("%1k").arg(mcFileSize * 1./(qint64(1)<<10), 0, 'f', 1);
    }

    if (mcTargetScale == "M" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<30))
    {
        CALL_OUT("");
        return QString("%1M").arg(mcFileSize * 1./(qint64(1)<<20), 0, 'f', 1);
    }

    if (mcTargetScale == "G" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<40))
    {
        CALL_OUT("");
        return QString("%1G").arg(mcFileSize * 1./(qint64(1)<<30), 0, 'f', 1);
    }

    if (mcTargetScale == "T" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<50))
    {
        CALL_OUT("");
        return QString("%1T").arg(mcFileSize * 1./(qint64(1)<<40), 0, 'f', 1);
    }

    if (mcTargetScale == "P" ||
        (mcTargetScale == "automatic" && mcFileSize < qint64(1)<<60))
    {
        CALL_OUT("");
        return QString("%1P").arg(mcFileSize * 1./(qint64(1)<<50), 0, 'f', 1);
    }

    if (mcTargetScale == "E" || mcTargetScale == "automatic")
    {
        CALL_OUT("");
        return QString("%1E").arg(mcFileSize * 1./(qint64(1)<<60), 0, 'f', 1);
    }

    // Unknown target scale
    const QString reason = tr("Unknown target scale \"%1\".")
        .arg(mcTargetScale);
    MessageLogger::Error(CALL_METHOD, reason);
    CALL_OUT(reason);
    return QString();
}



///////////////////////////////////////////////////////////////////////////////
// Convert number
QString StringHelper::ConvertNumber(const qint64 mcNumber)
{
    CALL_IN(QString("mcNumber=%1")
        .arg(CALL_SHOW(mcNumber)));

    if (mcNumber < 1000000)
    {
        CALL_OUT("");
        return QString::number(mcNumber);
    }

    if (mcNumber < qint64(1000000000))
    {
        CALL_OUT("");
        return QString("%1 M").arg(mcNumber * 1./1e6, 0, 'f', 2);
    }

    CALL_OUT("");
    return QString("%1 B").arg(mcNumber * 1./1e9, 0, 'f', 2);
}



///////////////////////////////////////////////////////////////////////////////
// Get portion bounded by brackets "([{" or quotes till the last closes.
QString StringHelper::GetBracketedPortion(QString & mrText)
{
    CALL_IN(QString("mrText=%1")
        .arg(CALL_SHOW(mrText)));

    // Count quotes, braces ({}), brackets ([]], and parantheses (()).
    int count_open_quotes = 0;
    int count_open_braces = 0;
    int count_open_brackets = 0;
    int count_open_parantheses = 0;
    int idx = 0;
    while (idx < mrText.size())
    {
        if (mrText[idx] == '{' && count_open_quotes == 0)
        {
            count_open_braces++;
        }
        if (mrText[idx] == '}' && count_open_quotes == 0)
        {
            count_open_braces--;
        }
        if (mrText[idx] == '[' && count_open_quotes == 0)
        {
            count_open_brackets++;
        }
        if (mrText[idx] == ']' && count_open_quotes == 0)
        {
            count_open_brackets--;
        }
        if (mrText[idx] == '(' && count_open_quotes == 0)
        {
            count_open_parantheses--;
        }
        if (mrText[idx] == ')' && count_open_quotes == 0)
        {
            count_open_parantheses--;
        }
        if (mrText[idx] == '\"')
        {
            // Quotes are never nested
            count_open_quotes = 1 - count_open_quotes;
        }

        // Look at next position
        idx++;

        // Check for termination
        if (count_open_quotes == 0 &&
            count_open_braces == 0 &&
            count_open_brackets == 0 &&
            count_open_parantheses == 0)
        {
            break;
        }
    }

    // Return value
    QString ret = mrText.left(idx);

    // All the rest
    mrText = mrText.right(mrText.size() - idx);

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Natural language lists
QString StringHelper::NaturalLanguageList(const QStringList mcList)
{
    CALL_IN(QString("mcList=%1")
        .arg(CALL_SHOW(mcList)));

    QString ret;
    switch (mcList.size())
    {
        case 0:
            // Do nothing.
            break;
        case 1:
            ret = mcList[0];
            break;
        case 2:
            ret = mcList.join(tr(" and "));
            break;
        default:
            for (int idx = 0;
                 idx < mcList.size() - 1;
                 idx++)
            {
                ret += (idx > 0 ? ", " : "") + mcList[idx];
            }
            ret += tr(", and ") + mcList.last();
            break;
    }

    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Decode encoded string, convert charset
QByteArray StringHelper::DecodeText(const QByteArray mcBody,
    const QString mcCharset, const QString mcTransferEncoding)
{
    CALL_IN(QString("mcBody=%1, mcCharset=%2, mcTransferEncoding=%3")
        .arg(CALL_SHOW(mcBody),
             CALL_SHOW(mcCharset),
             CALL_SHOW(mcTransferEncoding)));

    // === Reverse transfer encoding
    QByteArray decoded;
    if (mcTransferEncoding == "quoted-printable")
    {
        int index = 0;
        while (index < mcBody.size())
        {
            if (mcBody.at(index) == '=')
            {
                // Check for end of line
                if (index < mcBody.size() - 1 &&
                    mcBody.at(index+1) == '\n')
                {
                    // Remove line continuation ("=\n")
                    index += 2;
                    continue;
                }
                if (index < mcBody.size() - 2 &&
                    mcBody.at(index+1) == '\r' &&
                    mcBody.at(index+2) == '\n')
                {
                    // Remove line continuation ("=\r\n")
                    index += 3;
                    continue;
                }

                // Check for something to decode
                if (index < mcBody.size() - 2)
                {
                    QChar char_1 = mcBody.at(index+1);
                    if ((char_1 >= '0' && char_1 <= '9') ||
                        (char_1 >= 'A' && char_1 <= 'F'))
                    {
                        QChar char_2 = mcBody.at(index+2);
                        if ((char_2 >= '0' && char_2 <= '9') ||
                            (char_2 >= 'A' && char_2 <= 'F'))
                        {
                            // Decode
                            const QString hex_value = QString("%1%2")
                                .arg(char_1,
                                     char_2);
                            bool status = false;
                            unsigned short char_value =
                                hex_value.toUShort(&status, 16);
                            decoded += (char)char_value;
                            index += 3;
                            continue;
                        }
                    }
                }

                // Just an equal sign, not an encoded character.
                decoded += '=';
                index++;
            } else
            {
                // No decoding
                decoded += mcBody.at(index);
                index++;
            }
        }
    } else if (mcTransferEncoding == "7bit" ||
        mcTransferEncoding == "8bit" ||
        mcTransferEncoding == "binary" ||
        mcTransferEncoding.isEmpty())
    {
        // No encoding
        decoded = mcBody;
    } else if (mcTransferEncoding == "base64")
    {
        CALL_OUT("");
        return QByteArray::fromBase64(mcBody);
    } else
    {
        // Unknown encoding
        const QString reason =
            tr("Unknown transfer encoding \"%1\".").arg(mcTransferEncoding);
        MessageLogger::Error(CALL_METHOD, reason);
    }

    // === Convert character sets

    // Check if we need to guess the charset
    QString charset = mcCharset;
    if (charset == "unknown-8bit" ||
        charset == "x-unknown" ||
        charset.isEmpty())
    {
        charset = GuessCharset(decoded);
    }

    QByteArray ret;
    if (charset == "utf-8")
    {
        // UTF-8
        ret = QString::fromUtf8(decoded).toUtf8();
    } else if (charset == "ascii" ||
        charset == "us-ascii" ||
        charset == "iso-8859-1")
    {
        // ISO-8859-1/Latin-1
        ret = QString::fromLatin1(decoded).toUtf8();
    } else if (charset == "x-roman8")
    {
        // HP Roman-8
        decoded = ConvertRoman8ToISO8859_1(decoded);
        ret = QString::fromLatin1(decoded).toLatin1();
    } else if (charset == "iso-2022-jp")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "iso-2022-kr")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "iso-8859-2")
    {
        // ISO-8859-2
        decoded = ConvertISO8859_2ToISO8859_1(decoded);
        ret = QString::fromLatin1(decoded).toUtf8();
    } else if (charset == "iso-8859-7")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "iso-8859-13")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "iso-8859-15")
    {
        // ISO-8859-15
        decoded = ConvertISO8859_15ToISO8859_1(decoded);
        ret = QString::fromLatin1(decoded).toLatin1();
    } else if (charset == "koi8-r")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "windows-1250")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "windows-1251")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "windows-1252")
    {
        // Windows-1252
        decoded = ConvertWindows1252ToISO8859_1(decoded);
        ret = QString::fromLatin1(decoded).toLatin1();
    } else if (charset == "windows-1254")
    {
        // Not supported
        const QString reason =
            tr("Character set %1 is not supported.").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else if (charset == "unknown-8bit")
    {
        // Guessing was unsuccessful; leave data untouched.
        const QString reason =
            tr("Character was \"unknown-8bit\" but could not be guessed.");
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    } else
    {
        // Unknown character set
        const QString reason =
            tr("Unknown character set \"%1\".").arg(charset);
        MessageLogger::Error(CALL_METHOD, reason);
        ret = decoded;
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Guess charset from text
QString StringHelper::GuessCharset(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // This is used if a charset "unknown-8bit", "x-unknown", or an empty
    // charset was given.
    QHash < int, int > char_count;
    for (int idx = 0;
         idx < mcText.size();
         idx++)
    {
        char_count[(unsigned int)mcText.at(idx)]++;
    }

    // Set for plain ASCII
    static QSet < int > ascii;
    if (ascii.isEmpty())
    {
        for (int char_value = 0;
             char_value < 128;
             char_value++)
        {
            ascii += char_value;
        }
    }

    // Set for ISO-8859-1 (Latin-1)
    static QSet < int > iso_8859_1;
    if (iso_8859_1.isEmpty())
    {
        iso_8859_1 += ascii;
        for (int char_value = 160;
             char_value < 255;
             char_value++)
        {
            iso_8859_1 += char_value;
        }
    }

    // Set for Windows-1252
    static QSet < int > windows_1252;
    if (windows_1252.isEmpty())
    {
        windows_1252 += iso_8859_1;
        for (int char_value = 128;
             char_value < 160;
             char_value++)
        {
            if (char_value == 0x81 || char_value == 0x8d ||
                char_value == 0x8f || char_value == 0x90 ||
                char_value == 0x9d)
            {
                // Not defined in Windows-1252
                continue;
            }
            windows_1252 += char_value;
        }
    }

    // Now guess character set
    const QSet < int > used_characters(char_count.keyBegin(),
        char_count.keyEnd());

    // Penalty for ASCII
    QSet < int > residual = used_characters;
    residual -= ascii;
    int penalty_ascii = 0;
    for (int char_value : std::as_const(residual))
    {
        penalty_ascii += char_count[char_value];
    }
    if (penalty_ascii == 0)
    {
        CALL_OUT("");
        return "us-ascii";
    }

    // Penalty for ISO-8859-1
    residual = used_characters;
    residual -= iso_8859_1;
    int penalty_iso_8859_1 = 0;
    for (int char_value : std::as_const(residual))
    {
        penalty_iso_8859_1 += char_count[char_value];
    }
    if (penalty_iso_8859_1 == 0)
    {
        CALL_OUT("");
        return "iso-8859-1";
    }

    // Penalty for Windows-1252
    residual = used_characters;
    residual -= windows_1252;
    int penalty_windows_1252 = 0;
    for (int char_value : std::as_const(residual))
    {
        penalty_windows_1252 += char_count[char_value];
    }
    if (penalty_windows_1252 == 0)
    {
        CALL_OUT("");
        return "windows-1252";
    }

    // Approximate match only.
    if (penalty_ascii <= penalty_iso_8859_1 &&
        penalty_ascii <= penalty_windows_1252)
    {
        CALL_OUT("");
        return "us-ascii";
    }
    if (penalty_iso_8859_1 <= penalty_ascii &&
        penalty_iso_8859_1 <= penalty_windows_1252)
    {
        CALL_OUT("");
        return "iso-8859-1";
    }
    if (penalty_windows_1252 <= penalty_ascii &&
        penalty_windows_1252 <= penalty_iso_8859_1)
    {
        CALL_OUT("");
        return "windows-1252";
    }

    // Can't end up here.
    CALL_OUT("");
    return "unknown-8bit";
}



///////////////////////////////////////////////////////////////////////////////
// Escape non-ASCII characters (usually for debugging purposes)
QString StringHelper::EscapeNonAscii(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    QByteArray ret;
    for (int idx = 0;
         mcText.at(idx) != '\0';
         idx++)
    {
        unsigned char char_value = (unsigned char)mcText.at(idx);
        if (char_value < 128)
        {
            ret.append(mcText.at(idx));
        } else
        {
            ret.append(QString("=%1").arg(char_value, 0, 16).toLatin1());
        }
    }

    CALL_OUT("");
    return QString(ret);
}



///////////////////////////////////////////////////////////////////////////////
// Convert ISO-8859-1 (binary, unquoted representation) to UTF-8
QByteArray StringHelper::ConvertISO8859_1ToUTF8(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // From https://github.com/sebkirche/pbniregex/blob/master/stuff/
    //      CP1252%20%20%20ISO-8859-1%20%20%20UTF-8%20Conversion%20Chart.htm

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // 0x00 to 0x7F are identical
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg(QChar(character));
    }

    // 0x80 to 0x9F are not used in ISO-8859-1

    // 0xA0 to 0xFF - translated
    mapper[0xA0] = "&nbsp;";
    mapper[0xA1] = QString("%1").arg(QChar(0xc2a0)); // Inverted exclamation point
    mapper[0xA2] = "&cent;";        // Cent symbol
    mapper[0xA3] = "&pound;";       // Pound symbol
    mapper[0xA4] = "&curren;";      // Currency sign
    mapper[0xA5] = "&yen;";         // Yen sign
    mapper[0xA6] = "&brvbar;";      // Broken bar
    mapper[0xA7] = "&sect;";        // Section sign
    mapper[0xA8] = "&uml;";         // Umlaut or diaresis
    mapper[0xA9] = "&copy;";        // Copyright sign
    mapper[0xAA] = "&ordf;";        // Feminine ordinal
    mapper[0xAB] = "&laquo;";       // Left angle quotes
    mapper[0xAC] = "&not;";         // Logical not sign
    mapper[0xAD] = "&shy;";         // Soft hyphen
    mapper[0xAE] = "&reg;";         // Registered trademark
    mapper[0xAF] = "&macr;";        // Spacing macron

    mapper[0xB0] = "&deg;";         // Degree sign
    mapper[0xB1] = "&plusmn;";      // Plus-minus sign
    mapper[0xB2] = "&sup2;";        // Superscript 2
    mapper[0xB3] = "&sup3;";        // Superscript 3
    mapper[0xB4] = "&acute;";       // Spacing acute
    mapper[0xB5] = "&micro;";       // Micro sign
    mapper[0xB6] = "&para;";        // Paragraph sign
    mapper[0xB7] = "&middot;";      // Middle dot
    mapper[0xB8] = "&cedil;";       // Spacing cedilla
    mapper[0xB9] = "&sup1;";        // Superscript 1
    mapper[0xBA] = "&ordm;";        // Masculine ordinal
    mapper[0xBB] = "&raquo;";       // Right angle quotes
    mapper[0xBC] = "&frac14;";      // One quarter
    mapper[0xBD] = "&frac12;";      // One half
    mapper[0xBE] = "&frac34;";      // Three quarters
    mapper[0xBF] = QString("%1").arg(QChar(0xc2bf));   // Inverted question mark

    mapper[0xC0] = QString("%1").arg(QChar(0xC380)); // A grave
    mapper[0xC1] = QString("%1").arg(QChar(0xC381)); // A acute
    mapper[0xC2] = QString("%1").arg(QChar(0xC382)); // A circumflex
    mapper[0xC3] = QString("%1").arg(QChar(0xC383)); // A tilde
    mapper[0xC4] = QString("%1").arg(QChar(0xC384)); // A umlaut
    mapper[0xC5] = QString("%1").arg(QChar(0xC385)); // A ring
    mapper[0xC6] = QString("%1").arg(QChar(0xC386)); // AE ligature
    mapper[0xC7] = QString("%1").arg(QChar(0xC387)); // C cedilla
    mapper[0xC8] = QString("%1").arg(QChar(0xC388)); // E grave
    mapper[0xC9] = QString("%1").arg(QChar(0xC389)); // E acute
    mapper[0xCA] = QString("%1").arg(QChar(0xC38A)); // E circumflex
    mapper[0xCB] = QString("%1").arg(QChar(0xC38B)); // E umlaut
    mapper[0xCC] = QString("%1").arg(QChar(0xC38C)); // I grave
    mapper[0xCD] = QString("%1").arg(QChar(0xC38D)); // I aute
    mapper[0xCE] = QString("%1").arg(QChar(0xC38E)); // I circumflex
    mapper[0xCF] = QString("%1").arg(QChar(0xC38F)); // I umlaut

    mapper[0xD0] = QString("%1").arg(QChar(0xC390)); // ETH
    mapper[0xD1] = QString("%1").arg(QChar(0xC391)); // N tilde
    mapper[0xD2] = QString("%1").arg(QChar(0xC392)); // O grave
    mapper[0xD3] = QString("%1").arg(QChar(0xC393)); // O acute
    mapper[0xD4] = QString("%1").arg(QChar(0xC394)); // O circumflex
    mapper[0xD5] = QString("%1").arg(QChar(0xC395)); // O tilde
    mapper[0xD6] = QString("%1").arg(QChar(0xC396)); // O umlaut
    mapper[0xD7] = "&times;"; // Multiplication sign
    mapper[0xD8] = QString("%1").arg(QChar(0xC398)); // O slash
    mapper[0xD9] = QString("%1").arg(QChar(0xC399)); // U grave
    mapper[0xDA] = QString("%1").arg(QChar(0xC39A)); // U acute
    mapper[0xDB] = QString("%1").arg(QChar(0xC39B)); // U circumflex
    mapper[0xDC] = QString("%1").arg(QChar(0xC39C)); // U umlaut
    mapper[0xDD] = QString("%1").arg(QChar(0xC39D)); // Y acute
    mapper[0xDE] = QString("%1").arg(QChar(0xC39E)); // THORN
    mapper[0xDF] = QString("%1").arg(QChar(0xC39F)); // sharp s

    mapper[0xE0] = QString("%1").arg(QChar(0xC3A0)); // a grave
    mapper[0xE1] = QString("%1").arg(QChar(0xC3A1)); // a acute
    mapper[0xE2] = QString("%1").arg(QChar(0xC3A2)); // a circumflex
    mapper[0xE3] = QString("%1").arg(QChar(0xC3A3)); // a tilde
    mapper[0xE4] = QString("%1").arg(QChar(0xC3A4)); // a umlaut
    mapper[0xE5] = QString("%1").arg(QChar(0xC3A5)); // a ring
    mapper[0xE6] = QString("%1").arg(QChar(0xC3A6)); // ae ligature
    mapper[0xE7] = QString("%1").arg(QChar(0xC3A7)); // c cedilla
    mapper[0xE8] = QString("%1").arg(QChar(0xC3A8)); // e grave
    mapper[0xE9] = QString("%1").arg(QChar(0xC3A9)); // e acute
    mapper[0xEA] = QString("%1").arg(QChar(0xC3AA)); // e circumflex
    mapper[0xEB] = QString("%1").arg(QChar(0xC3AB)); // e umlaut
    mapper[0xEC] = QString("%1").arg(QChar(0xC3AC)); // i grave
    mapper[0xED] = QString("%1").arg(QChar(0xC3AD)); // i aute
    mapper[0xEE] = QString("%1").arg(QChar(0xC3AE)); // i circumflex
    mapper[0xEF] = QString("%1").arg(QChar(0xC3AF)); // i umlaut

    mapper[0xF0] = QString("%1").arg(QChar(0xC3B0)); // eth
    mapper[0xF1] = QString("%1").arg(QChar(0xC3B1)); // n tilde
    mapper[0xF2] = QString("%1").arg(QChar(0xC3B2)); // o grave
    mapper[0xF3] = QString("%1").arg(QChar(0xC3B3)); // o acute
    mapper[0xF4] = QString("%1").arg(QChar(0xC3B4)); // o circumflex
    mapper[0xF5] = QString("%1").arg(QChar(0xC3B5)); // o tilde
    mapper[0xF6] = QString("%1").arg(QChar(0xC3B6)); // o umlaut
    mapper[0xF7] = "&divide;"; // Division sign
    mapper[0xF8] = QString("%1").arg(QChar(0xC3B8)); // o slash
    mapper[0xF9] = QString("%1").arg(QChar(0xC3B9)); // u grave
    mapper[0xFA] = QString("%1").arg(QChar(0xC3BA)); // u acute
    mapper[0xFB] = QString("%1").arg(QChar(0xC3BB)); // u circumflex
    mapper[0xFC] = QString("%1").arg(QChar(0xC3BC)); // u umlaut
    mapper[0xFD] = QString("%1").arg(QChar(0xC3BD)); // y acute
    mapper[0xFE] = QString("%1").arg(QChar(0xC3BE)); // thorn
    mapper[0xFF] = QString("%1").arg(QChar(0xC3BF)); // y umlaut

    // Map
    QByteArray ret;
    for (int idx = 0;
         mcText.at(idx) != '\0';
         idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Error(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert ISO-8859-2 (binary, unquoted representation) to ISO-8859-1
QByteArray StringHelper::ConvertISO8859_2ToISO8859_1(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // From https://github.com/sebkirche/pbniregex/blob/master/stuff/
    //      CP1252%20%20%20ISO-8859-1%20%20%20UTF-8%20Conversion%20Chart.htm

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // 0x00 to 0x7F are all identical
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg((char)character);
    }

    // 0x80 to 0x9F are not used in ISO-8859-2

    // 0xA0 to 0xFF - mapped
    mapper[0xA0] = "&nbsp;";    // Non-breakable space
    mapper[0xA1] = "&Aogon;";   // latin capital letter A with ogonek
    mapper[0xA2] = "&breve;";   // breve
    mapper[0xA3] = "&Lstrok;";  // latin capital letter L with stroke
    mapper[0xA4] = "&curren;";  // currency sign
    mapper[0xA5] = "&Lcaron;";  // latin capital letter L with caron
    mapper[0xA6] = "&Sacute;";  // latin capital letter S with acute
    mapper[0xA7] = "&sect;";    // section sign
    mapper[0xA8] = "&uml;";     // diaeresis
    mapper[0xA9] = "&Scaron;";  // latin capital letter S with caron
    mapper[0xAA] = "&Scedil;";  // latin capital letter S with cedilla
    mapper[0xAB] = "&Tcaron;";  // latin capital letter T with caron
    mapper[0xAC] = "&Zacute;";  // latin capital letter Z with acute
    mapper[0xAD] = "&shy;";     // soft hyphen
    mapper[0xAE] = "&Zcaron;";  // latin capital letter Z with caron
    mapper[0xAF] = "&Zdot;";    // latin capital letter Z with dot above

    mapper[0xB0] = "&deg;";     // degree sign
    mapper[0xB1] = "&aogon;";   // latin small letter a with ogonek
    mapper[0xB2] = "&ogon;";    // ogonek
    mapper[0xB3] = "&lstrok;";  // latin small letter l with stroke
    mapper[0xB4] = "&acute;";   // acute accent
    mapper[0xB5] = "&lcaron;";  // latin small letter l with caron
    mapper[0xB6] = "&sacute;";  // latin small letter s with acute
    mapper[0xB7] = "&caron;";   // caron
    mapper[0xB8] = "&cedil;";   // cedilla
    mapper[0xB9] = "&scaron;";  // latin small letter s with caron
    mapper[0xBA] = "&scedil;";  // latin small letter s with cedilla
    mapper[0xBB] = "&tcaron;";  // latin small letter t with caron
    mapper[0xBC] = "&zacute;";  // latin small letter z with acute
    mapper[0xBD] = "&dblac;";   // double acute accent
    mapper[0xBE] = "&zcaron;";  // latin small letter z with caron
    mapper[0xBF] = "&zdot;";    // latin small letter z with dot above

    mapper[0xC0] = "&Racute;";  // latin capital letter R with acute
    mapper[0xC1] = "&Aacute;";  // latin capital letter A with acute
    mapper[0xC2] = "&Acric;";   // latin capital letter A with circumflex
    mapper[0xC3] = "&Abreve;";  // latin capital letter A with breve
    mapper[0xC4] = "&Auml;";    // latin capital letter A with diaeresis
    mapper[0xC5] = "&Lacute;";  // latin capital letter L with acute
    mapper[0xC6] = "&Cacute;";  // latin capital letter C with acute
    mapper[0xC7] = "&Ccedil;";  // latin capital letter C with cedilla
    mapper[0xC8] = "&Ccaron;";  // latin capital letter C with caron
    mapper[0xC9] = "&Eacute;";  // latin capital letter E with acute
    mapper[0xCA] = "&Eogon;";   // latin capital letter E with ogonek
    mapper[0xCB] = "&Euml;";    // latin capital letter E with diaeresis
    mapper[0xCC] = "&Ecaron;";  // latin capital letter E with caron
    mapper[0xCD] = "&Iacute;";  // latin capital letter I with acute
    mapper[0xCE] = "&Icirc;";   // latin capital letter I with circumflex
    mapper[0xCF] = "&Dcaron;";  // latin capital letter D with caron

    mapper[0xD0] = "&Dstrok;";  // latin capital letter D with stroke
    mapper[0xD1] = "&Nacute;";  // latin capital letter N with acute
    mapper[0xD2] = "&Ncaron;";  // latin capital letter N with caron
    mapper[0xD3] = "&Oacute;";  // latin capital letter O with acute
    mapper[0xD4] = "&Ocirc;";   // latin capital letter O with circumflex
    mapper[0xD5] = "&Odblac;";  // latin capital letter O with double acute
    mapper[0xD6] = "&Ouml;";    // latin capital letter O with diaeresis
    mapper[0xD7] = "&times;";   // multiplication sign
    mapper[0xD8] = "&Rcaron;";  // latin capital letter R with caron
    mapper[0xD9] = "&Uring;";   // latin capital letter U with ring above
    mapper[0xDA] = "&Uacute;";  // latin capital letter U with acute
    mapper[0xDB] = "&Udblac;";  // latin capital letter U with double acute
    mapper[0xDC] = "&Uuml;";    // latin capital letter U with diaeresis
    mapper[0xDD] = "&Yacute;";  // latin capital letter Y with acute
    mapper[0xDE] = "&Tcedil;";  // latin capital letter T with cedilla
    mapper[0xDF] = "&szlig;";   // latin small letter sharp s

    mapper[0xE0] = "&racute;";  // latin small letter r with acute
    mapper[0xE1] = "&aacute;";  // latin small letter a with acute
    mapper[0xE2] = "&acirc;";   // latin small letter a with circumflex
    mapper[0xE3] = "&abreve;";  // latin small letter a with breve
    mapper[0xE4] = "&auml;";    // latin small letter a with diaeresis
    mapper[0xE5] = "&lacute;";  // latin small letter l with acute
    mapper[0xE6] = "&cacute;";  // latin small letter c with acute
    mapper[0xE7] = "&ccedil;";  // latin small letter c with cedilla
    mapper[0xE8] = "&ccaron;";  // latin small letter c with caron
    mapper[0xE9] = "&eacute;";  // latin small letter e with acute
    mapper[0xEA] = "&eogon;";   // latin small letter e with ogonek
    mapper[0xEB] = "&euml;";    // latin small letter e with diaeresis
    mapper[0xEC] = "&ecaron;";  // latin small letter e with caron
    mapper[0xED] = "&iacute;";  // latin small letter i with acute
    mapper[0xEE] = "&icirc;";   // latin small letter i with circumflex
    mapper[0xEF] = "&dcaron;";  // latin small letter d with caron

    mapper[0xF0] = "&dstrok;";  // latin small letter d with stroke
    mapper[0xF1] = "&nacute;";  // latin small letter n with acute
    mapper[0xF2] = "&ncaron;";  // latin small letter n with caron
    mapper[0xF3] = "&oacute;";  // latin small letter o with acute
    mapper[0xF4] = "&ocirc;";   // latin small letter o with circumflex
    mapper[0xF5] = "&odblac;";  // latin small letter o with double acute
    mapper[0xF6] = "&ouml;";    // latin small letter o with diaeresis
    mapper[0xF7] = "&divide;";  // division sign
    mapper[0xF8] = "&rcaron;";  // latin small letter r with caron
    mapper[0xF9] = "&Uring;";   // latin small letter u with ring above
    mapper[0xFA] = "&uacute;";  // latin small letter u with acute
    mapper[0xFB] = "&udblac;";  // latin small letter u with double acute
    mapper[0xFC] = "&uuml;";    // latin small letter u with diaeresis
    mapper[0xFD] = "&yacute;";  // latin small letter y with acute
    mapper[0xFE] = "&tcedil;";  // latin small letter t with cedilla
    mapper[0xFF] = "&dot;";     // dot above

    // Map
    QByteArray ret;
    for (int idx = 0;
         mcText.at(idx) != '\0';
         idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Message(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert ISO-8859-2 (binary, unquoted representation) to UTF-8
QByteArray StringHelper::ConvertISO8859_2ToUTF8(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    const QString reason = tr("Not yet implemented.");
    MessageLogger::Error(CALL_METHOD, reason);
    CALL_OUT(reason);
    return mcText;
}



///////////////////////////////////////////////////////////////////////////////
// Convert ISO-8859-15 (binary, unquoted representation) to ISO-8859-1
QByteArray StringHelper::ConvertISO8859_15ToISO8859_1(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // From https://github.com/sebkirche/pbniregex/blob/master/stuff/
    //      CP1252%20%20%20ISO-8859-1%20%20%20UTF-8%20Conversion%20Chart.htm

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // 0x00 to 0x7F are all identical
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg((char)character);
    }

    // 0x80 to 0x9F are not used in ISO-8859-15

    // 0xA0 to 0xFF - same with exceptions
    // (need to use "int" in the loop below because in unsigned ints, (255)++
    // is zero, and the loop would run forever)
    for (int character = 0xA0;
        character <= 0xFF;
        character++)
    {
        mapper[character] = QString("%1").arg((char)character);
    }
    // Here are the exceptions:
    mapper[0xA4] = "&euro;";        // = Euro symbol
    mapper[0xA6] = "&Scaron;";      // = Latin capital S w/ caron
    mapper[0xA8] = "&scaron;";      // = Latin lower case s w/ caron
    mapper[0xB4] = "&Zcaron;";      // = Latin capital Z w/ caron
    mapper[0xB8] = "&zcaron;";      // = Latin lower case z w/ caron
    mapper[0xBC] = "&OElig;";       // = Latin capital ligature OE
    mapper[0xBD] = "&oelig;";       // = Latin lower case ligature oe
    mapper[0xBE] = "&Yuml;";        // = Latin capital Y w/ diaeresis

    // Map
    QByteArray ret;
    for (int idx = 0;
         mcText.at(idx) != '\0';
         idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Message(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert ISO-8859-15 (binary, unquoted representation) to UTF-8
QByteArray StringHelper::ConvertISO8859_15ToUTF8(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    const QString reason = tr("Not yet implemented.");
    MessageLogger::Error(CALL_METHOD, reason);
    CALL_OUT(reason);
    return mcText;
}



///////////////////////////////////////////////////////////////////////////////
// Convert Roman-8 (binary, unquoted representation) to ISO-8859-1
QByteArray StringHelper::ConvertRoman8ToISO8859_1(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // https://en.wikipedia.org/wiki/HP_Roman

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // 0x00 to 0x7F are all identical
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg((char)character);
    }

    // 0x80 to 0x9F are control characters
    for (unsigned char character = 0x80;
        character <= 0x9F;
        character++)
    {
        mapper[character] = QString();
    }

    // 0xA0 to 0xFF - mapped
    mapper[0xA0] = "&nbsp;";    // Non-breakable space
    mapper[0xA1] = QString("%1").arg((char)0xC0);    // A grave
    mapper[0xA2] = QString("%1").arg((char)0xC2);    // A circumflex
    mapper[0xA3] = QString("%1").arg((char)0xC8);    // E grave
    mapper[0xA4] = QString("%1").arg((char)0xCA);    // E circumflex
    mapper[0xA5] = QString("%1").arg((char)0xCB);    // E umlaut
    mapper[0xA6] = QString("%1").arg((char)0xCE);    // I circumflex
    mapper[0xA7] = QString("%1").arg((char)0xCF);    // I umlaut
    mapper[0xA8] = QString("%1").arg((char)0xB4);    // Spacing acute
    mapper[0xA9] = QString("%1").arg((char)0x60);    // Spacing grave
    mapper[0xAA] = "&circ;";                    // Spacing circumflex
    mapper[0xAB] = QString("%1").arg((char)0xA8);    // Spacing Diaresis
    mapper[0xAC] = "&tilde;";                   // Spacing tilde
    mapper[0xAD] = QString("%1").arg((char)0xD9);    // U grave
    mapper[0xAE] = QString("%1").arg((char)0xDB);    // U circumflex
    mapper[0xAF] = QString("%1").arg((char)0xA3);    // Pound sign

    mapper[0xB0] = QString("%1").arg((char)0xAF);    // Spacing macron
    mapper[0xB1] = QString("%1").arg((char)0xDD);    // Y acute
    mapper[0xB2] = QString("%1").arg((char)0xFD);    // y acute
    mapper[0xB3] = QString("%1").arg((char)0xB0);    // Degree sign
    mapper[0xB4] = QString("%1").arg((char)0xC7);    // C cedilla
    mapper[0xB5] = QString("%1").arg((char)0xE7);    // c cedilla
    mapper[0xB6] = QString("%1").arg((char)0xD1);    // N tilde
    mapper[0xB7] = QString("%1").arg((char)0xF1);    // n tilde
    mapper[0xB8] = QString("%1").arg((char)0xA1);    // Inverted exclamation point
    mapper[0xB9] = QString("%1").arg((char)0xBF);    // Inverted question mark
    mapper[0xBA] = QString("%1").arg((char)0xA4);    // Currency sign
    mapper[0xBB] = QString("%1").arg((char)0xA3);    // Pound sign
    mapper[0xBC] = QString("%1").arg((char)0xA5);    // Yen sign
    mapper[0xBD] = QString("%1").arg((char)0xA7);    // Section sign
    mapper[0xBE] = "&fnof;";                    // Florin sign
    mapper[0xBF] = QString("%1").arg((char)0xA2);    // Cent symbol

    mapper[0xC0] = QString("%1").arg((char)0xE2);    // a circumflex
    mapper[0xC1] = QString("%1").arg((char)0xEA);    // e circumflex
    mapper[0xC2] = QString("%1").arg((char)0xF4);    // o circumflex
    mapper[0xC3] = QString("%1").arg((char)0xFB);    // u circumflex
    mapper[0xC4] = QString("%1").arg((char)0xE1);    // a acute
    mapper[0xC5] = QString("%1").arg((char)0xE9);    // e acute
    mapper[0xC6] = QString("%1").arg((char)0xF3);    // o acute
    mapper[0xC7] = QString("%1").arg((char)0xFA);    // u acute
    mapper[0xC8] = QString("%1").arg((char)0xE0);    // a grave
    mapper[0xC9] = QString("%1").arg((char)0xE8);    // e grave
    mapper[0xCA] = QString("%1").arg((char)0xF2);    // o grave
    mapper[0xCB] = QString("%1").arg((char)0xF9);    // u grave
    mapper[0xCC] = QString("%1").arg((char)0xE4);    // a umlaut
    mapper[0xCD] = QString("%1").arg((char)0xEB);    // e umlaut
    mapper[0xCE] = QString("%1").arg((char)0xF6);    // o umlaut
    mapper[0xCF] = QString("%1").arg((char)0xFC);    // u umlaut

    mapper[0xD0] = QString("%1").arg((char)0xC5);    // A ring
    mapper[0xD1] = QString("%1").arg((char)0xEE);    // i circumflex
    mapper[0xD2] = QString("%1").arg((char)0xD8);    // O slash
    mapper[0xD3] = QString("%1").arg((char)0xC6);    // AE ligature
    mapper[0xD4] = QString("%1").arg((char)0xE5);    // a ring
    mapper[0xD5] = QString("%1").arg((char)0xED);    // i aute
    mapper[0xD6] = QString("%1").arg((char)0xF8);    // o slash
    mapper[0xD7] = QString("%1").arg((char)0xE6);    // ae ligature
    mapper[0xD8] = QString("%1").arg((char)0xC4);    // A umlaut
    mapper[0xD9] = QString("%1").arg((char)0xEC);    // i grave
    mapper[0xDA] = QString("%1").arg((char)0xD6);    // O umlaut
    mapper[0xDB] = QString("%1").arg((char)0xDC);    // U umlaut
    mapper[0xDC] = QString("%1").arg((char)0xC9);    // E acute
    mapper[0xDD] = QString("%1").arg((char)0xEF);    // i umlaut
    mapper[0xDE] = QString("%1").arg((char)0xDF);    // sharp s
    mapper[0xDF] = QString("%1").arg((char)0xD4);    // O acute

    mapper[0xE0] = QString("%1").arg((char)0xC1);    // A acute
    mapper[0xE1] = QString("%1").arg((char)0xC3);    // A tilde
    mapper[0xE2] = QString("%1").arg((char)0xE3);    // a tilde
    mapper[0xE3] = QString("%1").arg((char)0xD0);    // ETH
    mapper[0xE4] = QString("%1").arg((char)0xF0);    // eth
    mapper[0xE5] = QString("%1").arg((char)0xCD);    // I aute
    mapper[0xE6] = QString("%1").arg((char)0xCC);    // I grave
    mapper[0xE7] = QString("%1").arg((char)0xD3);    // O acute
    mapper[0xE8] = QString("%1").arg((char)0xD2);    // O grave
    mapper[0xE9] = QString("%1").arg((char)0xD5);    // O tilde
    mapper[0xEA] = QString("%1").arg((char)0xF5);    // o tilde
    mapper[0xEB] = "&Scaron;";                  // latin capital S with caron
    mapper[0xEC] = "&scaron;";                  // latin lower case s w/ caron
    mapper[0xED] = QString("%1").arg((char)0xDA);    // latin capital U with acute
    mapper[0xEE] = "&Yuml;";                    // latin capital Y w/ diaresis
    mapper[0xEF] = QString("%1").arg((char)0xFF);    // y umlaut

    mapper[0xF0] = QString("%1").arg((char)0xDE);    // THORN
    mapper[0xF1] = QString("%1").arg((char)0xFE);    // thorn
    mapper[0xF2] = QString("%1").arg((char)0xB7);    // Middle dot
    mapper[0xF3] = QString("%1").arg((char)0xB5);    // Micro sign
    mapper[0xF4] = QString("%1").arg((char)0xB6);    // Paragraph sign
    mapper[0xF5] = QString("%1").arg((char)0xBE);    // Three quarters
    mapper[0xF6] = QString("%1").arg((char)0xAD);    // Soft hyphen
    mapper[0xF7] = QString("%1").arg((char)0xBC);    // One quarter
    mapper[0xF8] = QString("%1").arg((char)0xBD);    // One half
    mapper[0xF9] = QString("%1").arg((char)0xAA);    // Feminine ordinal
    mapper[0xFA] = QString("%1").arg((char)0xBA);    // Masculine ordinal
    mapper[0xFB] = QString("%1").arg((char)0xAB);    // Left angle quotes
    mapper[0xFC] = "&#x25A0;";                  // Black square
    mapper[0xFD] = QString("%1").arg((char)0xBB);    // Right angle quotes
    mapper[0xFE] = QString("%1").arg((char)0xB1);    // Plus-minus sign
    // 0xFF unused

    // Map
    QByteArray ret;
    for (int idx = 0;
         mcText.at(idx) != '\0';
         idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Error(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert UTF-8 (binary, unquoted representation) to ISO-8859-1
QByteArray StringHelper::ConvertUTF8ToISO8859_1(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    const QString reason = tr("Not yet implemented.");
    MessageLogger::Error(CALL_METHOD, reason);
    CALL_OUT(reason);
    return mcText;
}



///////////////////////////////////////////////////////////////////////////////
// Convert Windows-1252 (binary, unquoted representation) to ISO-8859-1
QByteArray StringHelper::ConvertWindows1252ToISO8859_1(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // Keep identical for 0x00 to 0x7f
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg((char)character);
    }

    // 0x80 to 0x9F are specific to Windows-1252
    mapper[0x80] = "&euro;";    // Euro sign
    // 0x81 not used
    mapper[0x82] = "&sbquo;";   // single low-9 quotation mark
    mapper[0x83] = "&fnof;";    // Latin small letter f with hook
    mapper[0x84] = "&bdquo;";   // double low-9 quotation mark
    mapper[0x85] = "&hellip;";  // horizontal ellipsis
    mapper[0x86] = "&dagger;";  // dagger
    mapper[0x87] = "&Dagger;";  // double dagger
    mapper[0x88] = "&circ;";    // modifier letter circumflex accent
    mapper[0x89] = "&permil;";  // per mille sign
    mapper[0x8A] = "&Scaron;";  // Latin capital S w/ caron
    mapper[0x8B] = "&lsaquo;";  // single left angle quotation mark
    mapper[0x8C] = "&OElig;";   // Latin capital ligature OE
    // 0x8D not used
    mapper[0x8E] = "&Zcaron;";   // Latin capital Z w/ caron
    // 0x8F not used

    // 0x90 not used
    mapper[0x91] = "&lsquo;";   // left single quotation mark
    mapper[0x92] = "&rsquo;";   // right single quotation mark
    mapper[0x93] = "&ldquo;";   // left double quotation mark
    mapper[0x94] = "&rdquo;";   // right double quotation mark
    mapper[0x95] = "&bull;";    // bullet
    mapper[0x96] = "&ndash;";   // en dash
    mapper[0x97] = "&mdash;";   // em dash
    mapper[0x98] = "&tilde;";   // small tilde
    mapper[0x99] = "&trade;";   // trade mark sign
    mapper[0x9A] = "&scaron;";  // Latin small s with caron
    mapper[0x9B] = "&rsaquo;";  // single right angle quot. mark
    mapper[0x9C] = "&ealig;";   // Latin small ligature oe
    // 0x9D not used
    mapper[0x9E] = "&zcaron";   // Latin small z with caron
    mapper[0x9F] = "&Yuml;";    // Latin capital Y w/ diaeresis

    // 0xA0 to 0xFF - identical
    // (need to use int because (255)++ == 0 in unsigned int, and the loop
    // goes on forever)
    for (int character = 0xA0;
        character <= 0xFF;
        character++)
    {
        mapper[(unsigned int)character] = QString("%1").arg((char)character);
    }

    // Map
    QByteArray ret;
    for (int idx = 0; mcText.at(idx) != '\0'; idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Message(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert Windows-1252 (binary, unquoted representation) to UTF-8
QByteArray StringHelper::ConvertWindows1252ToUTF8(const QByteArray mcText)
{
    CALL_IN(QString("mcText=%1")
        .arg(CALL_SHOW(mcText)));

    // Create character mapper
    QHash < unsigned char, QString > mapper;

    // Keep identical for 0x00 to 0x7f
    for (unsigned char character = 0x00;
        character <= 0x7F;
        character++)
    {
        mapper[character] = QString("%1").arg(QChar(character));
    }

    // 0x80 to 0x9F are specific to Windows-1252
    mapper[0x80] = "&euro;";    // Euro sign
    // 0x81 not used
    mapper[0x82] = "&sbquo;";   // single low-9 quotation mark
    mapper[0x83] = "&fnof;";    // Latin small letter f with hook
    mapper[0x84] = "&bdquo;";   // double low-9 quotation mark
    mapper[0x85] = "&hellip;";  // horizontal ellipsis
    mapper[0x86] = "&dagger;";  // dagger
    mapper[0x87] = "&Dagger;";  // double dagger
    mapper[0x88] = "&circ;";    // modifier letter circumflex accent
    mapper[0x89] = "&permil;";  // per mille sign
    mapper[0x8A] = QString("%1").arg(QChar(0xC5A0)); // Latin capital S w/ caron
    mapper[0x8B] = "&lsaquo;";  // single left angle quotation mark
    mapper[0x8C] = QString("%1").arg(QChar(0xC592)); // Latin capital ligature OE
    // 0x8D not used
    mapper[0x8E] = QString("%1").arg(QChar(0xC5BD)); // Latin capital Z w/ caron
    // 0x8F not used

    // 0x90 not used
    mapper[0x91] = "&lsquo;";   // left single quotation mark
    mapper[0x92] = "&rsquo;";   // right single quotation mark
    mapper[0x93] = "&ldquo;";   // left double quotation mark
    mapper[0x94] = "&rdquo;";   // right double quotation mark
    mapper[0x95] = "&bull;";    // bullet
    mapper[0x96] = "&ndash;";   // en dash
    mapper[0x97] = "&mdash;";   // em dash
    mapper[0x98] = "&tilde;";   // small tilde
    mapper[0x99] = "&trade;";   // trade mark sign
    mapper[0x9A] = QString("%1").arg(QChar(0xC5A1)); // Latin small s with caron
    mapper[0x9B] = "&rsaquo;";  // single right angle quot. mark
    mapper[0x9C] = QString("%1").arg(QChar(0xC593)); // Latin small ligature oe
    // 0x9D not used
    mapper[0x9E] = QString("%1").arg(QChar(0xC5BE)); // Latin small z with caron
    mapper[0x9F] = QString("%1").arg(QChar(0xC5B8)); // Latin capital Y w/ diaeresis

    // 0xA0 to 0xFF - translated
    mapper[0xA0] = "&nbsp;";
    mapper[0xA1] = QString("%1").arg(QChar(0xc2a0));   // Inverted exclamation point
    mapper[0xA2] = "&cent;";        // Cent symbol
    mapper[0xA3] = "&pound;";       // Pound symbol
    mapper[0xA4] = "&curren;";      // Currency sign
    mapper[0xA5] = "&yen;";         // Yen sign
    mapper[0xA6] = "&brvbar;";      // Broken bar
    mapper[0xA7] = "&sect;";        // Section sign
    mapper[0xA8] = "&uml;";         // Umlaut or diaresis
    mapper[0xA9] = "&copy;";        // Copyright sign
    mapper[0xAA] = "&ordf;";        // Feminine ordinal
    mapper[0xAB] = "&laquo;";       // Left angle quotes
    mapper[0xAC] = "&not;";         // Logical not sign
    mapper[0xAD] = "&shy;";         // Soft hyphen
    mapper[0xAE] = "&reg;";         // Registered trademark
    mapper[0xAF] = "&macr;";        // Spacing macron

    mapper[0xB0] = "&deg;";         // Degree sign
    mapper[0xB1] = "&plusmn;";      // Plus-minus sign
    mapper[0xB2] = "&sup2;";        // Superscript 2
    mapper[0xB3] = "&sup3;";        // Superscript 3
    mapper[0xB4] = "&acute;";       // Spacing acute
    mapper[0xB5] = "&micro;";       // Micro sign
    mapper[0xB6] = "&para;";        // Paragraph sign
    mapper[0xB7] = "&middot;";      // Middle dot
    mapper[0xB8] = "&cedil;";       // Spacing cedilla
    mapper[0xB9] = "&sup1;";        // Superscript 1
    mapper[0xBA] = "&ordm;";        // Masculine ordinal
    mapper[0xBB] = "&raquo;";       // Right angle quotes
    mapper[0xBC] = "&frac14;";      // One quarter
    mapper[0xBD] = "&frac12;";      // One half
    mapper[0xBE] = "&frac34;";      // Three quarters
    mapper[0xBF] = QString("%1").arg(QChar(0xc2bf));   // Inverted question mark

    mapper[0xC0] = QString("%1").arg(QChar(0xC380)); // A grave
    mapper[0xC1] = QString("%1").arg(QChar(0xC381)); // A acute
    mapper[0xC2] = QString("%1").arg(QChar(0xC382)); // A circumflex
    mapper[0xC3] = QString("%1").arg(QChar(0xC383)); // A tilde
    mapper[0xC4] = QString("%1").arg(QChar(0xC384)); // A umlaut
    mapper[0xC5] = QString("%1").arg(QChar(0xC385)); // A ring
    mapper[0xC6] = QString("%1").arg(QChar(0xC386)); // AE ligature
    mapper[0xC7] = QString("%1").arg(QChar(0xC387)); // C cedilla
    mapper[0xC8] = QString("%1").arg(QChar(0xC388)); // E grave
    mapper[0xC9] = QString("%1").arg(QChar(0xC389)); // E acute
    mapper[0xCA] = QString("%1").arg(QChar(0xC38A)); // E circumflex
    mapper[0xCB] = QString("%1").arg(QChar(0xC38B)); // E umlaut
    mapper[0xCC] = QString("%1").arg(QChar(0xC38C)); // I grave
    mapper[0xCD] = QString("%1").arg(QChar(0xC38D)); // I aute
    mapper[0xCE] = QString("%1").arg(QChar(0xC38E)); // I circumflex
    mapper[0xCF] = QString("%1").arg(QChar(0xC38F)); // I umlaut

    mapper[0xD0] = QString("%1").arg(QChar(0xC390)); // ETH
    mapper[0xD1] = QString("%1").arg(QChar(0xC391)); // N tilde
    mapper[0xD2] = QString("%1").arg(QChar(0xC392)); // O grave
    mapper[0xD3] = QString("%1").arg(QChar(0xC393)); // O acute
    mapper[0xD4] = QString("%1").arg(QChar(0xC394)); // O circumflex
    mapper[0xD5] = QString("%1").arg(QChar(0xC395)); // O tilde
    mapper[0xD6] = QString("%1").arg(QChar(0xC396)); // O umlaut
    mapper[0xD7] = "&times;"; // Multiplication sign
    mapper[0xD8] = QString("%1").arg(QChar(0xC398)); // O slash
    mapper[0xD9] = QString("%1").arg(QChar(0xC399)); // U grave
    mapper[0xDA] = QString("%1").arg(QChar(0xC39A)); // U acute
    mapper[0xDB] = QString("%1").arg(QChar(0xC39B)); // U circumflex
    mapper[0xDC] = QString("%1").arg(QChar(0xC39C)); // U umlaut
    mapper[0xDD] = QString("%1").arg(QChar(0xC39D)); // Y acute
    mapper[0xDE] = QString("%1").arg(QChar(0xC39E)); // THORN
    mapper[0xDF] = QString("%1").arg(QChar(0xC39F)); // sharp s

    mapper[0xE0] = QString("%1").arg(QChar(0xC3A0)); // a grave
    mapper[0xE1] = QString("%1").arg(QChar(0xC3A1)); // a acute
    mapper[0xE2] = QString("%1").arg(QChar(0xC3A2)); // a circumflex
    mapper[0xE3] = QString("%1").arg(QChar(0xC3A3)); // a tilde
    mapper[0xE4] = QString("%1").arg(QChar(0xC3A4)); // a umlaut
    mapper[0xE5] = QString("%1").arg(QChar(0xC3A5)); // a ring
    mapper[0xE6] = QString("%1").arg(QChar(0xC3A6)); // ae ligature
    mapper[0xE7] = QString("%1").arg(QChar(0xC3A7)); // c cedilla
    mapper[0xE8] = QString("%1").arg(QChar(0xC3A8)); // e grave
    mapper[0xE9] = QString("%1").arg(QChar(0xC3A9)); // e acute
    mapper[0xEA] = QString("%1").arg(QChar(0xC3AA)); // e circumflex
    mapper[0xEB] = QString("%1").arg(QChar(0xC3AB)); // e umlaut
    mapper[0xEC] = QString("%1").arg(QChar(0xC3AC)); // i grave
    mapper[0xED] = QString("%1").arg(QChar(0xC3AD)); // i aute
    mapper[0xEE] = QString("%1").arg(QChar(0xC3AE)); // i circumflex
    mapper[0xEF] = QString("%1").arg(QChar(0xC3AF)); // i umlaut

    mapper[0xF0] = QString("%1").arg(QChar(0xC3B0)); // eth
    mapper[0xF1] = QString("%1").arg(QChar(0xC3B1)); // n tilde
    mapper[0xF2] = QString("%1").arg(QChar(0xC3B2)); // o grave
    mapper[0xF3] = QString("%1").arg(QChar(0xC3B3)); // o acute
    mapper[0xF4] = QString("%1").arg(QChar(0xC3B4)); // o circumflex
    mapper[0xF5] = QString("%1").arg(QChar(0xC3B5)); // o tilde
    mapper[0xF6] = QString("%1").arg(QChar(0xC3B6)); // o umlaut
    mapper[0xF7] = "&divide;"; // Division sign
    mapper[0xF8] = QString("%1").arg(QChar(0xC3B8)); // o slash
    mapper[0xF9] = QString("%1").arg(QChar(0xC3B9)); // u grave
    mapper[0xFA] = QString("%1").arg(QChar(0xC3BA)); // u acute
    mapper[0xFB] = QString("%1").arg(QChar(0xC3BB)); // u circumflex
    mapper[0xFC] = QString("%1").arg(QChar(0xC3BC)); // u umlaut
    mapper[0xFD] = QString("%1").arg(QChar(0xC3BD)); // y acute
    mapper[0xFE] = QString("%1").arg(QChar(0xC3BE)); // thorn
    mapper[0xFF] = QString("%1").arg(QChar(0xC3BF)); // y umlaut

    // Map
    QByteArray ret;
    for (int idx = 0; mcText.at(idx) != '\0'; idx++)
    {
        unsigned char single_char = (unsigned char)mcText.at(idx);
        if (mapper.contains(single_char))
        {
            ret += mapper[single_char].toUtf8();
        } else
        {
            const QString reason =
                tr("Text contains untranslated characters: %1 (%2)")
                    .arg(mcText.at(idx),
                         single_char);
            MessageLogger::Error(CALL_METHOD, reason);
            ret += QString("[untranslated]%1").arg(mcText.at(idx)).toUtf8();
        }
    }

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Mark search text
QString StringHelper::MarkSearchword(const QString mcText,
    const QString mcSearchText, const QString mcColor, const QString mcBgColor)
{
    CALL_IN(QString("mcText=%1, mcSearchText=%2, mcColor=%3, mcBgColor=%4")
        .arg(CALL_SHOW(mcText),
             CALL_SHOW(mcSearchText),
             CALL_SHOW(mcColor),
             CALL_SHOW(mcBgColor)));

    // Simple things...
    if (mcSearchText.isEmpty())
    {
        CALL_OUT("");
        return mcText;
    }
    const int size = mcSearchText.size();

    // Splitting is case-insensitive, but we preserve the original case
    // in the text that we display
    const QStringList parts = mcText.toLower().split(mcSearchText.toLower());
    int pos = 0;
    QString ret;
    for (int part_idx = 0; part_idx < parts.size(); part_idx++)
    {
        ret += mcText.mid(pos, parts[part_idx].size());
        pos += parts[part_idx].size();
        if (pos < mcText.size())
        {
            ret += "<span style=\"";
            if (!mcBgColor.isEmpty())
            {
                ret += QString("background-color: #%1").arg(mcBgColor);
            }
            if (!mcColor.isEmpty())
            {
                ret += (mcBgColor.isEmpty() ? "" : "; ") +
                    QString("color: #%1").arg(mcColor);
            }
            ret += QString("\">%1</span>").arg(mcText.mid(pos, size));
            pos += size;
        }
    }

    CALL_OUT("");
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Month to name
QString StringHelper::MonthName(const int mcNumber)
{
    CALL_IN(QString("mcNumber=%1")
        .arg(CALL_SHOW(mcNumber)));

    // Mapper
    static QHash < int, QString > number_to_name;
    if (number_to_name.isEmpty())
    {
        number_to_name[ 1] = "Jan";
        number_to_name[ 2] = "Feb";
        number_to_name[ 3] = "Mar";
        number_to_name[ 4] = "Apr";
        number_to_name[ 5] = "May";
        number_to_name[ 6] = "Jun";
        number_to_name[ 7] = "Jul";
        number_to_name[ 8] = "Aug";
        number_to_name[ 9] = "Sep";
        number_to_name[10] = "Oct";
        number_to_name[11] = "Nov";
        number_to_name[12] = "Dec";
    }
    if (number_to_name.contains(mcNumber))
    {
        CALL_OUT("");
        return number_to_name[mcNumber];
    } else
    {
        CALL_OUT("");
        return QString();
    }

    // We don't get here
}



///////////////////////////////////////////////////////////////////////////////
// Strip HTML tags from a string
QString StringHelper::StripHTMLTags(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    // Remove tags: <...>
    mString.replace("\n", "##newline");
    static const QRegularExpression format_tags("^(.*)<[^>]+>(.*)$");
    QRegularExpressionMatch format_match = format_tags.match(mString);
    while (format_match.hasMatch())
    {
        mString = format_match.captured(1) + format_match.captured(2);
        format_match = format_tags.match(mString);
    }
    mString.replace("##newline", "\n");

    // Replace some other stuff
    mString.replace("\r\n", "\n");
    mString.replace("\r", "\n");
    mString.replace("&nbsp;", " ");

    CALL_OUT("");
    return mString.trimmed();
}



///////////////////////////////////////////////////////////////////////////////
// Check proper nesting (HTML)
QPair < bool, QString > StringHelper::CheckProperHTMLNesting(QString mHTML)
{
    CALL_IN(QString("mHTML=%1")
        .arg(CALL_SHOW(mHTML)));

    // "Proper HTML Nesting" (vs. proper nesting) relates to the fact that
    // HTML isn't XML and does not at all enforce proper nesting. Example:
    // <br> will create a line break but will not usually have a corresponding
    // closing tag </br>. (<br/> works, though)

    // Remove all <br> tags
    mHTML.replace("<br>", "");

    // Remove all escaped quotation marks
    mHTML.replace("\\\"", "");

    static QRegularExpression format_next_tag(
        "^[^<]*(<(/)?([^> /]+)( [^>/\"]*| ([^>/]+=\"[^\"]*\")*)?(/)?>(.*))?$");
    QStringList open_tags;
    while (!mHTML.isEmpty())
    {
        QRegularExpressionMatch match_next_tag = format_next_tag.match(mHTML);
        if (!match_next_tag.hasMatch())
        {
            break;
        }
        if (match_next_tag.captured(1).isEmpty())
        {
            // Text only.
            CALL_OUT("");
            return QPair < int, QString >(true, "");
        }
        const QString close = match_next_tag.captured(2);
        const QString tag = match_next_tag.captured(3);
        const QString short_close = match_next_tag.captured(6);
        mHTML = match_next_tag.captured(7);
        if (close.isEmpty())
        {
            if (short_close.isEmpty())
            {
                // Opening tag: <a>
                open_tags << tag;
            } else
            {
                // Short tag: <a/>
                // (ignore)
            }
        } else
        {
            if (short_close.isEmpty())
            {
                // Closing tag: </a>
                if (open_tags.isEmpty())
                {
                    // No more open tags
                    CALL_OUT("");
                    return QPair < bool, QString >(false,
                        tr("Closing tag <%1> was never opened")
                            .arg(tag));
                }
                const QString matching_opening_tag = open_tags.takeLast();
                if (tag != matching_opening_tag)
                {
                    CALL_OUT("");
                    return QPair < bool, QString >(false,
                        tr("Closing tag <%1> does not match last opening "
                           "tag <%2>")
                            .arg(tag,
                                 matching_opening_tag));
                }
            } else
            {
                // Invalid tag format: </a/>
                CALL_OUT("");
                return QPair < bool, QString >(false,
                    tr("Malformed tag <%1%2%3>")
                        .arg(close,
                             tag,
                             short_close));
            }
        }
    }
    if (!mHTML.isEmpty())
    {
        // Rest of HTML did not match regular expression. Shouldn't really
        // happen.
        CALL_OUT("");
        return QPair < bool, QString >(false,
            tr("Residual HTML appears to be malformed: \"%1\"")
                .arg(EncodeToHTML(mHTML)));
    }
    if (!open_tags.isEmpty())
    {
        // Some tags still open
        CALL_OUT("");
        return QPair < bool, QString >(false,
            tr("Some tags were never closed: &lt;%1&gt;")
                .arg(open_tags.join("&lt;&gt;")));
    }

    CALL_OUT("");
    return QPair < bool, QString >(true, "");
}



///////////////////////////////////////////////////////////////////////////////
// Convert some characters to HTML so the text can be put into XML
QString StringHelper::EncodeToHTML(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    mString.replace("&", "&amp;");
    mString.replace("\"", "&quot;");
    mString.replace("'", "&#039;");
    mString.replace("<", "&lt;");
    mString.replace(">", "&gt;");
    mString.replace("\n", "<br/>\n");

    CALL_OUT("");
    return mString;
}



///////////////////////////////////////////////////////////////////////////////
// Convert back from the previous function
QString StringHelper::DecodeFromHTML(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    mString.replace("<br/>\n", "\n");
    mString.replace("&quot;", "\"");
    mString.replace("&#039;", "'");
    mString.replace("&lt;", "<");
    mString.replace("&gt;", ">");
    // Has to go last
    mString.replace("&amp;", "&");

    CALL_OUT("");
    return mString;
}



///////////////////////////////////////////////////////////////////////////////
// Convert some characters to % (for text inclusion in XML)
QString StringHelper::PercentEncode(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    mString.replace("%", "%25");

    mString.replace("\n", "%0a");
    mString.replace("\r", "%0d");
    mString.replace("<", "%3c");
    mString.replace(">", "%3e");
    mString.replace("\"", "%22");
    mString.replace("&", "%26");

    CALL_OUT("");
    return mString;
}



///////////////////////////////////////////////////////////////////////////////
// Convert some characters from % (for text inclusion in XML)
QString StringHelper::PercentDecode(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    mString.replace("%0a", "\n");
    mString.replace("%0d", "\r");
    mString.replace("%3c", "<");
    mString.replace("%3e", ">");
    mString.replace("%22", "\"");
    mString.replace("%26", "&");

    mString.replace("%25", "%");

    CALL_OUT("");
    return mString;
}



///////////////////////////////////////////////////////////////////////////////
// Convert to CSV
QString StringHelper::EncodeToCSV(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    // For the structure of a CSV file, see RFC-4180

    // Escape double quotes
    mString.replace("\"", "\"\"");

    // Enclose in quotes if it contains quotes or newlines
    if (mString.contains("\n") ||
        mString.contains(","))
    {
        mString = "\"" + mString + "\"";
    }

    // Done
    CALL_OUT("");
    return mString;
}



///////////////////////////////////////////////////////////////////////////////
// Convert UTF-8 to ASCII by stripping all non-existing characters
QString StringHelper::StripNonASCII(QString mString)
{
    CALL_IN(QString("mString=%1")
        .arg(CALL_SHOW(mString)));

    // Valid characters
    static QHash < QChar, QString > replace_characters;
    if (replace_characters.isEmpty())
    {
        // Initialize
        QString ascii(" !\"#$%&'()*+,-./0123456789:;<=>?@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "[\\]^_`’"
            "abcdefghijklmnopqsrtuvwxyz"
            "{|}~");
        for (int index = 0;
             index < ascii.size();
             index++)
        {
            const QChar character = ascii.at(index);
            replace_characters[character] = character;
        }
        replace_characters[QChar(0x0a)] = "\n"; // New line

        replace_characters[QChar(0xC0)] = "A"; // A grave
        replace_characters[QChar(0xC1)] = "A"; // A acute
        replace_characters[QChar(0xC2)] = "A"; // A circumflex
        replace_characters[QChar(0xC3)] = "A"; // A tilde
        replace_characters[QChar(0xC4)] = "Ae"; // A umlaut
        replace_characters[QChar(0xC5)] = "A"; // A ring
        replace_characters[QChar(0xC6)] = "AE"; // AE ligature
        replace_characters[QChar(0xC7)] = "C"; // C cedilla
        replace_characters[QChar(0xC8)] = "E"; // E grave
        replace_characters[QChar(0xC9)] = "E"; // E acute
        replace_characters[QChar(0xCA)] = "E"; // E circumflex
        replace_characters[QChar(0xCB)] = "E"; // E umlaut
        replace_characters[QChar(0xCC)] = "I"; // I grave
        replace_characters[QChar(0xCD)] = "I"; // I aute
        replace_characters[QChar(0xCE)] = "I"; // I circumflex
        replace_characters[QChar(0xCF)] = "I"; // I umlaut
        replace_characters[QChar(0xD1)] = "N"; // N tilde
        replace_characters[QChar(0xD2)] = "O"; // O grave
        replace_characters[QChar(0xD3)] = "O"; // O acute
        replace_characters[QChar(0xD4)] = "O"; // O circumflex
        replace_characters[QChar(0xD5)] = "O"; // O tilde
        replace_characters[QChar(0xD6)] = "Oe"; // O umlaut
        replace_characters[QChar(0xD8)] = "O"; // O slash
        replace_characters[QChar(0xD9)] = "U"; // U grave
        replace_characters[QChar(0xDA)] = "U"; // U acute
        replace_characters[QChar(0xDB)] = "U"; // U circumflex
        replace_characters[QChar(0xDC)] = "Ue"; // U umlaut
        replace_characters[QChar(0xDD)] = "Y"; // Y acute
        replace_characters[QChar(0xDF)] = "ss"; // sharp s
        replace_characters[QChar(0xE0)] = "a"; // a grave
        replace_characters[QChar(0xE1)] = "a"; // a acute
        replace_characters[QChar(0xE2)] = "a"; // a circumflex
        replace_characters[QChar(0xE3)] = "a"; // a tilde
        replace_characters[QChar(0xE4)] = "ae"; // a umlaut
        replace_characters[QChar(0xE5)] = "a"; // a ring
        replace_characters[QChar(0xE6)] = "ae"; // ae ligature
        replace_characters[QChar(0xE7)] = "c"; // c cedilla
        replace_characters[QChar(0xE8)] = "e"; // e grave
        replace_characters[QChar(0xE9)] = "e"; // e acute
        replace_characters[QChar(0xEA)] = "e"; // e circumflex
        replace_characters[QChar(0xEB)] = "e"; // e umlaut
        replace_characters[QChar(0xEC)] = "i"; // i grave
        replace_characters[QChar(0xED)] = "i"; // i aute
        replace_characters[QChar(0xEE)] = "i"; // i circumflex
        replace_characters[QChar(0xEF)] = "i"; // i umlaut
        replace_characters[QChar(0xF1)] = "n"; // n tilde
        replace_characters[QChar(0xF2)] = "o"; // o grave
        replace_characters[QChar(0xF3)] = "o"; // o acute
        replace_characters[QChar(0xF4)] = "o"; // o circumflex
        replace_characters[QChar(0xF5)] = "o"; // o tilde
        replace_characters[QChar(0xF6)] = "oe"; // o umlaut
        replace_characters[QChar(0xF8)] = "o"; // o slash
        replace_characters[QChar(0xF9)] = "u"; // u grave
        replace_characters[QChar(0xFA)] = "u"; // u acute
        replace_characters[QChar(0xFB)] = "u"; // u circumflex
        replace_characters[QChar(0xFC)] = "ue"; // u umlaut
        replace_characters[QChar(0xFD)] = "y"; // y acute
        replace_characters[QChar(0xFF)] = "y"; // y umlaut
    }

    QString ret;
    for (int index = 0;
         index < mString.size();
         index++)
    {
        const QChar character = mString.at(index);
        if (replace_characters.contains(character))
        {
            ret += replace_characters[character];
        } else
        {
            ret += "_";
        }
    }

    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Convert long double to QString
QString StringHelper::ToString(const long double mcValue)
{
    CALL_IN(QString("mcValue=%1")
        .arg(CALL_SHOW(mcValue)));

    char buffer[50];
    int actual_size = snprintf(buffer, 50, "%.33Lf", mcValue);
    if (actual_size < 0 ||
        actual_size >= 50)
    {
        CALL_OUT("");
        return QString();
    }

    CALL_OUT("");
    return QString(buffer);
}



///////////////////////////////////////////////////////////////////////////////
// Convert QString to long double
long double StringHelper::ToLongDouble(const QString mcTextValue)
{
    CALL_IN(QString("mcTextValue=%1")
        .arg(CALL_SHOW(mcTextValue)));

    long double value;
    sscanf(mcTextValue.toStdString().data(), "%Lf", &value);

    CALL_OUT("");
    return value;
}



///////////////////////////////////////////////////////////////////////////////
// Convert integer into "first", "21st" etc.
QString StringHelper::ToNth(const int mcN)
{
    CALL_IN(QString("mcN=%1")
        .arg(CALL_SHOW(mcN)));

    // Does not work for negative numbers or zero
    if (mcN <= 0)
    {
        const QString reason = tr("mcN must be positive, but is %1.")
            .arg(QString::number(mcN));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QString();
    }

    if (mcN <= 12)
    {
        switch(mcN)
        {
        case 1:
            CALL_OUT("");
            return tr("first");
        case 2:
            CALL_OUT("");
            return tr("second");
        case 3:
            CALL_OUT("");
            return tr("third");
        case 4:
            CALL_OUT("");
            return tr("fourth");
        case 5:
            CALL_OUT("");
            return tr("fifth");
        case 6:
            CALL_OUT("");
            return tr("sixth");
        case 7:
            CALL_OUT("");
            return tr("seventh");
        case 8:
            CALL_OUT("");
            return tr("eighth");
        case 9:
            CALL_OUT("");
            return tr("ninth");
        case 10:
            CALL_OUT("");
            return tr("tenth");
        case 11:
            CALL_OUT("");
            return tr("eleventh");
        case 12:
            CALL_OUT("");
            return tr("twelfth");
        }

        // We never get here
    }

    // Bigger numbers
    switch (mcN % 10)
    {
    case 1:
        CALL_OUT("");
        return tr("%1st")
            .arg(QString::number(mcN));
    case 2:
        CALL_OUT("");
        return tr("%1nd")
            .arg(QString::number(mcN));
    case 3:
        CALL_OUT("");
        return tr("%1rd")
            .arg(QString::number(mcN));
    default:
        CALL_OUT("");
        return tr("%1th")
            .arg(QString::number(mcN));
    }

    // We never get here
}



///////////////////////////////////////////////////////////////////////////////
// Return unique elements of a list
QList < QString > StringHelper::UniqueElements(
    const QList < QString > mcValues)
{
    CALL_IN(QString("mcValues=%1")
        .arg(CALL_SHOW(mcValues)));

    const QSet < QString > unique_values(mcValues.begin(), mcValues.end());
    const QList < QString > ret(unique_values.begin(), unique_values.end());

    CALL_OUT("");
    return ret;
}



// ============================================================== Format stuff



///////////////////////////////////////////////////////////////////////////////
// Check if a date has a valid format
bool StringHelper::IsValidDate(const QString & mcrDate)
{
    CALL_IN(QString("mcrDate=%1")
        .arg(CALL_SHOW(mcrDate)));

    const QDate date = QDate::fromString(mcrDate, "yyyy-MM-dd");
    const bool is_valid = date.isValid();

    CALL_OUT("");
    return is_valid;
}



///////////////////////////////////////////////////////////////////////////////
// Check if a date/time has a valid format
bool StringHelper::IsValidDateTime(const QString & mcrDateTime)
{
    CALL_IN(QString("mcrDateTime=%1")
        .arg(CALL_SHOW(mcrDateTime)));

    const QDateTime date_time =
        QDateTime::fromString(mcrDateTime, "yyyy-MM-dd hh:mm:ss");
    const bool is_valid = date_time.isValid();

    CALL_OUT("");
    return is_valid;
}



// ========================================================== Photography stuff



///////////////////////////////////////////////////////////////////////////////
// Convert exposure time to double
double StringHelper::ConvertExposureTimeToDouble(const QString mcExposureTime)
{
    CALL_IN(QString("mcExposureTime=%1")
        .arg(CALL_SHOW(mcExposureTime)));

    // Format: 1/15
    static QRegularExpression format("1/([1-9][0-9]*|[1-9][0-9]*\\.[0-9]*)");
    QRegularExpressionMatch format_match = format.match(mcExposureTime);
    if (format_match.hasMatch())
    {
        CALL_OUT("");
        return 1. / format_match.captured(1).toDouble();
    }

    // Format: 10
    format = QRegularExpression("([1-9][0-9]*|[1-9][0-9]*\\.[0-9]*)");
    format_match = format.match(mcExposureTime);
    if (format_match.hasMatch())
    {
        CALL_OUT("");
        return format_match.captured(1).toDouble();
    }

    // Unknown format
    CALL_OUT("");
    return double(NAN);
}



///////////////////////////////////////////////////////////////////////////////
// Convert f stop to double
double StringHelper::ConvertFStopToDouble(const QString mcFStop)
{
    CALL_IN(QString("mcFStop=%1")
        .arg(CALL_SHOW(mcFStop)));

    // Format: f/4.5
    static const QRegularExpression format(
        "f/([1-9][0-9]*|[1-9][0-9]*\\.[0-9]*)");
    QRegularExpressionMatch format_match = format.match(mcFStop);
    if (format_match.hasMatch())
    {
        CALL_OUT("");
        return format_match.captured(1).toDouble();
    }

    // Unknown format
    CALL_OUT("");
    return double(NAN);
}
