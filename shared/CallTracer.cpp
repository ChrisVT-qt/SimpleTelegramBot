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

// CallTracer.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "Deploy.h"
#include "StringHelper.h"

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QUrl>



// CallTracer cannot utilize any methods in classes that utilize CallTracer
// to avoid infinite loops


// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
CallTracer::CallTracer()
{
    // Nothing to do
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
CallTracer::~CallTracer()
{
    // Nothing to do
}



// ================================================================ Call Stack



///////////////////////////////////////////////////////////////////////////////
// Reset history
void CallTracer::ResetHistory()
{
    m_CallStack_Time.clear();
    m_CallStack_Method.clear();
    m_CallStack_Text.clear();
    m_CallCount.clear();
    m_OriginatorCount.clear();
}



///////////////////////////////////////////////////////////////////////////////
// Set keeping all history
void CallTracer::SetKeepAllHistory(const bool mcKeepHistory)
{
    m_KeepAllHistory = mcKeepHistory;
}



///////////////////////////////////////////////////////////////////////////////
// Enter function
void CallTracer::EnterFunction(const QString mcFilename,
    const QString mcFunction, const QString mcParameters)
{
    const QString timestamp =
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString class_name = ClassName(mcFilename);

    // Originator (method that called the current method)
    QString caller_method;
    if (!m_CallStack_Method.isEmpty())
    {
        caller_method = m_CallStack_Method.last();
    }

    // Call count
    const QString called_method = QString("%1::%2")
        .arg(class_name,
            mcFunction);
    m_CallStack_Time << timestamp;
    m_CallStack_Method << called_method;
    m_CallStack_Text << QString("(%1)").arg(mcParameters);
    m_CallCount[class_name][mcFunction]++;

    // Log originator
    m_OriginatorCount[called_method][caller_method]++;

    // Print on screen if required
    if (m_IsVerbose)
    {
        qDebug().noquote() << tr("Enter: %1 %2%3")
            .arg(m_CallStack_Time.last(),
                m_CallStack_Method.last(),
                m_CallStack_Text.last());
    }
}



///////////////////////////////////////////////////////////////////////////////
// Exit function
void CallTracer::ExitFunction(const QString mcFilename,
    const QString mcFunction, const int mcLine, const QString mcReason)
{
    // Full method name
    const QString class_name = ClassName(mcFilename);
    const QString full_method = QString("%1::%2")
        .arg(class_name,
            mcFunction);

    // Check if we just ran out of stack
    // (happens if we forget to have a CALL_IN() but we do a CALL_OUT()
    if (m_CallStack_Method.isEmpty())
    {
        // That shouldn't happen!
        qDebug().noquote() << tr("CallTracer::ExitFunction(): Ran out of "
            "stack when exiting \"%1\" - probably a missing CALL_IN().")
            .arg(full_method);
        return;
    }

    // Check if exiting function mathes last incoming method
    if (m_CallStack_Method.last() != full_method)
    {
        // That shouldn't happen!
        qDebug().noquote() << tr("CallTracer::ExitFunction(): Mismatching "
            "method names exiting method %1 (matching incoming method is %2)")
            .arg(full_method,
                m_CallStack_Method.last());
        return;
    }

    // Print on screen if required
    const QString timestamp = CALL_TIMESTAMP;
    if (m_IsVerbose)
    {
        if (mcReason.isEmpty())
        {
            qDebug().noquote() << tr("Exit: %1 %2()")
                .arg(timestamp,
                     full_method);
        } else
        {
            qDebug().noquote() << tr("Exit: %1 %2(): %3")
                .arg(timestamp,
                     full_method,
                     mcReason);
        }
    }

    if (m_KeepAllHistory)
    {
        m_CallStack_Time << timestamp;
        m_CallStack_Method << QString("%1 (%2)")
            .arg(full_method,
                 QString::number(mcLine));
        if (mcReason.isEmpty())
        {
            m_CallStack_Text << tr(": leaving");
        } else
        {
            m_CallStack_Text << tr(": leaving (%1)").arg(mcReason);
        }
    } else
    {
        m_CallStack_Time.takeLast();
        m_CallStack_Method.takeLast();
        m_CallStack_Text.takeLast();
    }
}



///////////////////////////////////////////////////////////////////////////////
// Return stack
QString CallTracer::GetCallTrace()
{
    QString trace = tr("--------- Trace start\n");
    for (int index = 0; index < m_CallStack_Time.size(); index++)
    {
        trace += QString("%1 %2%3\n")
            .arg(m_CallStack_Time[index],
                m_CallStack_Method[index],
                m_CallStack_Text[index]);
    }
    trace += tr("--------- Trace end\n\n");

    return trace;
}



///////////////////////////////////////////////////////////////////////////////
// Class name
QString CallTracer::ClassName(const QString mcFilename)
{
    QString class_name;
    const int index = mcFilename.lastIndexOf("/");
    if (index != -1)
    {
        class_name = mcFilename.mid(index+1);
    } else
    {
        class_name = mcFilename;
    }
    class_name.replace(".cpp", "");

    return class_name;
}



///////////////////////////////////////////////////////////////////////////////
// Call stacks
QList < QString > CallTracer::m_CallStack_Time;
QList < QString > CallTracer::m_CallStack_Method;
QList < QString > CallTracer::m_CallStack_Text;



///////////////////////////////////////////////////////////////////////////////
// Keeping history
bool CallTracer::m_KeepAllHistory = false;



// =============================================================== Method usage



///////////////////////////////////////////////////////////////////////////////
// Reset usage
void CallTracer::ResetUsage(const QString mcClass, const QString mcMethod)
{
    if (mcClass.isEmpty())
    {
        m_CallCount.clear();
    } else
    {
        if (mcMethod.isEmpty())
        {
            m_CallCount[mcClass].clear();
        } else
        {
            m_CallCount[mcClass].remove(mcMethod);
        }
    }
}



///////////////////////////////////////////////////////////////////////////////
// Show message usage
void CallTracer::ShowUsage(const QString mcClass, const QString mcMethod)
{
    if (mcClass.isEmpty())
    {
        QList < QString > all_classes = m_CallCount.keys();
        std::sort(all_classes.begin(), all_classes.end());
        for (auto class_iterator = all_classes.begin();
             class_iterator != all_classes.end();
             class_iterator++)
        {
            const QString class_name = *class_iterator;
            ShowUsage(class_name);
        }
    } else
    {
        if (mcMethod.isEmpty())
        {
            QList < QString > all_methods = m_CallCount[mcClass].keys();
            std::sort(all_methods.begin(), all_methods.end());
            for (auto method_iterator = all_methods.begin();
                 method_iterator != all_methods.end();
                 method_iterator++)
            {
                const QString method_name = *method_iterator;
                ShowUsage(mcClass, method_name);
            }
        } else
        {
            const QString count =
                "      " + QString::number(m_CallCount[mcClass][mcMethod]);
            qDebug().noquote() << QString("%1: %2::%3()")
                .arg(count.right(7),
                     mcClass,
                     mcMethod);
        }
    }
}



///////////////////////////////////////////////////////////////////////////////
// Show call originator analysis
void CallTracer::ShowCallOriginators(const QString mcClass,
    const QString mcMethod)
{
    // Check if we know this method
    const QString called_method = QString("%1::%2")
        .arg(mcClass,
            mcMethod);

    qDebug().noquote() << tr("Caller statistics for %1").arg(called_method);

    if (!m_OriginatorCount.contains(called_method))
    {
        qDebug().noquote() << tr("  This method has never been called.");
        return;
    }

    // Sort by frequency
    QList < QString > sorted_keys =
        StringHelper::SortHash(m_OriginatorCount[called_method]);
    while (!sorted_keys.isEmpty())
    {
        const QString calling_method = sorted_keys.takeLast();
        const QString count = "      " +
            QString::number(m_OriginatorCount[called_method][calling_method]);
        qDebug().noquote() << QString("%1: %2()")
            .arg(count.right(7),
                 calling_method);
    }
}



///////////////////////////////////////////////////////////////////////////////
// Call count
QHash < QString, QHash < QString, int > > CallTracer::m_CallCount =
    QHash < QString, QHash < QString, int > > ();



///////////////////////////////////////////////////////////////////////////////
// Originator count
QHash < QString, QHash < QString, int > > CallTracer::m_OriginatorCount =
    QHash < QString, QHash < QString, int > > ();



///////////////////////////////////////////////////////////////////////////////
// Verbosity
void CallTracer::SetVerbosity(const bool mcNewVerbosity)
{
    m_IsVerbose = mcNewVerbosity;
}



///////////////////////////////////////////////////////////////////////////////
// Verbosity
bool CallTracer::m_IsVerbose = false;



// ================================================================ Convenience



///////////////////////////////////////////////////////////////////////////////
// Show bool
QString CallTracer::Show(const bool mcValue)
{
    return mcValue ? QString("true") : QString("false");
}



///////////////////////////////////////////////////////////////////////////////
// Show int
QString CallTracer::Show(const int mcValue)
{
    return QString::number(mcValue);
}



///////////////////////////////////////////////////////////////////////////////
// Show double
QString CallTracer::Show(const double mcValue)
{
    return QString::number(mcValue, 'g', 16);
}



///////////////////////////////////////////////////////////////////////////////
// Show long double
QString CallTracer::Show(const long double mcValue)
{
    return StringHelper::ToString(mcValue);
}



///////////////////////////////////////////////////////////////////////////////
// Show QByteArray
QString CallTracer::Show(const QByteArray & mcrValue)
{
    if (mcrValue.isNull())
    {
        return QString("null");
    }
    return QString("%1 bytes").arg(mcrValue.size());
}



///////////////////////////////////////////////////////////////////////////////
// Show QColor
QString CallTracer::Show(const QColor & mcrValue)
{
    if (mcrValue.isValid())
    {
        return QString("(%1, %2, %3; %4)")
            .arg(QString::number(mcrValue.red()),
                 QString::number(mcrValue.green()),
                 QString::number(mcrValue.blue()),
                 QString::number(mcrValue.alpha()));
    } else
    {
        return QString("(invalid color)");
    }
}



///////////////////////////////////////////////////////////////////////////////
// Show QDate
QString CallTracer::Show(const QDate & mcrValue)
{
    if (mcrValue.isValid())
    {
        return mcrValue.toString("yyyy-MM-dd");
    } else
    {
        return tr("(invalid date)");
    }
}



///////////////////////////////////////////////////////////////////////////////
// Show QDateTime
QString CallTracer::Show(const QDateTime & mcrValue)
{
    if (mcrValue.isValid())
    {
        return mcrValue.toString("yyyy-MM-dd hh:mm:ss");
    } else
    {
        return tr("(invalid date)");
    }
}



#ifdef QT_XML_LIB

///////////////////////////////////////////////////////////////////////////////
// Show QDomDocument
QString CallTracer::Show(const QDomDocument & mcrValue)
{
    Q_UNUSED(mcrValue);

    return QString("...");
}



///////////////////////////////////////////////////////////////////////////////
// Show QDomElement
QString CallTracer::Show(const QDomElement & mcrValue)
{
    if (mcrValue.isNull())
    {
        return QString("(null)");
    }
    QDomNamedNodeMap attributes = mcrValue.attributes();
    QStringList attributes_txt;
    for (int index = 0;
         index < attributes.size();
         index++)
    {
        QDomAttr attribute = attributes.item(index).toAttr();
        attributes_txt << QString(" %1=\"%2\"")
            .arg(attribute.name(),
                 attribute.value());
    }
    return QString("<%1%2>")
        .arg(mcrValue.tagName(),
             attributes_txt.join(""));
}



///////////////////////////////////////////////////////////////////////////////
// Show QDomNode
QString CallTracer::Show(const QDomNode & mcrValue)
{
    if (mcrValue.isNull())
    {
        return QString("(null)");
    }

    QString ret;
    if (mcrValue.isText())
    {
        ret = QString("(text) %1").arg(mcrValue.toText().data());
    } else if (mcrValue.isElement())
    {
        ret = QString("(element) %1").arg(CALL_SHOW(mcrValue.toElement()));
    } else if (mcrValue.isDocument())
    {
        ret = QString("(document) %1").arg(CALL_SHOW(mcrValue.toDocument()));
    } else
    {
        ret = QString("(unknown node type)");
    }

    ret.replace("\n", "\\n");
    if (ret.size() > 20)
    {
        ret = ret.left(20) + "...";
    }

    return QString("{%1}")
        .arg(ret);
}
#endif



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < double, double > & mcrValue)
{
    QStringList values;
    QList < double > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const double key = *key_iterator;
        double value = mcrValue[key];
        values << QString("%1: %2")
            .arg(QString::number(key, 'g', 6),
                 QString::number(value, 'g', 6));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < int, double > & mcrValue)
{
    QStringList values;
    QList < int > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const int key = *key_iterator;
        double value = mcrValue[key];
        values << QString("%1: %2")
            .arg(QString::number(key),
                 QString::number(value, 'g', 6));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < int, int > & mcrValue)
{
    QStringList values;
    QList < int > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const int key = *key_iterator;
        int value = mcrValue[key];
        values << QString("%1: %2")
            .arg(QString::number(key),
                 QString::number(value));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < int, QDateTime > & mcrValue)
{
    QStringList values;
    QList < int > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const int key = *key_iterator;
        QDateTime value = mcrValue[key];
        values << QString("%1: \"%2\"")
            .arg(QString::number(key),
                 value.isValid() ?
                    value.toString("yyyy-MM-dd hh:mm:ss.zzz") : "null");
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < int, QString > & mcrValue)
{
    QStringList values;
    QList < int > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const int key = *key_iterator;
        QString value = mcrValue[key];
        value.replace("\n", "\\n");
        if (value.size() > 20)
        {
            value = value.left(20) + "...";
        }
        values << QString("%1: \"%2\"")
            .arg(QString::number(key),
                 value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < QString, int > & mcrValue)
{
    QStringList values;
    QList < QString > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const QString key = *key_iterator;
        int value = mcrValue[key];
        values << QString("%1: %2")
            .arg(key,
                 QString::number(value));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
QString CallTracer::Show(const QHash < QString, QList < double > > & mcrValue)
{
    QStringList values;
    QList < QString > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const QString key = *key_iterator;
        values << QString("%1: {%2}")
            .arg(key,
                 CALL_SHOW(mcrValue[key]));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
QString CallTracer::Show(const QHash < QString, QList < QString > > & mcrValue)
{
    QStringList values;
    QList < QString > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const QString key = *key_iterator;
        values << QString("%1: {%2}")
            .arg(key,
                 CALL_SHOW(mcrValue[key]));
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QHash
QString CallTracer::Show(const QHash < QString, QString > & mcrValue)
{
    QStringList values;
    QList < QString > sorted_keys = mcrValue.keys();
    std::sort(sorted_keys.begin(), sorted_keys.end());
    for (auto key_iterator = sorted_keys.begin();
         key_iterator != sorted_keys.end();
         key_iterator++)
    {
        const QString key = *key_iterator;
        QString value = mcrValue[key];
        value.replace("\n", "\\n");
        if (value.size() > 20)
        {
            value = value.left(20) + "...";
        }
        values << QString("%1: \"%2\"")
            .arg(key,
                 value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QImage
QString CallTracer::Show(const QImage & mcrValue)
{
    if (mcrValue.isNull())
    {
        return QString("{null}");
    } else
    {
        return QString("{width:%1, height:%2}")
            .arg(QString::number(mcrValue.width()),
                 QString::number(mcrValue.height()));
    }

    // We never get here
}



///////////////////////////////////////////////////////////////////////////////
// Show qint64
QString CallTracer::Show(const qint64 mcValue)
{
    return QString::number(mcValue);
}



///////////////////////////////////////////////////////////////////////////////
// Show QJsonArray
QString CallTracer::Show(const QJsonArray & mcrValue,
    const bool mcTruncate)
{
    QJsonDocument json_doc(mcrValue);
    QString text = json_doc.toJson(QJsonDocument::Compact);
    if (mcTruncate &&
        text.size() > 20)
    {
        text = text.left(20) + "...";
    }
    return text;
}



///////////////////////////////////////////////////////////////////////////////
// Show QJsonObject
QString CallTracer::Show(const QJsonObject & mcrValue,
    const bool mcTruncate)
{
    QJsonDocument json_doc(mcrValue);
    QString text = json_doc.toJson(QJsonDocument::Compact);
    if (mcTruncate &&
        text.size() > 20)
    {
        text = text.left(20) + "...";
    }
    return text;
}



///////////////////////////////////////////////////////////////////////////////
// Show QJsonValue
QString CallTracer::Show(const QJsonValue & mcrValue,
    const bool mcTruncate)
{
    QString text;
    if (mcrValue.isObject())
    {
        QJsonDocument json_doc(mcrValue.toObject());
        text = json_doc.toJson(QJsonDocument::Compact);
    } else if(mcrValue.isArray())
    {
        QJsonDocument json_doc(mcrValue.toArray());
        text = json_doc.toJson(QJsonDocument::Compact);
    } else
    {
        return tr("Unknown JSON value type.");
    }
    if (mcTruncate &&
        text.size() > 20)
    {
        text = text.left(20) + "...";
    }
    return text;
}



///////////////////////////////////////////////////////////////////////////////
// Show QList < int >
QString CallTracer::Show(const QList < int > & mcrValue)
{
    QStringList values;
    for (const int value : mcrValue)
    {
        values << QString::number(value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QList < qint64 >
QString CallTracer::Show(const QList < qint64 > & mcrValue)
{
    QStringList values;
    for (const qint64 value : mcrValue)
    {
        values << QString::number(value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QList < QString >
QString CallTracer::Show(const QList < QString > & mcrValue)
{
    QStringList values;
    for (const QString & value : mcrValue)
    {
        QString short_value = value;
        short_value.replace("\n", "\\n");
        if (short_value.size() > 20)
        {
            short_value = short_value.left(20) + "...";
        }
        values << QString("\"%1\"")
            .arg(short_value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QList < QStringList >
QString CallTracer::Show(const QList < QStringList > & mcrValue)
{
    QStringList values;
    for (const QStringList & value : mcrValue)
    {
        values << Show(value);
    }
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < double, double > & mcrValue)
{
    return QString("{%1, %2}")
        .arg(QString::number(mcrValue.first, 'g', 6),
             QString::number(mcrValue.second, 'g', 6));
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < int, double > & mcrValue)
{
    return QString("{%1, %2}")
        .arg(QString::number(mcrValue.first),
             QString::number(mcrValue.second, 'g', 6));
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < int, int > & mcrValue)
{
    return QString("{%1, %2}")
        .arg(QString::number(mcrValue.first),
             QString::number(mcrValue.second));
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < int, QString > & mcrValue)
{
    QString short_value = mcrValue.second;
    short_value.replace("\n", "\\n");
    if (short_value.size() > 20)
    {
        short_value = short_value.left(20) + "...";
    }

    return QString("{%1, \"%2\"}")
        .arg(QString::number(mcrValue.first),
             short_value);
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < QString, int > & mcrValue)
{
    QString short_value = mcrValue.first;
    short_value.replace("\n", "\\n");
    if (short_value.size() > 20)
    {
        short_value = short_value.left(20) + "...";
    }

    return QString("{\"%1\", %2}")
        .arg(short_value,
             QString::number(mcrValue.second));
}



///////////////////////////////////////////////////////////////////////////////
// Show QPair
QString CallTracer::Show(const QPair < QString, QString > & mcrValue)
{
    QString left_value = mcrValue.first;
    left_value .replace("\n", "\\n");
    if (left_value .size() > 20)
    {
        left_value  = left_value.left(20) + "...";
    }
    QString right_value = mcrValue.second;
    right_value .replace("\n", "\\n");
    if (right_value .size() > 20)
    {
        right_value  = right_value.left(20) + "...";
    }

    return QString("{\"%1\", \"%2\"}")
        .arg(left_value,
             right_value);
}



///////////////////////////////////////////////////////////////////////////////
// Show QPixmap
QString CallTracer::Show(const QPixmap & mcrValue)
{
    if (mcrValue.isNull())
    {
        return QString("{null}");
    } else
    {
        return QString("{width:%1, height:%2}")
            .arg(QString::number(mcrValue.width()),
                 QString::number(mcrValue.height()));
    }

    // We never get here
}



///////////////////////////////////////////////////////////////////////////////
// Show QSet
QString CallTracer::Show(const QSet < double > & mcrValue)
{
    QStringList values;
    for (const int value : mcrValue)
    {
        values << QString::number(value);
    }
    std::sort(values.begin(), values.end());
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QSet
QString CallTracer::Show(const QSet < int > & mcrValue)
{
    QStringList values;
    for (const int value : mcrValue)
    {
        values << QString::number(value);
    }
    std::sort(values.begin(), values.end());
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show QSet
QString CallTracer::Show(const QSet < QString > & mcrValue)
{
    QStringList values;
    for (const QString & value : mcrValue)
    {
        QString short_value = value;
        short_value.replace("\n", "\\n");
        if (short_value.size() > 20)
        {
            short_value = short_value.left(20) + "...";
        }
        values << QString("\"%1\"")
            .arg(short_value);
    }
    std::sort(values.begin(), values.end());
    return QString("{%1}")
        .arg(values.join(", "));
}



#ifdef QT_SQL_LIB

///////////////////////////////////////////////////////////////////////////////
// Show QSqlQuery
QString CallTracer::Show(const QSqlQuery & mcrValue)
{
    // !!! Not sure if this is sufficient
    return mcrValue.lastQuery();

}

#endif



///////////////////////////////////////////////////////////////////////////////
// Show QString
QString CallTracer::Show(const QString & mcrValue, const bool mcTruncate)
{
    QString value = mcrValue;
    if (mcTruncate &&
        value.size() > 20)
    {
        value = value.left(17) + "...";
    }
    value.replace("\n", "\\n");
    value.replace("\r", "\\r");
    value.replace("\t", "\\t");
    return QString("\"%1\"")
        .arg(value);
}



///////////////////////////////////////////////////////////////////////////////
// Show QUrl
QString CallTracer::Show(const QUrl & mcrValue)
{
    return QString("\"%1\"")
        .arg(mcrValue.toString());
}



///////////////////////////////////////////////////////////////////////////////
// Show QVector
QString CallTracer::Show(const QVector < double > & mcrValue)
{
    QStringList values;
    for (const double value : mcrValue)
    {
        values << QString::number(value);
    }
    std::sort(values.begin(), values.end());
    return QString("{%1}")
        .arg(values.join(", "));
}



///////////////////////////////////////////////////////////////////////////////
// Show generic pointer address
QString CallTracer::Show(const void * mcpValue)
{
    return QString::number((unsigned long long)mcpValue, 16);
}
