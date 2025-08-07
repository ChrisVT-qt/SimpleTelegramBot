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

// TelegramComms.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "Config.h"
#include "DatabaseHelper.h"
#include "MessageLogger.h"
#include "StringHelper.h"
#include "TelegramComms.h"

// Qt includes
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>

#define DEBUG false



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
TelegramComms::TelegramComms()
{
    CALL_IN("");

    // Default preferences (also defined valid preference tags)
    m_DefaultPreferences["greedy"] = "no";
    m_DefaultPreferences["provide_sticker_set"] = "always";
    m_DefaultPreferences["silent"] = "no";

    // Create some directories
    QDir dir("/");
    if (!dir.exists(BOT_FILES))
    {
        dir.mkpath(BOT_FILES);
    }
    if (!dir.exists(BOT_DATABASE_DIR))
    {
        dir.mkpath(BOT_DATABASE_DIR);
    }

    // Database not connected
    m_DatabaseConnected = false;

    // Not running from the get go (needs to be configured first)
    m_IsRunning = false;

    // No previous updates
    m_OffsetSet = false;

    // Start with 0 with button list and button IDs
    m_NextButtonListID = 0;
    m_NextButtonID = 0;

    // Set up network access
    m_NetworkAccessManager = new QNetworkAccessManager(this);
    connect (m_NetworkAccessManager, SIGNAL(finished(QNetworkReply *)),
        this, SLOT(HandleResponse(QNetworkReply *)));

    // Timestamp for bot start
    m_StartDateTime = QDateTime::currentDateTime();

    // Start periodic updates
    // (won't actually do anything because m_IsRunning is false)
    Periodic_WatchForUpdates();
    Periodic_DownloadFiles();
    Periodic_DownloadStickerSetInfo();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
TelegramComms::~TelegramComms()
{
    CALL_IN("");

    delete m_NetworkAccessManager;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Instanciator
TelegramComms * TelegramComms::Instance()
{
    CALL_IN("");

    // Check if we already have an instance
    if (!m_Instance)
    {
        // Nope. Create one.
        m_Instance = new TelegramComms;
    }

    // Return instance
    CALL_OUT("");
    return m_Instance;
}



///////////////////////////////////////////////////////////////////////////////
// Instance
TelegramComms * TelegramComms::m_Instance = nullptr;



// =================================================================== Database



///////////////////////////////////////////////////////////////////////////////
// Set database name
void TelegramComms::SetDatabaseFile(const QString & mcrFilename)
{
    CALL_IN(QString("mcrFilename=%1")
        .arg(CALL_SHOW(mcrFilename)));

    // Check if filename is empty
    if (mcrFilename.isEmpty())
    {
        const QString reason = tr("Empty database filename provided.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    // Check if database is already connected
    if (m_DatabaseConnected)
    {
        const QString reason =
            tr("Cannot set database filename; database is already connected.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    // Set database filename
    m_DatabaseFilename = mcrFilename;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Open database
bool TelegramComms::OpenDatabase()
{
    CALL_IN("");

    // Check if database is already open
    if (m_DatabaseConnected)
    {
        const QString reason = tr("Cannot open an already open database.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Check if the filename has been set
    if (m_DatabaseFilename.isEmpty())
    {
        const QString reason = tr("No database name specified.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Check if the file exists
    const bool database_initialized = QFile::exists(m_DatabaseFilename);

    // Try to connect
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_DatabaseFilename);
    const bool successfully_connected = db.open();
    if (!successfully_connected)
    {
        const QString reason = tr("Could not open %1")
            .arg(m_DatabaseFilename);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    if (!database_initialized)
    {
        // Create database
        const bool success = CreateDatabase();
        if (!success)
        {
            const QString reason = tr("Could not set up database in %1")
                .arg(m_DatabaseFilename);
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return false;
        }
    }

    // Update database
    UpdateDatabase();

    // Database is connected now
    m_DatabaseConnected = true;

    // Read database
    const bool success = ReadDatabase();

    CALL_OUT("");
    return success;
}



///////////////////////////////////////////////////////////////////////////////
// Create database
bool TelegramComms::CreateDatabase()
{
    CALL_IN("");

    // Check if database is already connected
    if (m_DatabaseConnected)
    {
        const QString reason =
            tr("Cannot initialize database; it is already connected.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Create tables for Telegram data
    bool success =
        CreateDatabase_Table("button_info")
        && CreateDatabase_Table("button_list_info")
        && CreateDatabase_Table("channel_post_info")
        && CreateDatabase_Table("chat_info")
        && CreateDatabase_Table("file_info", "text")
        && CreateDatabase_Table("message_info")
        && CreateDatabase_Table("my_chat_member_info")
        && CreateDatabase_Table_StickerSet("sticker_set_info")
        && CreateDatabase_Table("update_info")
        && CreateDatabase_Table("user_info");

    // User preferences
    success = success
        && CreateDatabase_Preferences();

    // Successful
    CALL_OUT("");
    return success;
}



///////////////////////////////////////////////////////////////////////////////
// Create a particular table
bool TelegramComms::CreateDatabase_Table(const QString & mcrTableName,
    const QString & mcrIDType)
{
    CALL_IN(QString("mcrTableName=%1, mcrIDType=%2")
        .arg(CALL_SHOW(mcrTableName),
             CALL_SHOW(mcrIDType)));

    // Create table
    QSqlQuery query;
    query.exec(QString("CREATE TABLE %1 ("
       "id %2, "
       "key text, "
       "value text);")
       .arg(mcrTableName,
            mcrIDType));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error creating table \"%1\"")
            .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Create sticker set info table
bool TelegramComms::CreateDatabase_Table_StickerSet(
    const QString & mcrTableName)
{
    CALL_IN(QString("mcrTableName=%1")
        .arg(CALL_SHOW(mcrTableName)));

    // Create table
    QSqlQuery query;
    query.exec(QString("CREATE TABLE %1 ("
       "id text, "
       "sequence int, "
       "key text, "
       "value text);")
       .arg(mcrTableName));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error creating table \"%1\"")
            .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Create table for user preferences
bool TelegramComms::CreateDatabase_Preferences()
{
    CALL_IN("");

    // Create table
    QSqlQuery query;
    query.exec(QString("CREATE TABLE preferences ("
       "user_id longlong, "
       "key text, "
       "value text);"));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error creating table \"preferences\"");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Read database
bool TelegramComms::ReadDatabase()
{
    CALL_IN("");

    // Check if database has been connected
    if (!m_DatabaseConnected)
    {
        const QString reason =
            tr("Cannot read database; it has not been connected yet.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Read tables
    bool success =
        ReadDatabase_Table("button_info", m_ButtonIDToInfo) &&
        ReadDatabase_Table("button_list_info", m_ButtonListIDToInfo) &&
        ReadDatabase_Table("channel_post_info",
            m_MessageIDToChannelPostInfo) &&
        ReadDatabase_Table("chat_info", m_ChatIDToInfo) &&
        ReadDatabase_Table("file_info", m_FileIDToInfo) &&
        ReadDatabase_Table("message_info", m_MessageIDToInfo) &&
        ReadDatabase_Table("my_chat_member_info", m_MyChatMemberIDToInfo) &&
        ReadDatabase_Table_StickerSet("sticker_set_info") &&
        ReadDatabase_Table("update_info", m_UpdateIDToInfo) &&
        ReadDatabase_Table("user_info", m_UserIDToInfo);
    success = success &&
        ReadDatabase_Table_Preferences();
    if (!success)
    {
        // Error specifics have been reported elsewhere.
        CALL_OUT("");
        return false;
    }

    // Next offset
    QList < qint64 > ids;
    ids = QList < qint64 >(m_UpdateIDToInfo.keyBegin(),
        m_UpdateIDToInfo.keyEnd());
    if (ids.isEmpty())
    {
        m_Offset = -1;
        m_OffsetSet = false;
    } else
    {
        std::sort(ids.begin(), ids.end());
        m_Offset = ids.last() + 1;
        m_OffsetSet = true;
    }

    // Next button list ID
    ids = QList < qint64 >(m_ButtonListIDToInfo.keyBegin(),
        m_ButtonListIDToInfo.keyEnd());
    if (ids.isEmpty())
    {
        m_NextButtonListID = 0;
    } else
    {
        std::sort(ids.begin(), ids.end());
        m_NextButtonListID = ids.last() + 1;
    }

    // Next button ID
    ids = QList < qint64 >(m_ButtonIDToInfo.keyBegin(),
        m_ButtonIDToInfo.keyEnd());
    if (ids.isEmpty())
    {
        m_NextButtonID = 0;
    } else
    {
        std::sort(ids.begin(), ids.end());
        m_NextButtonID = ids.last() + 1;
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Read any table with a qint64/bigint ID
bool TelegramComms::ReadDatabase_Table(const QString & mcrTableName,
    QHash < qint64, QHash < QString, QString > > & mrInfoData)
{
    CALL_IN(QString("mcrTableName=%1, mrInfoData=%2")
        .arg(CALL_SHOW(mcrTableName),
             "..."));

    // Internal, but let's check anyway...
    if (!mrInfoData.isEmpty())
    {
        // Should be empty.
        const QString reason =
            tr("mrInfoData when readingtable \"%1\" should be empty.")
                .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Read entire table
    QSqlQuery query;
    query.exec(QString("SELECT id, key, value FROM %1")
        .arg(mcrTableName));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error reading table \"%1\".")
            .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Collect results
    while (query.next())
    {
        const qint64 id = query.value(0).toLongLong();
        const QString key = query.value(1).toString();
        const QString value = query.value(2).toString();
        mrInfoData[id][key] = value;
    }

    // Simple statistics
    qDebug().noquote() << tr("Read table %1: %2 info entries")
        .arg(mcrTableName,
             QString::number(mrInfoData.size()));

    // Done
    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Read any table with a text ID
bool TelegramComms::ReadDatabase_Table(const QString & mcrTableName,
    QHash < QString, QHash < QString, QString > > & mrInfoData)
{
    CALL_IN(QString("mcrTableName=%1, mrInfoData=%2")
        .arg(CALL_SHOW(mcrTableName),
             "..."));

    // Internal, but let's check anyway...
    if (!mrInfoData.isEmpty())
    {
        // Should be empty.
        const QString reason =
            tr("mrInfoData when readingtable \"%1\" should be empty.")
                .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Read entire table
    QSqlQuery query;
    query.exec(QString("SELECT id, key, value FROM %1")
        .arg(mcrTableName));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error reading table \"%1\".")
            .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Collect results
    while (query.next())
    {
        const QString id = query.value(0).toString();
        const QString key = query.value(1).toString();
        const QString value = query.value(2).toString();
        mrInfoData[id][key] = value;
    }

    // Simple statistics
    qDebug().noquote() << tr("Read table %1: %2 info entries")
        .arg(mcrTableName,
             QString::number(mrInfoData.size()));

    // Done
    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Read sticker_set_info table
bool TelegramComms::ReadDatabase_Table_StickerSet(const QString & mcrTableName)
{
    CALL_IN(QString("mcrTableName=%1")
        .arg(CALL_SHOW(mcrTableName)));

    // Read entire table
    QSqlQuery query;
    query.exec(QString("SELECT id, sequence, key, value FROM %1 "
        "ORDER BY id, sequence")
        .arg(mcrTableName));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error reading table \"%1\".")
            .arg(mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Collect results
    while (query.next())
    {
        const QString id = query.value(0).toString();
        const QString key = query.value(2).toString();
        const QString value = query.value(3).toString();
        if (key == "sticker_file_id")
        {
            m_StickerSetNameToFileIDs[id] << value;
        } else
        {
            m_StickerSetNameToInfo[id][key] = value;
        }
    }

    // Simple statistics
    qDebug().noquote() << tr("Read table %1: %2 info enties")
        .arg(mcrTableName,
             QString::number(m_StickerSetNameToInfo.size()));

    // Done
    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Read preferences table
bool TelegramComms::ReadDatabase_Table_Preferences()
{
    CALL_IN("");

    // Read entire table
    QSqlQuery query;
    query.exec(QString("SELECT user_id, key, value FROM preferences;"));
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error reading preferences table.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Collect results
    while (query.next())
    {
        const qint64 user_id = query.value(0).toLongLong();
        const QString key = query.value(1).toString();
        const QString value = query.value(2).toString();
        m_UserIDToPreferences[user_id][key] = value;
    }

    // Done
    CALL_OUT("");
    return true;
}




///////////////////////////////////////////////////////////////////////////////
// Save info data
bool TelegramComms::SaveInfoData(const QString & mcrTableName,
    const QHash < QString, QString > & mcrInfoData, const QString & mcrIDType)
{
    CALL_IN(QString("mcrTableName=%1, mcrInfoData=%2, mcrIDType=%3")
        .arg(CALL_SHOW(mcrTableName),
             CALL_SHOW(mcrInfoData),
             CALL_SHOW(mcrIDType)));

    // Delete existing data
    QSqlQuery query;
    query.prepare(QString("DELETE FROM %1 WHERE id=:id;")
        .arg(mcrTableName));
    if (mcrIDType == "bigint")
    {
        const qint64 id = mcrInfoData["id"].toLongLong();
        query.bindValue(":id", id);
    } else if (mcrIDType == "text")
    {
        query.bindValue(":id", mcrInfoData["id"]);
    }
    query.exec();
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason =
            tr("SQL error deleting ID %1 in table \"%2\".")
                .arg(mcrInfoData["id"],
                     mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Save new data
    query.prepare(QString("INSERT INTO %1 (id, key, value) "
        "VALUES (:id, :key, :value);")
        .arg(mcrTableName));
    if (mcrIDType == "bigint")
    {
        const qint64 id = mcrInfoData["id"].toLongLong();
        query.bindValue(":id", id);
    } else if (mcrIDType == "text")
    {
        query.bindValue(":id", mcrInfoData["id"]);
    }
    for (auto key_iterator = mcrInfoData.keyBegin();
         key_iterator != mcrInfoData.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        const QString value = mcrInfoData[key];
        query.bindValue(":key", key);
        query.bindValue(":value", value);
        query.exec();
        if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
        {
            const QString reason =
                tr("SQL error saving single update ID %1 (key: %2, value: %3) "
                "to table \"%4\".")
                    .arg(mcrInfoData["id"],
                         key,
                         value,
                         mcrTableName);
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return false;
        }
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Save info data
bool TelegramComms::SaveInfoData_StickerSet(const QString & mcrTableName,
    const QString & mcrStickerSetID)
{
    CALL_IN(QString("mcrTableName=%1, mcrStickerSetID=%2")
        .arg(CALL_SHOW(mcrTableName),
             CALL_SHOW(mcrStickerSetID)));

    // Delete entry (if it exists)
    QSqlQuery query;
    query.prepare(QString("DELETE FROM %1 WHERE id=:id;")
        .arg(mcrTableName));
    query.bindValue(":id", mcrStickerSetID);
    query.exec();
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason =
            tr("SQL error deleting ID \"%1\" from table \"%2\".")
                .arg(mcrStickerSetID,
                     mcrTableName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Insert new values
    query.prepare(QString("INSERT INTO %1 (id, key, sequence, value) "
        "VALUES (:id, :key, :sequence, :value);")
        .arg(mcrTableName));
    const QHash < QString, QString > & stickerset_info =
        m_StickerSetNameToInfo[mcrStickerSetID];
    query.bindValue(":id", mcrStickerSetID);
    query.bindValue(":sequence", "0");
    for (auto key_iterator = stickerset_info.keyBegin();
         key_iterator != stickerset_info.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        const QString value = stickerset_info[key];
        query.bindValue(":key", key);
        query.bindValue(":value", value);
        query.exec();
        if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
        {
            const QString reason =
                tr("SQL error saving single update ID %1 (key: %2, value: %3) "
                    "to table \"%4\".")
                    .arg(mcrStickerSetID,
                         key,
                         value,
                         mcrTableName);
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return false;
        }
    }

    // Insert file IDs
    query.bindValue(":key", "sticker_file_id");
    const QStringList sticker_file_ids =
        m_StickerSetNameToFileIDs[mcrStickerSetID];
    for (int index = 0;
         index < sticker_file_ids.size();
         index++)
    {
        query.bindValue(":sequence", QString::number(index + 1));
        query.bindValue(":value", sticker_file_ids[index]);
        query.exec();
        if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
        {
            const QString reason =
                tr("SQL error saving file IDs of sticker set ID %1 "
                    "(sequence: %2, value: %3) to table \"%4\".")
                    .arg(mcrStickerSetID,
                         QString::number(index + 1),
                         sticker_file_ids[index],
                         mcrTableName);
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return false;
        }
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Update database
void TelegramComms::UpdateDatabase()
{
    CALL_IN("");

    // 01 May 2025
    // CreateDatabase_Table("channel_post_info");

    // 14 May 2025
    // CreateDatabase_Preferences();

    CALL_OUT("");
}



// ====================================================================== Setup



///////////////////////////////////////////////////////////////////////////////
// Bot name
bool TelegramComms::SetBotName(const QString & mcrBotName)
{
    CALL_IN(QString("mcrBotName=%1")
        .arg(CALL_SHOW(mcrBotName)));

    // Can't be empty
    if (mcrBotName.isEmpty())
    {
        const QString reason = tr("Telegram bot name cannot be empty.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Can't be set already
    if (!m_BotName.isEmpty())
    {
        const QString reason = tr("Telegram bot has already been set.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    m_BotName = mcrBotName;

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Set bot token
bool TelegramComms::SetToken(const QString & mcrToken)
{
    CALL_IN(QString("mcrToken=%1")
        .arg(CALL_SHOW(mcrToken)));

    // Catch empty token
    if (mcrToken.isEmpty())
    {
        const QString reason = tr("Empty token provided.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Can't be set already
    if (!m_Token.isEmpty())
    {
        const QString reason = tr("Telegram bot has already been set.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Check if token is valid
    static const QRegularExpression format_token(
        "^[0-9]+:[A-Za-z0-9\\-_]+$");
    QRegularExpressionMatch match_token = format_token.match(mcrToken);
    if (!match_token.hasMatch())
    {
        const QString reason = tr("Token does not have a valid format: \"%1\"")
            .arg(mcrToken);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Set new token
    m_Token = mcrToken;

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Set offset
void TelegramComms::SetOffset(const qint64 mcOffset)
{
    CALL_IN(QString("mcOffset=%1")
        .arg(CALL_SHOW(mcOffset)));

    m_Offset = mcOffset;
    m_OffsetSet = true;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Start the bot (start listening)
void TelegramComms::StartBot()
{
    CALL_IN("");

    // Check if bot is already running
    if (m_IsRunning)
    {
        const QString reason = tr("Bot is already running.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    // Check if bot name is set
    if (m_BotName.isEmpty())
    {
        const QString reason = tr("Bot name has not been set.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    // Check if bot token is set
    if (m_Token.isEmpty())
    {
        const QString reason = tr("Token has not been set.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    // Check if database is open
    if (!m_DatabaseConnected)
    {
        const QString reason = tr("Database is not connected.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    // Bot is running
    m_IsRunning = true;

    // Remember start date/time
    m_StartDateTime = QDateTime::currentDateTime();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Stop the bot (stop listening)
void TelegramComms::StopBot()
{
    CALL_IN("");

    // Check if bot is already running
    if (!m_IsRunning)
    {
        const QString reason = tr("Bot is not running.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    // Bot is not running
    m_IsRunning = false;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Uptime
QString TelegramComms::GetUptime() const
{
    CALL_IN("");

    const qint64 secs = m_StartDateTime.secsTo(QDateTime::currentDateTime());
    QString uptime =
        QString::number(secs/3600) + ":" +
        ("0" + QString::number((secs/60)%60)).right(2) + ":" +
        ("0" + QString::number(secs%60)).right(2);

    CALL_OUT("");
    return uptime;
}



// ================================================================ Preferences



///////////////////////////////////////////////////////////////////////////////
// Get preferences
QHash < QString, QString > TelegramComms::GetPreferences(
    const qint64 mcUserID) const
{
    CALL_IN(QString("mcUserID=%1")
        .arg(CALL_SHOW(mcUserID)));

    if (!m_UserIDToPreferences.contains(mcUserID))
    {
        // Defaults
        CALL_OUT("");
        return m_DefaultPreferences;
    }

    QHash < QString, QString > prefs = m_DefaultPreferences;
    for (auto key_iterator = m_UserIDToPreferences[mcUserID].keyBegin();
        key_iterator != m_UserIDToPreferences[mcUserID].keyEnd();
        key_iterator++)
    {
        const QString & key = *key_iterator;
        prefs[key] = m_UserIDToPreferences[mcUserID][key];
    }

    CALL_OUT("");
    return prefs;
}



///////////////////////////////////////////////////////////////////////////////
// Get preferences
QString TelegramComms::GetPreferenceValue(const qint64 mcUserID,
    const QString & mcrKey) const
{
    CALL_IN(QString("mcUserID=%1, mcrKey=%2")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcrKey)));

    // Check if tag exists
    if (!m_DefaultPreferences.contains(mcrKey))
    {
        const QString reason = tr("Unknown preferences tag \"%1\".")
            .arg(mcrKey);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QString();
    }

    if (m_UserIDToPreferences.contains(mcUserID) &&
        m_UserIDToPreferences[mcUserID].contains(mcrKey))
    {
        CALL_OUT("");
        return m_UserIDToPreferences[mcUserID][mcrKey];
    } else
    {
        CALL_OUT("");
        return m_DefaultPreferences[mcrKey];
    }

    // We never get here
}



///////////////////////////////////////////////////////////////////////////////
// Set preference value
void TelegramComms::SetPreferenceValue(const qint64 mcUserID,
    const QString & mcrKey, const QString & mcrNewValue)
{
    CALL_IN(QString("mcUserID=%1, mcrKey=%2, mcrNewValue=%3")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcrKey),
             CALL_SHOW(mcrNewValue)));

    // Check if tag exists
    if (!m_DefaultPreferences.contains(mcrKey))
    {
        const QString reason = tr("Unknown preferences tag \"%1\".")
            .arg(mcrKey);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    // Set value
    m_UserIDToPreferences[mcUserID][mcrKey] = mcrNewValue;

    // Set value in database
    QSqlQuery query;
    query.prepare(QString("DELETE FROM preferences "
        "WHERE user_id=:user_id "
        "AND key=:key;"));
    query.bindValue(":user_id", mcUserID);
    query.bindValue(":key", mcrKey);
    query.exec();
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error removing old preferences value "
            "\"%1\" for user %2.")
            .arg(mcrKey,
                 QString::number(mcUserID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    query.prepare(QString("INSERT INTO preferences "
        "(user_id, key, value) VALUES "
        "(:user_id, :key, :value);"));
    query.bindValue(":user_id", mcUserID);
    query.bindValue(":key", mcrKey);
    query.bindValue(":value", mcrNewValue);
    query.exec();
    if (DatabaseHelper::HasSQLError(query, __FILE__, __LINE__))
    {
        const QString reason = tr("SQL error adding new preferences value "
            "\"%1\" for user %2 (new value \"%3\").")
            .arg(mcrKey,
                 QString::number(mcUserID),
                 mcrNewValue);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    CALL_OUT("");
}



// ==================================================== Reading from the Server



///////////////////////////////////////////////////////////////////////////////
// For periodic checks
void TelegramComms::Periodic_WatchForUpdates()
{
    CALL_IN("");

    if (m_IsRunning)
    {
        CheckForUpdates();
    }

    // Try again in a bit
    QTimer::singleShot(POLL_DELAY,
        this, &TelegramComms::Periodic_WatchForUpdates);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Check for updates
void TelegramComms::CheckForUpdates()
{
    CALL_IN("");

    // Check if bot is running
    if (!m_IsRunning)
    {
        const QString reason = tr("Bot is not running.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT("");
        return;
    }

    QString url = QString("https://api.telegram.org/bot%1/getUpdates")
        .arg(m_Token);
    if (m_OffsetSet)
    {
        url += "?offset=" + QString::number(m_Offset);
    }
    QNetworkRequest request;
    request.setUrl(url);
    m_NetworkAccessManager -> get(request);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Handle response
bool TelegramComms::HandleResponse(QNetworkReply * mpResponse)
{
    CALL_IN(QString("mpResponse=%1")
        .arg(CALL_SHOW(mpResponse)));


    // Keep error
    const int nework_error = mpResponse -> error();

    // Read content of the response
    const QByteArray content = mpResponse -> readAll();
    if (content.isEmpty())
    {
        // No response
        QString reason;

        // Check if we have an error
        if (nework_error != QNetworkReply::NoError)
        {
            // No response
            reason = tr("An error has occurred processing the network request "
                "(%1). No response content received.")
                    .arg(QString::number(nework_error));
        }  else
        {
            reason = tr("No response content received");
        }

        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // == Check for binary file
    const QString url = mpResponse -> url().toString();
    static const QRegularExpression format_binary(
        "^https://api.telegram.org/file/bot[^/]+/(.*)$");
    const QRegularExpressionMatch match_binary = format_binary.match(url);
    if (match_binary.hasMatch())
    {
        const QString file_path = match_binary.captured(1);
        const bool success = SaveFile(file_path, content);
        CALL_OUT("");
        return success;
    }


    // == Otherwise, result will be in JSON

    // Parse JSON
    QJsonDocument doc_response = QJsonDocument::fromJson(content);
    if (doc_response.isNull())
    {
        // Not JSON format
        const QString reason = tr("No JSON response received");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }
    if (DEBUG)
    {
        qDebug().noquote() << doc_response.toJson(QJsonDocument::Indented);
    }

    // Parse response
    QJsonObject response = doc_response.object();
    bool success = Parse_Response(response);

    CALL_OUT("");
    return success;
}



///////////////////////////////////////////////////////////////////////////////
// Original server response
bool TelegramComms::Parse_Response(const QJsonObject & mcrResponse)
{
    CALL_IN(QString("mcrResponse=%1")
        .arg(CALL_SHOW(mcrResponse)));

    // {
    //   "ok":true,
    //   "result":[...]
    //   "description": "...",
    //   "error_code": 409
    // }

    // Parse object
    QHash < QString, QString > response_info;

    for (auto key_iterator = mcrResponse.constBegin();
         key_iterator != mcrResponse.constEnd();
         key_iterator++)
    {
        const QString key = key_iterator.key();
        if (key == "description")
        {
            response_info["description"] = mcrResponse[key].toString();
            continue;
        }
        if (key == "error_code")
        {
            const qint64 error_code = mcrResponse[key].toInteger();
            response_info["error_code"] = QString::number(error_code);
            continue;
        }
        if (key == "ok")
        {
            response_info["ok"] = mcrResponse[key].toBool() ? "true" : "false";
            continue;
        }
        if (key == "result")
        {
            // Ignore; we'll deal with this later.
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in response [ignored]")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }


    // == Check if response was "ok"
    if (!response_info.contains("ok"))
    {
        const QString reason = tr("Response does not have an \"ok\" status.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }
    if (response_info["ok"] != "true")
    {
        // We received a response, but something didn't work
        const QString description = (response_info.contains("description") ?
            response_info["description"] : "");
        const QString error_code = (response_info.contains("error_code") ?
            response_info["error_code"] : "");

        if (error_code == "400" &&
            description == "Bad Request: STICKERSET_INVALID")
        {
            // Unknown sticker set
            Error_StickerSetInvalid();
        } else
        {
            // Unhandled problem
            qDebug().noquote() << tr("Receiving an update was unsuccessful: "
                "%1 (error %2)")
                .arg(description,
                     error_code);
        }

        CALL_OUT("");
        return false;
    }


    // == Result

    // Check if we have a "result"
    if (!mcrResponse.contains("result"))
    {
        const QString reason =
            tr("Response does not have results (\"result\")");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Check for an array or responses
    if (mcrResponse["result"].isArray())
    {
        // Parse list of updates
        const QJsonArray all_updates = mcrResponse["result"].toArray();
        const bool success = Parse_UpdateArray(all_updates);
        CALL_OUT("");
        return success;
    }

    const QJsonObject result_obj = mcrResponse["result"].toObject();

    // Check if result is a single message
    if (result_obj.contains("chat") &&
        result_obj.contains("date") &&
        result_obj.contains("from") &&
        result_obj.contains("message_id"))
    {
        const QHash < QString, QString > message_info =
            Parse_Message(result_obj);
        const bool success = !message_info.isEmpty();
        CALL_OUT("");
        return success;
    }

    // Check if result is a file info
    if (result_obj.contains("file_path") &&
        result_obj.contains("file_unique_id") &&
        result_obj.contains("file_size"))
    {
        QHash < QString, QString > file_info = Parse_File(result_obj);
        bool success = true;

        // Download file if we have one
        if (file_info.contains("file_path"))
        {
            const QString id = file_info["id"];
            const QString file_path = file_info["file_path"];
            success = Download_FilePath(id, file_path);
            file_info.remove("file_path");
            SaveInfoData("file_info", file_info, "text");
        }
        CALL_OUT("");
        return success;
    }

    // Check if result is a sticker set info
    if (result_obj.contains("name") &&
        result_obj.contains("title") &&
        result_obj.contains("sticker_type") &&
        result_obj.contains("stickers"))
    {
        const QHash < QString, QString > stickerset_info =
            Parse_StickerSet(result_obj);
        const bool success = !stickerset_info.isEmpty();
        CALL_OUT("");
        return success;
    }

    // Check if we have an empty result (if response was just "ok" without
    // additional feedback)
    if (result_obj.isEmpty())
    {
        CALL_OUT("");
        return true;
    }

    // == It's none of the above
    const QString json = QJsonDocument(result_obj).toJson();
    const QString message = tr("Unknown JSON object in response: %1")
        .arg(json);
    MessageLogger::Error(CALL_METHOD, message);

    CALL_OUT("");
    return false;
}



///////////////////////////////////////////////////////////////////////////////
// Parse an array of updates
bool TelegramComms::Parse_UpdateArray(const QJsonArray & mcrUpdates)
{
    CALL_IN(QString("mcrUpdates=%1")
        .arg(CALL_SHOW(mcrUpdates)));

    // Loop elements of the array
    for (auto update_iterator = mcrUpdates.begin();
         update_iterator != mcrUpdates.end();
         update_iterator++)
    {
        const QJsonObject & update = update_iterator -> toObject();
        const QHash < QString, QString > update_info = Parse_Update(update);
        if (update_info.isEmpty())
        {
            const QString reason = tr("Update could not be parsed.");
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return false;
        }

        // Update last update ID
        const qint64 update_id = update_info["id"].toLongLong();
        m_Offset = update_id + 1;
        m_OffsetSet = true;

        // Send signal for update
        const qint64 chat_id = update_info["chat_id"].toLongLong();
        emit UpdateReceived(chat_id, update_id);
    }

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Parse update
QHash < QString, QString > TelegramComms::Parse_Update(
    const QJsonObject & mcrUpdate)
{
    CALL_IN(QString("mcrUpdate=%1")
        .arg(CALL_SHOW_FULL(mcrUpdate)));

    // {
    //   "update_id":494953278,
    //   "message":{...}
    // }

    // {
    //   "update_id":494953279,
    //   "my_chat_member":{...}
    // }

    // {
    //   "update_id":494953279,
    //   "edited_message":{...}
    // }

    // Check if we have parsed this update previously
    const qint64 update_id = mcrUpdate["update_id"].toInteger();
    if (m_UpdateIDToInfo.contains(update_id))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_UpdateIDToInfo[update_id];
    }

    // Parse the new update
    QHash < QString, QString > update_info;

    const QStringList all_keys = mcrUpdate.keys();
    for (const QString & key : all_keys)
    {
        if (key == "channel_post")
        {
            const QHash < QString, QString > channel_post_info =
                Parse_ChannelPost(mcrUpdate[key].toObject());
            if (channel_post_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing update (channel post)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            update_info["type"] = "channel post";
            update_info["message_id"] = channel_post_info["id"];
            update_info["chat_id"] = channel_post_info["chat_id"];
            continue;
        }

        if (key == "edited_channel_post")
        {
            const QHash < QString, QString > channel_post_info =
                Parse_ChannelPost(mcrUpdate[key].toObject());
            if (channel_post_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing update (channel post)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            update_info["type"] = "channel post";
            update_info["message_id"] = channel_post_info["id"];
            update_info["chat_id"] = channel_post_info["chat_id"];
            continue;
        }

        if (key == "edited_message")
        {
            const QHash < QString, QString > message_info =
                Parse_Message(mcrUpdate[key].toObject());
            if (message_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing update (edited message)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            update_info["type"] = "message";
            update_info["message_id"] = message_info["id"];
            update_info["chat_id"] = message_info["chat_id"];
            continue;
        }

        if (key == "message")
        {
            const QHash < QString, QString > message_info =
                Parse_Message(mcrUpdate[key].toObject());
            if (message_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing update (message)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            update_info["type"] = "message";
            update_info["message_id"] = message_info["id"];
            update_info["chat_id"] = message_info["chat_id"];
            continue;
        }

        if (key == "my_chat_member")
        {
            const QHash < QString, QString > mychatmember_info =
                Parse_MyChatMember(mcrUpdate[key].toObject());
            if (mychatmember_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing update (my_chat_member)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            update_info["type"] = "my_chat_member";
            update_info["my_chat_member_id"] = mychatmember_info["id"];
            update_info["chat_id"] = mychatmember_info["chat_id"];
            continue;
        }

        if (key == "update_id")
        {
            update_info["id"] =
                QString::number(mcrUpdate[key].toInteger());
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in update")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save update info
    if (!update_info.contains("id"))
    {
        const QString reason = tr("Update is missing an ID");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    m_UpdateIDToInfo[update_id] = update_info;
    SaveInfoData("update_info", update_info);

    CALL_OUT("");
    return update_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if update info exists
bool TelegramComms::DoesUpdateInfoExist(const qint64 mcUpdateID) const
{
    CALL_IN(QString("mcUpdateID=%1")
        .arg(CALL_SHOW(mcUpdateID)));

    const bool exists = m_UpdateIDToInfo.contains(mcUpdateID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Update
QHash < QString, QString > TelegramComms::GetUpdateInfo(
    const qint64 mcUpdateID)
{
    CALL_IN(QString("mcUpdateID=%1")
        .arg(CALL_SHOW(mcUpdateID)));

    // Check if we have this update
    if (!m_UpdateIDToInfo.contains(mcUpdateID))
    {
        const QString reason = tr("Update ID %1 does not exist")
            .arg(QString::number(mcUpdateID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_UpdateIDToInfo[mcUpdateID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Message
QHash < QString, QString > TelegramComms::Parse_Message(
    const QJsonObject & mcrMessage)
{
    CALL_IN(QString("mcrMessage=%1")
        .arg(CALL_SHOW_FULL(mcrMessage)));

    // General message
    // {
    //   "message_id":5,
    //   "from":{...}
    //   "chat":{...}
    //   "date":1737148752,
    //   "message_thread_id":4006,
    //   "reply_to_message":{...}
    //   "reply_markup":{...}
    //   "text":"Hey there!"
    // OR
    //   "sticker":{...}
    // OR
    //   "photo":[...]
    //   "caption":"[caption]",
    // OR
    //   "document":{...}
    // OR
    //   "forward_date":1744913109,
    //   "forward_from":{...},
    //   "forward_from_chat":{...},
    //   "forward_from_message_id":10010,
    //   "forward_origin":{...},
    //   "forward_sender_name":"..."
    //   "forward_signature":"...",
    //
    //   "link_preview_options":
    //   {
    //     "url":"https://t.me/addstickers/RakanHowls"
    //   }
    // }

    // Bot command
    // {
    //   "message_id":1,
    //   "from":{...},
    //   "chat":{...},
    //   "date":1737146691,
    //   "text":"/start",
    //   "entities":[...]
    // }

    // User joined
    // {
    //   "message_id":4,
    //   "from":{...},
    //   "chat":{...},
    //   "date":1737146713,
    //   "new_chat_participant":{...},
    //   "new_chat_member":{...},
    //   "new_chat_members":[...],
    //   "sender_chat": {...}
    // }

    // Channel renamed
    // {
    //   "message_id":60092,
    //   "from":{...},
    //   "chat":{...},
    //   "date":1746692561,
    //   "new_chat_title":"Shima's MakeFox Bot"
    // }

    // Channel received new picture
    // {
    //   "chat":{...},
    //   "date":1748260123,
    //   "from":{...},
    //   "message_id":61565,
    //   "new_chat_photo":[...],
    //  }



    // Check if we have parsed this message previously
    const qint64 message_id = mcrMessage["message_id"].toInteger();
    if (m_MessageIDToInfo.contains(message_id))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_MessageIDToInfo[message_id];
    }

    // Parse the new message

    // Loop contents
    QHash < QString, QString > message_info;
    const QStringList all_keys = mcrMessage.keys();
    for (const QString & key : all_keys)
    {
        if (key == "animation")
        {
            const QHash < QString, QString > animation_info =
                Parse_File(mcrMessage[key].toObject());
            message_info["animation_file_id"] = animation_info["id"];
            continue;
        }

        if (key == "caption")
        {
            message_info["caption"] = mcrMessage[key].toString();
            continue;
        }

        if (key == "chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrMessage[key].toObject());
            if (chat_info.isEmpty())
            {
                const QString reason = tr("Error parsing chat info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["chat_id"] = chat_info["id"];

            // Add chat to active ones
            m_ActiveChats += chat_info["id"].toLongLong();

            continue;
        }

        if (key == "date")
        {
            const QDateTime date =
                QDateTime::fromSecsSinceEpoch(mcrMessage[key].toInt());
            message_info["date_time"] =
                date.toString("yyyy-MM-dd hh:mm:ss");
            continue;
        }

        if (key == "document")
        {
            const QHash < QString, QString > document_info =
                Parse_File(mcrMessage[key].toObject());
            if (document_info.isEmpty())
            {
                const QString reason = tr("Error parsing document info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["document_id"] = document_info["id"];
            continue;
        }

        if (key == "edit_date")
        {
            const QDateTime date =
                QDateTime::fromSecsSinceEpoch(mcrMessage[key].toInt());
            message_info["edit_date_time"] =
                date.toString("yyyy-MM-dd hh:mm:ss");
            continue;
        }

        if (key == "entities")
        {
            // [
            //   {
            //     "length": 20,
            //     "offset": 0,
            //     "type": "bot_command"
            //   }
            // ]
            // !!! Ignore for now
            continue;
        }

        if (key == "forward_date")
        {
            const QDateTime date =
                QDateTime::fromSecsSinceEpoch(mcrMessage[key].toInt());
            message_info["forward_date_time"] =
                date.toString("yyyy-MM-dd hh:mm:ss");
            continue;
        }

        if (key == "forward_from")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrMessage[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing user info (forward_from)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["forward_from_id"] = user_info["id"];
            continue;
        }

        if (key == "forward_from_chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrMessage[key].toObject());
            if (chat_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing chat info (forward_from_chat)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["forward_from_chat_id"] = chat_info["id"];
            continue;
        }

        if (key == "forward_from_message_id")
        {
            message_info["forward_from_message_id"] =
                QString::number(mcrMessage[key].toInteger());
            continue;
        }

        if (key == "forward_origin")
        {
            // Combines the following:
            // - forward_signature
            // - forward_from_chat
            // - forward_date
            // - forward_from_message
            // - forward_sender_name
            // "forward_origin":
            // {
            //   "author_signature":"...",
            //   "chat":
            //   {
            //     "id":-1001732480834,
            //     "title":"Feral Temptation",
            //     "type":"channel",
            //     "username":"feraltemptation"
            //   },
            //   "date":1745021693,
            //   "message_id":10010,
            //   "sender_user":{...}
            //   "type":"channel"
            // },

            // Ignore
            continue;
        }

        if (key == "forward_sender_name")
        {
            message_info["forward_sender_name"] = mcrMessage[key].toString();
            continue;
        }

        if (key == "forward_signature")
        {
            message_info["forward_singature"] = mcrMessage[key].toString();
            continue;
        }

        if (key == "from")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrMessage[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason = tr("Error parsing user info (from)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["from_id"] = user_info["id"];
            continue;
        }

        if (key == "link_preview_options")
        {
            // "link_preview_options":
            // {
            //   "url":"https://t.me/addstickers/RakanHowls"
            // }
            // !!! Ignore for now
            qDebug().noquote() << key
                << QJsonDocument(mcrMessage[key].toArray()).toJson();
            continue;
        }

        if (key == "message_id")
        {
            const qint64 message_id = mcrMessage[key].toInteger();
            message_info["id"] = QString::number(message_id);
            continue;
        }

        if (key == "message_thread_id")
        {
            const qint64 message_id = mcrMessage[key].toInteger();
            message_info["message_thread_id"] = QString::number(message_id);
            continue;
        }

        if (key == "new_chat_member")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrMessage[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing user info (new_chat_member)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["new_chat_member_id"] = user_info["id"];
            continue;
        }

        if (key == "new_chat_photo")
        {
            QJsonArray photos = mcrMessage[key].toArray();
            QJsonObject last_photo = photos.last().toObject();
            if (last_photo.isEmpty())
            {
                // Not successful
                const QString reason =
                    tr("New channel photo array did not have a last entry.");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            const QHash < QString, QString > photo_info =
                Parse_File(last_photo);
            if (photo_info.isEmpty())
            {
                const QString reason = tr("Error parsing in picture list "
                    "(new channel photo)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["new_chat_photo_id"] = photo_info["id"];
            continue;
        }

        if (key == "new_chat_members")
        {
            // Ignored because redundant with new_chat_member
            continue;
        }

        if (key == "new_chat_participant")
        {
            // Ignored because redundant with new_chat_member
            continue;
        }

        if (key == "new_chat_title")
        {
            message_info[key] = mcrMessage[key].toString();
            continue;
        }

        if (key == "photo")
        {
            // Contains an array of the picture in various sizes
            // [
            //   {
            //     "file_id":"[file_id]",
            //     "file_size":1505,
            //     "file_unique_id":"[file_unique_id]",
            //     "height":81,
            //     "width":90
            //   },{
            //     "file_id":"[file_id]",
            //     "file_size":25047,
            //     "file_unique_id":"[file_unqiue_id]",
            //     "height":289,
            //     "width":320
            //   },{
            //     "file_id":"[file_id]",
            //     "file_size":102120,
            //     "file_unique_id":"[file_unique_id]",
            //     "height":722,
            //     "width":800
            //   },{
            //     "file_id":"[file_id]",
            //     "file_size":183122,
            //     "file_unique_id":"[file_unique_id]",
            //     "height":1156,
            //     "width":1280
            //   }
            // ]
            QJsonArray photos = mcrMessage[key].toArray();
            QJsonObject last_photo = photos.last().toObject();
            if (last_photo.isEmpty())
            {
                // Not successful
                const QString reason =
                    tr("photo array did not have a last entry.");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            const QHash < QString, QString > photo_info =
                Parse_File(last_photo);
            if (photo_info.isEmpty())
            {
                const QString reason = tr("Error parsing in photos list "
                    "(photo)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["photo_file_id"] = photo_info["id"];
            continue;
        }

        if (key == "reply_markup")
        {
            const QHash < QString, QString > button_list =
                Parse_ButtonList(mcrMessage[key].toObject());
            message_info["button_list_id"] = button_list["id"];
            continue;
        }

        if (key == "reply_to_message")
        {
            const QHash < QString, QString > reply_to_message_info =
                Parse_Message(mcrMessage[key].toObject());
            if (reply_to_message_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing message info (reply_to)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["reply_to_message_id"] = reply_to_message_info["id"];
            continue;
        }

        if (key == "sender_chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrMessage[key].toObject());
            if (chat_info.isEmpty())
            {
                const QString reason = tr("Error parsing sender_chat info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["sender_chat_id"] = chat_info["id"];
            continue;
        }

        if (key == "sticker")
        {
            const QHash < QString, QString > sticker_info =
                Parse_File(mcrMessage[key].toObject());
            if (sticker_info.isEmpty())
            {
                const QString reason = tr("Error parsing sticker info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            message_info["sticker_id"] = sticker_info["id"];
            continue;
        }

        if (key == "text")
        {
            message_info["text"] = mcrMessage[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in message")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save message info
    if (!message_info.contains("id"))
    {
        const QString reason = tr("Message is missing an ID");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    m_MessageIDToInfo[message_id] = message_info;
    SaveInfoData("message_info", message_info);

    // Let everybody know
    const qint64 chat_id = message_info["chat_id"].toLongLong();
    emit MessageReceived(chat_id, message_id);

    CALL_OUT("");
    return message_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if message info exists
bool TelegramComms::DoesMessageInfoExist(const qint64 mcMessageID) const
{
    CALL_IN(QString("mcMessageID=%1")
        .arg(CALL_SHOW(mcMessageID)));

    const bool exists = m_MessageIDToInfo.contains(mcMessageID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Message
QHash < QString, QString > TelegramComms::GetMessageInfo(
    const qint64 mcMessageID)
{
    CALL_IN(QString("mcMessageID=%1")
        .arg(CALL_SHOW(mcMessageID)));

    // Check if we have this message
    if (!m_MessageIDToInfo.contains(mcMessageID))
    {
        const QString reason = tr("Message ID %1 does not exist")
            .arg(QString::number(mcMessageID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_MessageIDToInfo[mcMessageID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: User
QHash < QString, QString > TelegramComms::Parse_User(
    const QJsonObject & mcrUser)
{
    CALL_IN(QString("mcrUser=%1")
        .arg(CALL_SHOW_FULL(mcrUser)));

    // {
    //   "id":725804777,
    //   "is_bot":false,
    //   "is_premium":true,
    //   "first_name":"Shimaron",
    //   "last_name":"Greywolf",
    //   "username":"shimarongreywolf",
    //   "language_code":"en"
    // }

    // Check if we have parsed this user previously
    const qint64 user_id = mcrUser["id"].toInteger();
    if (m_UserIDToInfo.contains(user_id))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_UserIDToInfo[user_id];
    }

    // Parse the new user

    // Loop contents
    QHash < QString, QString > user_info;
    const QStringList all_keys = mcrUser.keys();
    for (const QString & key : all_keys)
    {
        if (key == "first_name")
        {
            user_info["first_name"] = mcrUser[key].toString();
            continue;
        }
        if (key == "id")
        {
            user_info["id"] = QString::number(mcrUser[key].toInteger());
            continue;
        }
        if (key == "is_bot")
        {
            user_info["is_bot"] =
                mcrUser[key].toBool() ? "true" : "false";
            continue;
        }
        if (key == "is_premium")
        {
            user_info["is_premium"] =
                mcrUser[key].toBool() ? "true" : "false";
            continue;
        }
        if (key == "language_code")
        {
            user_info["language_code"] = mcrUser[key].toString();
            continue;
        }
        if (key == "last_name")
        {
            user_info["last_name"] = mcrUser[key].toString();
            continue;
        }
        if (key == "username")
        {
            user_info["username"] = mcrUser[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in user")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save user info
    if (!user_info.contains("id"))
    {
        const QString reason = tr("User is missing an ID");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    m_UserIDToInfo[user_id] = user_info;
    SaveInfoData("user_info", user_info);

    CALL_OUT("");
    return user_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if user info exists
bool TelegramComms::DoesUserInfoExist(const qint64 mcUserID) const
{
    CALL_IN(QString("mcUserID=%1")
        .arg(CALL_SHOW(mcUserID)));

    const bool exists = m_UserIDToInfo.contains(mcUserID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// User
QHash < QString, QString > TelegramComms::GetUserInfo(const qint64 mcUserID)
{
    CALL_IN(QString("mcUserID=%1")
        .arg(CALL_SHOW(mcUserID)));

    // Check if we have this user
    if (!m_UserIDToInfo.contains(mcUserID))
    {
        const QString reason = tr("User ID %1 does not exist")
            .arg(QString::number(mcUserID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_UserIDToInfo[mcUserID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Chat
QHash < QString, QString > TelegramComms::Parse_Chat(
    const QJsonObject & mcrChat)
{
    CALL_IN(QString("mcrChat=%1")
        .arg(CALL_SHOW_FULL(mcrChat)));

    // Group
    // {
    //   "id":-1002375397830,
    //   "title":"ShimaTest",
    //   "type":"supergroup"
    //   "all_members_are_administrators":true
    // }

    // Private chat
    // {
    //   "id":725804777,
    //   "is_bot":true,
    //   "first_name":"Shimaron",
    //   "last_name":"Greywolf",
    //   "username":"shimarongreywolf",
    //   "type":"private"
    // }

    // Check if we have parsed this chat previously
    const qint64 chat_id = mcrChat["id"].toInteger();
    if (m_ChatIDToInfo.contains(chat_id))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_ChatIDToInfo[chat_id];
    }

    // Parse the new chat

    // Loop contents
    QHash < QString, QString > chat_info;
    const QStringList all_keys = mcrChat.keys();
    for (const QString & key : all_keys)
    {
        if (key == "all_members_are_administrators")
        {
            chat_info["all_members_are_administrators"] =
                mcrChat[key].toBool() ? "true" : "false";
            continue;
        }

        if (key == "first_name")
        {
            chat_info["first_name"] = mcrChat[key].toString();
            continue;
        }
        if (key == "id")
        {
            chat_info["id"] =
                QString::number(mcrChat[key].toInteger());
            continue;
        }
        if (key == "title")
        {
            chat_info["title"] = mcrChat[key].toString();
            continue;
        }
        if (key == "type")
        {
            chat_info["type"] = mcrChat[key].toString();
            continue;
        }
        if (key == "is_bot")
        {
            chat_info["first_name"] =
                mcrChat[key].toBool() ? "true" : "false";
            continue;
        }
        if (key == "last_name")
        {
            chat_info["last_name"] = mcrChat[key].toString();
            continue;
        }
        if (key == "username")
        {
            chat_info["username"] = mcrChat[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in chat")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save chat info
    if (!chat_info.contains("id"))
    {
        const QString reason = tr("Chat is missing an id");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    m_ChatIDToInfo[chat_id] = chat_info;
    SaveInfoData("chat_info", chat_info);

    CALL_OUT("");
    return chat_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if chat info exists
bool TelegramComms::DoesChatInfoExist(const qint64 mcChatID) const
{
    CALL_IN(QString("mcChatID=%1")
        .arg(CALL_SHOW(mcChatID)));

    const bool exists = m_ChatIDToInfo.contains(mcChatID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Chat
QHash < QString, QString > TelegramComms::GetChatInfo(const qint64 mcChatID)
{
    CALL_IN(QString("mcCharID=%1")
        .arg(CALL_SHOW(mcChatID)));

    // Check if we have this chat
    if (!m_ChatIDToInfo.contains(mcChatID))
    {
        const QString reason = tr("Chat ID %1 does not exist")
            .arg(QString::number(mcChatID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_ChatIDToInfo[mcChatID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Chat members
QHash < QString, QString > TelegramComms::Parse_MyChatMember(
    const QJsonObject & mcrMyChatMember)
{
    CALL_IN(QString("mcrMyChatMember=%1")
        .arg(CALL_SHOW_FULL(mcrMyChatMember)));

    // {
    //   "chat":{...},
    //   "from":{...},
    //   "date":1737146726,
    //   "old_chat_member":{...},
    //   "new_chat_member":{...}
    // }

    // Loop contents
    QHash < QString, QString > mychatmember_info;
    const QStringList all_keys = mcrMyChatMember.keys();
    for (const QString & key : all_keys)
    {
        if (key == "chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrMyChatMember[key].toObject());
            if (chat_info.isEmpty())
            {
                const QString reason = tr("Error parsing chat info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            mychatmember_info["chat_id"] = chat_info["id"];
            continue;
        }

        if (key == "date")
        {
            const QDateTime date =
                QDateTime::fromSecsSinceEpoch(mcrMyChatMember[key].toInt());
            mychatmember_info["date_time"] =
                date.toString("yyyy-MM-dd hh:mm:ss");

            // Also use the timestamp as ID
            mychatmember_info["id"] =
                QString::number(mcrMyChatMember[key].toInteger());
            continue;
        }

        if (key == "from")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrMyChatMember[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing in my_chat_member info (from)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            mychatmember_info["from_id"] = user_info["id"];
            continue;
        }

        if (key == "old_chat_member")
        {
            const QJsonObject old_chat_member =
                mcrMyChatMember["old_chat_member"].toObject();
            const QHash < QString, QString > old_chat_member_info =
                Parse_MyChatMember_OldChatMember(old_chat_member);
            if (old_chat_member_info.isEmpty())
            {
                const QString reason = tr("Error parsing in my_chat_member "
                    "info (old_chat_member)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            for (auto subkey_iterator = old_chat_member_info.keyBegin();
                 subkey_iterator != old_chat_member_info.keyEnd();
                 subkey_iterator++)
            {
                const QString & subkey = *subkey_iterator;
                mychatmember_info["old_chat_member_" + subkey] =
                    old_chat_member_info[subkey];
            }
            continue;
        }

        if (key == "new_chat_member")
        {
            const QJsonObject new_chat_member =
                mcrMyChatMember["new_chat_member"].toObject();
            const QHash < QString, QString > new_chat_member_info =
                Parse_MyChatMember_NewChatMember(new_chat_member);
            if (new_chat_member_info.isEmpty())
            {
                const QString reason = tr("Error parsing in my_chat_member "
                    "info (new_chat_member)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            for (auto subkey_iterator = new_chat_member_info.keyBegin();
                 subkey_iterator != new_chat_member_info.keyEnd();
                 subkey_iterator++)
            {
                const QString & subkey = *subkey_iterator;
                mychatmember_info["new_chat_member_" + subkey] =
                    new_chat_member_info[subkey];
            }
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in my_chat_member")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save mychatmember info
    if (!mychatmember_info.contains("id"))
    {
        const QString reason = tr("MyChatMember is missing an ID");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    const qint64 mychatmember_id = mychatmember_info["id"].toLongLong();
    m_MyChatMemberIDToInfo[mychatmember_id] = mychatmember_info;
    SaveInfoData("my_chat_member_info", mychatmember_info);

    CALL_OUT("");
    return mychatmember_info;
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Chat members, old chat member
QHash < QString, QString > TelegramComms::Parse_MyChatMember_OldChatMember(
    const QJsonObject & mcrOldChatMember)
{
    CALL_IN(QString("mcrOldChatMember=%1")
        .arg(CALL_SHOW_FULL(mcrOldChatMember)));

    // {
    //   "user": {...}
    //   "status":"member"
    // }

    // Loop contents
    QHash < QString, QString > oldchatmember_info;
    const QStringList all_keys = mcrOldChatMember.keys();
    for (const QString & key : all_keys)
    {
        if (key == "user")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrOldChatMember[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason = tr("Error parsing user info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            oldchatmember_info["user_id"] = user_info["id"];
            continue;
        }

        if (key == "status")
        {
            oldchatmember_info["status"] = mcrOldChatMember[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in old_chat_member")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    CALL_OUT("");
    return oldchatmember_info;
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Chat members, new chat member
QHash < QString, QString > TelegramComms::Parse_MyChatMember_NewChatMember(
    const QJsonObject & mcrNewChatMember)
{
    CALL_IN(QString("mcrNewChatMember=%1")
        .arg(CALL_SHOW_FULL(mcrNewChatMember)));

    // {
    //   "user":{...}
    //   "status":"administrator",
    //   "can_be_edited":false,
    //   "can_change_info":true,
    //   "can_delete_messages":true,
    //   "can_delete_stories":true,
    //   "can_edit_messages":true,
    //   "can_edit_stories":true,
    //   "can_invite_users":true,
    //   "can_manage_chat":true,
    //   "can_manage_topics":false,
    //   "can_manage_video_chats":true,
    //   "can_manage_voice_chats":true,
    //   "can_pin_messages":true,
    //   "can_post_messages": true,
    //   "can_post_stories":true,
    //   "can_promote_members":false,
    //   "can_restrict_members":true,
    //   "is_anonymous":false
    //   "until_date":0,
    // }

    // Flag values
    static QSet < QString > flag_values;
    if (flag_values.isEmpty())
    {
        flag_values << "can_be_edited" << "can_manage_chat"
            << "can_change_info" << "can_delete_messages" << "can_invite_users"
            << "can_restrict_members" << "can_pin_messages"
            << "can_manage_topics" << "can_promote_members"
            << "can_manage_video_chats" << "can_post_stories"
            << "can_edit_stories" << "can_delete_stories" << "is_anonymous"
            << "can_manage_voice_chats" << "can_post_messages"
            << "can_edit_messages";
    }

    // Loop contents
    QHash < QString, QString > newchatmember_info;
    const QStringList all_keys = mcrNewChatMember.keys();
    for (const QString & key : all_keys)
    {
        if (key == "user")
        {
            const QHash < QString, QString > user_info =
                Parse_User(mcrNewChatMember[key].toObject());
            if (user_info.isEmpty())
            {
                const QString reason = tr("Error parsing user info");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            newchatmember_info["user_id"] = user_info["id"];
            continue;
        }

        if (key == "status")
        {
            newchatmember_info["status"] = mcrNewChatMember[key].toString();
            continue;
        }

        if (key == "until_date")
        {
            const qint64 since_epoch = mcrNewChatMember[key].toInteger();
            if (since_epoch == 0)
            {
                newchatmember_info["until_date"] = "";
            } else
            {
                const QDateTime date =
                    QDateTime::fromSecsSinceEpoch(since_epoch);
                newchatmember_info["until_date"] =
                    date.toString("yyyy-MM-dd hh:mm:ss");
            }
            continue;
        }

        if (flag_values.contains(key))
        {
            newchatmember_info[key] =
                mcrNewChatMember[key].toBool() ? "true" : "false";
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in new_chat_member")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    CALL_OUT("");
    return newchatmember_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if MyChatMember ID exists
bool TelegramComms::DoesMyChatMemberInfoExist(
    const qint64 mcMyChatMemberID) const
{
    CALL_IN(QString("mcMyChatMemberID=%1")
        .arg(CALL_SHOW(mcMyChatMemberID)));

    const bool exists = m_MyChatMemberIDToInfo.contains(mcMyChatMemberID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Chat
QHash < QString, QString > TelegramComms::GetMyChatMemberInfo(
    const qint64 mcMyChatMemberID)
{
    CALL_IN(QString("mcMyChatMemberID=%1")
        .arg(CALL_SHOW(mcMyChatMemberID)));

    // Check if we have this my chat member
    if (!m_MyChatMemberIDToInfo.contains(mcMyChatMemberID))
    {
        const QString reason = tr("My Chat Member ID %1 does not exist")
            .arg(QString::number(mcMyChatMemberID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_MyChatMemberIDToInfo[mcMyChatMemberID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: File
QHash < QString, QString > TelegramComms::Parse_File(
    const QJsonObject & mcrFile)
{
    CALL_IN(QString("mcrFile=%1")
        .arg(CALL_SHOW_FULL(mcrFile)));

    // Animation
    // {
    //   "duration":3,
    //   "file_id":"...Q",
    //   "file_name":"what-huh.mp4",
    //   "file_size":99407,
    //   "file_unique_id":"...",
    //   "height":288,
    //   "mime_type":"video/mp4",
    //   "width":288
    // }
    //
    // Document:
    // {
    //   "file_id":"...",
    //   "file_name":"...",
    //   "file_size":222334,
    //   "file_unique_id":"...",
    //   "mime_type":"application/pdf",
    //   "thumb":{...},
    //   "thumbnail":{...}
    // }
    //
    // Photo:
    // {
    //   "file_id":"[file_id]",
    //   "file_size":1505,
    //   "file_unique_id":"...",
    //   "height":81,
    //   "width":90
    // }
    //
    // Sticker:
    // {
    //   "width":512,
    //   "height":512,
    //   "emoji":"\ud83d\ude04",
    //   "set_name":"shimarongreywolf",
    //   "is_animated":false,
    //   "is_video":false,
    //   "type":"regular",
    //   "thumbnail":{...},
    //   "thumb":{...},
    //   "file_id":"...",
    //   "file_unique_id":"...",
    //   "file_size":25946
    // }

    // Special case with file infos: We get different versions of them.
    // - one e.g. in the list of stickers in a set
    // - one when we try and download a file (file_path)
    // So we need to merge information.

    // Get this file info
    QHash < QString, QString > file_info;
    const QStringList all_keys = mcrFile.keys();
    for (const QString & key : all_keys)
    {
        if (key == "duration")
        {
            file_info[key] = QString::number(mcrFile[key].toDouble());
            continue;
        }

        if (key == "emoji")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "file_id")
        {
            file_info[key] = mcrFile[key].toString();
            file_info["id"] = file_info[key];
            continue;
        }

        if (key == "file_name")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "file_path")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "file_size")
        {
            file_info[key] = QString::number(mcrFile[key].toInteger());
            continue;
        }

        if (key == "file_unique_id")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "height")
        {
            file_info[key] = QString::number(mcrFile[key].toInt());
            continue;
        }

        if (key == "is_animated")
        {
            file_info[key] = mcrFile[key].toBool() ? "true" : "false";
            continue;
        }

        if (key == "is_video")
        {
            file_info[key] = mcrFile[key].toBool() ? "true" : "false";
            continue;
        }

        if (key == "mime_type")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "premium_animation")
        {
            const QHash < QString, QString > premium_animation_info =
                Parse_File(mcrFile[key].toObject());
            file_info["premium_animation_file_id"] =
                premium_animation_info["id"];
            continue;
        }

        if (key == "set_name")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "thumb")
        {
            // Ignore
            continue;
        }

        if (key == "thumbnail")
        {
            // Ignore
            continue;
        }

        if (key == "type")
        {
            file_info[key] = mcrFile[key].toString();
            continue;
        }

        if (key == "width")
        {
            file_info[key] = QString::number(mcrFile[key].toInt());
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in file")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // If there was a previous file info, new information should match
    // existing information if present
    const QString file_id = file_info["id"];
    if (!m_FileIDToInfo.contains(file_id))
    {
        m_FileIDToInfo[file_id] = QHash < QString, QString >();
    }
    for (auto iterator_key = file_info.keyBegin();
         iterator_key != file_info.keyEnd();
         iterator_key++)
    {
        const QString key = *iterator_key;
        if (m_FileIDToInfo[file_id].contains(key))
        {
            if (m_FileIDToInfo[file_id][key] != file_info[key])
            {
                // Mismatch
                const QString reason = tr("File ID %1 has data mismatch "
                    "for key \"%2\": old: \"%3\", new: \"%4\"")
                    .arg(file_id,
                         key,
                         m_FileIDToInfo[file_id][key],
                         file_info[key]);
                qDebug().noquote() << reason;
            }
        } else
        {
            // Includen new information
            m_FileIDToInfo[file_id][key] = file_info[key];
        }
    }

    // Save file info
    SaveInfoData("file_info", m_FileIDToInfo[file_id], "text");

    CALL_OUT("");
    return m_FileIDToInfo[file_id];
}




///////////////////////////////////////////////////////////////////////////////
// Check if file info exists
bool TelegramComms::DoesFileInfoExist(const QString & mcrFileID) const
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    const bool exists = m_FileIDToInfo.contains(mcrFileID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// File info
QHash < QString, QString > TelegramComms::GetFileInfo(
    const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    // Check if we have this file
    if (!m_FileIDToInfo.contains(mcrFileID))
    {
        const QString reason = tr("File ID %1 does not exist")
            .arg(mcrFileID);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_FileIDToInfo[mcrFileID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: ButtonList
QHash < QString, QString > TelegramComms::Parse_ButtonList(
    const QJsonObject & mcrButtonList)
{
    CALL_IN(QString("mcrButtonList=%1")
        .arg(CALL_SHOW_FULL(mcrButtonList)));

    // {
    //   "inline_keyboard":
    //   [
    //     [
    //       {...},
    //       {...},
    //       {...},
    //       ...
    //     ],
    //     [
    //       {...},
    //       ...
    //     ]
    //   ]
    // }

    const QJsonArray button_row_array =
        mcrButtonList["inline_keyboard"].toArray();
    if (button_row_array.isEmpty())
    {
        const QString reason =
            tr("reply_markup did not have a \"inline_keyboard\" member.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    // Return value
    QHash < QString, QString > button_list;
    button_list["num_rows"] = QString::number(button_row_array.size());

    // Loop rows
    for (int row = 0;
         row < button_row_array.size();
         row++)
    {
        const QJsonArray this_row = button_row_array.at(row).toArray();
        if (this_row.isEmpty())
        {
            const QString reason = tr("Row %1 is empty.")
                .arg(QString::number(row));
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return QHash < QString, QString >();
        }

        const QString row_name = QString("row_%1")
            .arg(QString::number(row));
        button_list[row_name + "_num_cols"] =
            QString::number(this_row.size());

        // Loop columns
        for (int column = 0;
             column < this_row.size();
             column++)
        {
            const QJsonObject this_button = this_row.at(column).toObject();
            if (this_button.isEmpty())
            {
                const QString reason = tr("Row %1, column %2 is empty.")
                    .arg(QString::number(row),
                         QString::number(column));
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            const QHash < QString, QString > button =
                Parse_Button(this_button);

            const QString col_name = QString("col_%1")
                .arg(QString::number(column));
            button_list[row_name + "_" + col_name + "_button_id"] =
                button["id"];
        }
    }

    // Save button list info
    const qint64 button_list_id = m_NextButtonListID++;
    button_list["id"] = QString::number(button_list_id);
    m_ButtonListIDToInfo[button_list_id] = button_list;
    SaveInfoData("button_list_info", button_list);

    CALL_OUT("");
    return button_list;
}



///////////////////////////////////////////////////////////////////////////////
// Check if button list info exists
bool TelegramComms::DoesButtonListInfoExist(const qint64 mcButtonListID) const
{
    CALL_IN(QString("mcButtonListID=%1")
        .arg(CALL_SHOW(mcButtonListID)));

    const bool exists = m_ButtonListIDToInfo.contains(mcButtonListID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Button Lists
QHash < QString, QString > TelegramComms::GetButtonList(
    const qint64 mcButtonListID)
{
    CALL_IN(QString("mcButtonListID=%1")
        .arg(CALL_SHOW(mcButtonListID)));

    // Check if we have this button list
    if (!m_ButtonListIDToInfo.contains(mcButtonListID))
    {
        const QString reason = tr("Button list ID %1 does not exist")
            .arg(mcButtonListID);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_ButtonListIDToInfo[mcButtonListID];
}



///////////////////////////////////////////////////////////////////////////////
// Parse response: Button
QHash < QString, QString > TelegramComms::Parse_Button(
    const QJsonObject & mcrButton)
{
    CALL_IN(QString("mcrButton=%1")
        .arg(CALL_SHOW_FULL(mcrButton)));

    // {
    //   "callback_data":"/vote up 10346160",
    //   "text":"👍"
    // }

    // Loop contents
    QHash < QString, QString > button_info;
    const QStringList all_keys = mcrButton.keys();
    for (const QString & key : all_keys)
    {
        if (key == "callback_data")
        {
            button_info["callback_data"] = mcrButton[key].toString();
            continue;
        }

        if (key == "text")
        {
            button_info["text"] = mcrButton[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in button")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save button info
    const qint64 button_id = m_NextButtonID++;
    button_info["id"] = QString::number(button_id);
    m_ButtonIDToInfo[button_id] = button_info;
    SaveInfoData("button_info", button_info);

    CALL_OUT("");
    return button_info;
}



///////////////////////////////////////////////////////////////////////////////
// Check if button info exists
bool TelegramComms::DoesButtonInfoExist(const qint64 mcButtonID) const
{
    CALL_IN(QString("mcButtonID=%1")
        .arg(CALL_SHOW(mcButtonID)));

    const bool exists = m_ButtonIDToInfo.contains(mcButtonID);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Buttons
QHash < QString, QString > TelegramComms::GetButtonInfo(
    const qint64 mcButtonID)
{
    CALL_IN(QString("mcButtonID=%1")
        .arg(CALL_SHOW(mcButtonID)));

    // Check if we have this photo
    if (!m_ButtonIDToInfo.contains(mcButtonID))
    {
        const QString reason = tr("Button ID %1 does not exist")
            .arg(mcButtonID);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    CALL_OUT("");
    return m_ButtonIDToInfo[mcButtonID];
}



// =============================================================== Sticker Sets



///////////////////////////////////////////////////////////////////////////////
// Parse sticker set
QHash < QString, QString > TelegramComms::Parse_StickerSet(
    const QJsonObject & mcrStickerSet)
{
    CALL_IN(QString("mcrStickerSet=%1")
        .arg(CALL_SHOW(mcrStickerSet)));

    // {
    //   "name":"RakanHowls",
    //   "title":"@RakanHowls by Revaivwra",
    //   "sticker_type":"regular",
    //   "contains_masks":false,
    //   "stickers":
    //   [
    //     {...}
    //   ]
    // }

    // Check if we have parsed this sticker set previously
    const QString name = mcrStickerSet["name"].toString();
    if (m_StickerSetNameToInfo.contains(name))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_StickerSetNameToInfo[name];
    }

    // Loop contents
    QHash < QString, QString > stickerset_info;
    const QStringList all_keys = mcrStickerSet.keys();
    for (const QString & key : all_keys)
    {
        if (key == "contains_masks")
        {
            stickerset_info["contains_masks"] =
                (mcrStickerSet[key].toBool() ? "true" : "false");
            continue;
        }

        if (key == "name")
        {
            stickerset_info["name"] = mcrStickerSet[key].toString();
            stickerset_info["id"] = stickerset_info["name"];
            continue;
        }

        if (key == "sticker_type")
        {
            stickerset_info["sticker_type"] = mcrStickerSet[key].toString();
            continue;
        }

        if (key == "stickers")
        {
            QJsonArray all_stickers = mcrStickerSet[key].toArray();
            for (auto sticker_iterator = all_stickers.begin();
                 sticker_iterator != all_stickers.end();
                 sticker_iterator++)
            {
                const QJsonObject & sticker = sticker_iterator -> toObject();
                const QHash < QString, QString > sticker_info =
                    Parse_File(sticker);
                m_StickerSetNameToFileIDs[name] << sticker_info["id"];
            }
            continue;
        }

        if (key == "thumb")
        {
            // Ignore
            continue;
        }

        if (key == "thumbnail")
        {
            // Ignore
            continue;
        }

        if (key == "title")
        {
            stickerset_info["title"] = mcrStickerSet[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in sticker set")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save sticker info
    m_StickerSetNameToInfo[name] = stickerset_info;
    SaveInfoData_StickerSet("sticker_set_info", name);

    // Let everybody know
    emit StickerSetInfoReceived(name);

    // Done with this one...
    m_StickerSetInfoBeingDownloaded.clear();

    CALL_OUT("");
    return stickerset_info;
}



///////////////////////////////////////////////////////////////////////////////
// Does sticker set info exist?
bool TelegramComms::DoesStickerSetInfoExist(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    const bool exists = m_StickerSetNameToInfo.contains(mcrStickerSetName);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Get all sticker set names
QStringList TelegramComms::GetAllStickerSetNames() const
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
    return m_StickerSetNameToInfo.keys();
}



///////////////////////////////////////////////////////////////////////////////
// Remove existing sticker set info
bool TelegramComms::RemoveStickerSetInfo(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Check if we know this sticker set
    if (!m_StickerSetNameToInfo.contains(mcrStickerSetName))
    {
        // Nope
        const QString reason = tr("Unknown sticker set \"%1\".")
            .arg(mcrStickerSetName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Remove sticker set
    m_StickerSetNameToInfo.remove(mcrStickerSetName);
    m_StickerSetNameToFileIDs.remove(mcrStickerSetName);

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Get sticker set info
QHash < QString, QString > TelegramComms::GetStickerSetInfo(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Check if we know this sticker set
    if (!m_StickerSetNameToInfo.contains(mcrStickerSetName))
    {
        // Nope
        const QString reason = tr("Unknown sticker set \"%1\".")
            .arg(mcrStickerSetName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }

    const QHash < QString, QString > & sticker_set_info =
        m_StickerSetNameToInfo[mcrStickerSetName];

    CALL_OUT("");
    return sticker_set_info;
}



///////////////////////////////////////////////////////////////////////////////
// Get sticker set stickers (file IDs)
QStringList TelegramComms::GetStickerSetFileIDs(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Check if we know this sticker set
    if (!m_StickerSetNameToFileIDs.contains(mcrStickerSetName))
    {
        // Nope
        const QString reason = tr("Unknown sticker set \"%1\".")
            .arg(mcrStickerSetName);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QStringList();
    }

    const QStringList & sticker_list =
        m_StickerSetNameToFileIDs[mcrStickerSetName];

    CALL_OUT("");
    return sticker_list;
}



///////////////////////////////////////////////////////////////////////////////
// Download sticker set info
void TelegramComms::DownloadStickerSetInfo(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Add to download queue
    m_StickerSetInfo_DownloadQueue << mcrStickerSetName;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Check for sticker set infos to be downloaded
void TelegramComms::Periodic_DownloadStickerSetInfo()
{
    CALL_IN("");

    if (m_StickerSetInfoBeingDownloaded.isEmpty() &&
        !m_StickerSetInfo_DownloadQueue.isEmpty())
    {
        // Build URL
        const QString sticker_set_name =
            m_StickerSetInfo_DownloadQueue.takeFirst();
        QString url = QString("https://api.telegram.org/bot%1/getStickerSet?"
            "name=%2")
            .arg(m_Token,
                 sticker_set_name);
        QNetworkRequest request;
        request.setUrl(url);
        m_NetworkAccessManager -> get(request);

        // Remember we're downloading this set
        m_StickerSetInfoBeingDownloaded = sticker_set_name;
    }

    // Try again in a bit
    QTimer::singleShot(DOWNLOAD_DELAY,
        this, &TelegramComms::Periodic_DownloadStickerSetInfo);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
void TelegramComms::Error_StickerSetInvalid()
{
    CALL_IN("");

    // Let the world know
    emit StickerSetInfoFailed(m_StickerSetInfoBeingDownloaded);

    // Done with this one
    m_StickerSetInfoBeingDownloaded.clear();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Channel Post
QHash < QString, QString > TelegramComms::Parse_ChannelPost(
    const QJsonObject & mcrChannelPost)
{
    CALL_IN(QString("mcrChannelPost=%1")
        .arg(CALL_SHOW(mcrChannelPost)));

    // "channel_post":
    // {
    //   "caption":"میگن سوال امروز ریاضیاس \n\n@konkorfinity",
    //   "caption_entities":
    //   [
    //     {
    //       "length":13,
    //       "offset":26,
    //       "type":"mention"
    //     }
    //   ],
    //   "chat": {...}
    //   "date":1746077870,
    //   "message_id":73,
    //   "photo":
    //   [
    //     {
    //       "file_id":"...",
    //       "file_size":434,
    //       "file_unique_id":"...",
    //       "height":18,
    //       "width":90
    //     },
    //     {
    //       "file_id":"...",
    //       "file_size":3645,
    //       "file_unique_id":"...",
    //       "height":63,
    //       "width":320
    //     },
    //     {
    //       "file_id":"...",
    //       "file_size":17706,
    //       "file_unique_id":"...",
    //       "height":158,
    //       "width":800
    //     },
    //     {
    //       "file_id":"...",
    //       "file_size":35270,
    //       "file_unique_id":"...",
    //       "height":253,
    //       "width":1280
    //     }
    //   ],
    //   "sender_chat":{...}
    // }

    // Check if we have parsed this message previously
    const qint64 message_id = mcrChannelPost["message_id"].toInteger();
    if (m_MessageIDToChannelPostInfo.contains(message_id))
    {
        // Nothing to do here.
        CALL_OUT("");
        return m_MessageIDToChannelPostInfo[message_id];
    }

    // Parse the new message

    // Loop contents
    QHash < QString, QString > channel_post_info;
    const QStringList all_keys = mcrChannelPost.keys();
    for (const QString & key : all_keys)
    {
        if (key == "caption")
        {
            channel_post_info["caption"] = mcrChannelPost[key].toString();
            continue;
        }

        if (key == "caption_entities")
        {
            // Ignored
            continue;
        }

        if (key == "chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrChannelPost["chat"].toObject());
            channel_post_info["chat_id"] = chat_info["id"];
            continue;
        }

        if (key == "date")
        {
            const QDateTime date =
                QDateTime::fromSecsSinceEpoch(mcrChannelPost[key].toInt());
            channel_post_info["date_time"] =
                date.toString("yyyy-MM-dd hh:mm:ss");
            continue;
        }

        if (key == "document")
        {
            const QHash < QString, QString > file_info =
                Parse_File(mcrChannelPost["document"].toObject());
            channel_post_info["document_file_id"] = file_info["id"];
            continue;
        }

        if (key == "entities")
        {
            // Ignored
            continue;
        }

        if (key == "media_group_id")
        {
            channel_post_info["media_group_id"] =
                mcrChannelPost[key].toString();
            continue;
        }

        if (key == "message_id")
        {
            // That's probably the message ID of the channel posting this...
            const qint64 message_id = mcrChannelPost[key].toInteger();
            channel_post_info["message_id"] = QString::number(message_id);
            channel_post_info["id"] = QString::number(message_id);
            continue;
        }

        if (key == "photo")
        {
            QJsonArray photos = mcrChannelPost[key].toArray();
            QJsonObject last_photo = photos.last().toObject();
            if (last_photo.isEmpty())
            {
                // Not successful
                const QString reason =
                    tr("photo array did not have a last entry.");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            const QHash < QString, QString > photo_info =
                Parse_File(last_photo);
            if (photo_info.isEmpty())
            {
                const QString reason =
                    tr("Error parsing in photos list (photo)");
                MessageLogger::Error(CALL_METHOD, reason);
                CALL_OUT(reason);
                return QHash < QString, QString >();
            }
            channel_post_info["photo_file_id"] = photo_info["id"];
            continue;
        }

        if (key == "sender_chat")
        {
            const QHash < QString, QString > chat_info =
                Parse_Chat(mcrChannelPost["chat"].toObject());
            channel_post_info["sender_chat_id"] = chat_info["id"];
            continue;
        }

        if (key == "text")
        {
            channel_post_info["text"] = mcrChannelPost[key].toString();
            continue;
        }

        // Unknown key
        const QString message = tr("Unknown key \"%1\" in channel post set")
            .arg(key);
        MessageLogger::Error(CALL_METHOD, message);
    }

    // Save message info
    if (!channel_post_info.contains("id"))
    {
        const QString reason = tr("Channel Post Message is missing an ID");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QHash < QString, QString >();
    }
    m_MessageIDToChannelPostInfo[message_id] = channel_post_info;
    SaveInfoData("channel_post_info", channel_post_info);

    // Let everybody know
    const qint64 chat_id = channel_post_info["chat_id"].toLongLong();
    emit ChannelPostReceived(chat_id, message_id);

    CALL_OUT("");
    return channel_post_info;
}



///////////////////////////////////////////////////////////////////////////////
// Download file
void TelegramComms::DownloadFile(const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    // Check if file already has been downloaded
    if (HasFileBeenDownloaded(mcrFileID))
    {
        // Nothing to do. Same file ID, same file content.
        emit FileDownloaded(mcrFileID);
        CALL_OUT("");
        return;
    }

    // Add to download queue
    m_DownloadQueue << mcrFileID;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Download queue/worklist size
int TelegramComms::GetDownloadWorkListSize() const
{
    CALL_IN("");

    const int queue_size = m_DownloadQueue.size();

    CALL_OUT("");
    return queue_size;
}



///////////////////////////////////////////////////////////////////////////////
// Download some files
// (avoids too many requests in a short period of time)
void TelegramComms::Periodic_DownloadFiles()
{
    CALL_IN("");

    if (!m_DownloadQueue.isEmpty())
    {
        // Build URL
        const QString file_id = m_DownloadQueue.takeFirst();
        QString url = QString("https://api.telegram.org/bot%1/getFile?"
            "file_id=%2")
            .arg(m_Token,
                 file_id);
        QNetworkRequest request;
        request.setUrl(url);
        m_NetworkAccessManager -> get(request);
    }

    // Try again in a bit
    QTimer::singleShot(DOWNLOAD_DELAY,
        this, &TelegramComms::Periodic_DownloadFiles);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// File path
bool TelegramComms::Download_FilePath(const QString & mcrFileID,
    const QString & mcrFilePath)
{
    CALL_IN(QString("mcrFileID=%1, mcrFilePath=%2")
        .arg(CALL_SHOW(mcrFileID),
             CALL_SHOW(mcrFilePath)));

    // As the result of triggering a download, we get the info of where to
    // download the actual file (file_path)

    // Build URL
    QString url = QString("https://api.telegram.org/file/bot%1/%2")
        .arg(m_Token,
             mcrFilePath);
    m_FilePathToFileID[mcrFilePath] = mcrFileID;

    QNetworkRequest request;
    request.setUrl(url);
    m_NetworkAccessManager -> get(request);

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Save binary file
bool TelegramComms::SaveFile(const QString & mcrFilePath,
    const QByteArray & mcrData)
{
    CALL_IN(QString("mcrFilePath=%1, mcrData=%2")
        .arg(CALL_SHOW(mcrFilePath),
             CALL_SHOW(mcrData)));

    // Check if we received data
    if (mcrData.isEmpty())
    {
        const QString reason = tr("No data received.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Get File ID
    const QString file_id = m_FilePathToFileID[mcrFilePath];
    m_FilePathToFileID.remove(mcrFilePath);

    // Save file
    const QString filename = BOT_FILES + file_id;
    QFile out_file(filename);
    out_file.open(QFile::WriteOnly);
    out_file.write(mcrData);
    out_file.close();

    // Let everybody know
    emit FileDownloaded(file_id);

    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Check if a file has been downloaded
bool TelegramComms::HasFileBeenDownloaded(const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    const QString filename = BOT_FILES + mcrFileID;
    const bool downloaded = QFile::exists(filename);

    CALL_OUT("");
    return downloaded;
}



///////////////////////////////////////////////////////////////////////////////
// Get actual file data
QByteArray TelegramComms::GetFile(const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    if (!HasFileBeenDownloaded(mcrFileID))
    {
        const QString reason = tr("File ID \"%1\" has not been downloaded.")
            .arg(mcrFileID);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return QByteArray();
    }

    const QString filename = BOT_FILES + mcrFileID;
    QFile in_file(filename);
    in_file.open(QFile::ReadOnly);
    QByteArray data = in_file.readAll();
    in_file.close();

    CALL_OUT("");
    return data;
}



// ====================================================== Sending to the Server



///////////////////////////////////////////////////////////////////////////////
// Setting available commands
void TelegramComms::SetMyCommands(const QJsonObject & mcrAvailableCommands)
{
    CALL_IN(QString("mcrAvailableCommands=%1")
        .arg(CALL_SHOW(mcrAvailableCommands)));

    const QString json_commands =
        QJsonDocument(mcrAvailableCommands["commands"].toArray())
            .toJson(QJsonDocument::Compact);
    const QString json_scope =
        QJsonDocument(mcrAvailableCommands["scope"].toObject())
            .toJson(QJsonDocument::Compact);
    const QString url = QString("https://api.telegram.org/bot%1/setMyCommands?"
        "commands=%2&scope=%3")
        .arg(m_Token,
             json_commands,
             json_scope);
    QNetworkRequest request;
    request.setUrl(url);
    m_NetworkAccessManager -> get(request);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Sending messages
void TelegramComms::SendMessage(const qint64 mcChatID,
    const QString & mcrMessage)
{
    CALL_IN(QString("mcChatID=%1, mcrMessage=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrMessage)));

    // Add chat to active ones
    m_ActiveChats += mcChatID;

    // Some necessary replacements
    QString message = mcrMessage;
    // "%" must be first
    message.replace("%", "%25");
    message.replace("\n", "%0A");
    message.replace(" ", "%20");
    message.replace("\"", "%22");
    message.replace("&", "%26");

    QString url = QString("https://api.telegram.org/bot%1/sendMessage?"
        "parse_mode=html&"
        "chat_id=%2&"
        "text=%3")
        .arg(m_Token,
             QString::number(mcChatID),
             message);
    QNetworkRequest request;
    request.setUrl(url);
    m_NetworkAccessManager -> get(request);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Send broadcast messages
void TelegramComms::SendBroadcastMessage(const QString & mcrMessage)
{
    CALL_IN(QString("mcrMessage=%1")
        .arg(CALL_SHOW(mcrMessage)));

    for (auto chat_iterator = m_ActiveChats.constBegin();
         chat_iterator != m_ActiveChats.constEnd();
         chat_iterator++)
    {
        const qint64 chat_id = *chat_iterator;
        SendMessage(chat_id, mcrMessage);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Sending reply
void TelegramComms::SendReply(const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrMessage)
{
    CALL_IN(QString("mcChatID=%1, mcMessageID=%2, mcrMessage=%3")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrMessage)));

    // Add chat to active ones
    m_ActiveChats += mcChatID;

    // Some necessary replacements
    QString message = mcrMessage;
    // "%" must be first
    message.replace("%", "%25");
    message.replace("\n", "%0A");
    message.replace(" ", "%20");
    message.replace("\"", "%22");
    message.replace("&", "%26");


    // Reply information
    QJsonObject reply_parameters_obj;
    reply_parameters_obj.insert("message_id", mcMessageID);
    QJsonDocument reply_parameters(reply_parameters_obj);
    QString reply_parameter_json =
        reply_parameters.toJson(QJsonDocument::Compact);
    reply_parameter_json.replace("\"", "%22");

    // Build URL
    QString url = QString("https://api.telegram.org/bot%1/sendMessage?"
        "parse_mode=html&"
        "chat_id=%2&"
        "reply_parameters=%3&"
        "text=%4")
        .arg(m_Token,
             QString::number(mcChatID),
             reply_parameter_json,
             message);
    QNetworkRequest request;
    request.setUrl(url);
    m_NetworkAccessManager -> get(request);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Upload a file to a chat
bool TelegramComms::UploadFile(const qint64 mcChatID,
    const QString & mcrFilename)
{
    CALL_IN(QString("mcrChatID=%1, mcrFilename=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrFilename)));

    // Add chat to active ones
    m_ActiveChats += mcChatID;

    // Get binary data
    QFile in_file(mcrFilename);
    if (!in_file.open(QFile::ReadOnly))
    {
        const QString reason = tr("Could not open file \"%1\".")
            .arg(mcrFilename);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }
    QByteArray file_data = in_file.readAll();
    if (file_data.isEmpty())
    {
        const QString reason = tr("File \"%1\" has no data.")
            .arg(mcrFilename);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Send via multipart/form-data
    const QPair < QString, QString > split_filename =
        StringHelper::SplitFilename(mcrFilename);
    const QString file = split_filename.second;

    // Build request
    QString url = QString("https://api.telegram.org/bot%1/sendDocument")
        .arg(m_Token);
    QNetworkRequest request(url);

    const QString boundary = "StickerBoundary";

    request.setHeader(QNetworkRequest::ContentTypeHeader,
        QString("multipart/form-data; boundary=%1")
            .arg(boundary)
            .toLocal8Bit());

    QByteArray payload;

    // Chat ID
    payload.append(QString("--%1\r\n")
        .arg(boundary)
        .toLocal8Bit());
    payload.append(
        QString("Content-Disposition: form-data; name=\"%1\"\r\n")
        .arg("chat_id")
        .toLocal8Bit());
    payload.append("\r\n");
    payload.append(QString("%1\r\n")
        .arg(QString::number(mcChatID))
        .toLocal8Bit());

    // Data
    payload.append(QString("--%1\r\n")
        .arg(boundary)
        .toLocal8Bit());
    payload.append(
        QString("Content-Disposition: form-data; name=\"%1\"; "
            "filename=\"%2\"\r\n")
        .arg("document",
             file)
        .toLocal8Bit());
    payload.append("Content-Type: application/zip\r\n");
    // payload.append("Content-Transfer-Encoding: base64\r\n");
    payload.append("\r\n");
    // payload.append(file_data.toBase64());
    payload.append(file_data);
    payload.append("\r\n");

    // Closing boundary
    payload.append(QString("--%1--\r\n")
        .arg(boundary)
        .toLocal8Bit());

    m_NetworkAccessManager -> post(request, payload);

    CALL_OUT("");
    return true;
}



// ====================================================================== Debug



///////////////////////////////////////////////////////////////////////////////
// Dump everything
void TelegramComms::Dump() const
{
    CALL_IN("");

    // == Updates
    qDebug().noquote() << "===== Updates";
    QList < qint64 > all_keys = m_UpdateIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_UpdateIDToInfo[key]));
    }

    // == Messages
    qDebug().noquote() << "===== Messages";
    all_keys = m_MessageIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_MessageIDToInfo[key]));
    }

    // == Users
    qDebug().noquote() << "===== Users";
    all_keys = m_UserIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_UserIDToInfo[key]));
    }

    // == Chats
    qDebug().noquote() << "===== Chats";
    all_keys = m_ChatIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_ChatIDToInfo[key]));
    }

    // == MyChatMembers
    qDebug().noquote() << "===== MyChatMembers";
    all_keys = m_MyChatMemberIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_MyChatMemberIDToInfo[key]));
    }

    // == Files
    qDebug().noquote() << "===== Files";
    QStringList all_keys_str = m_FileIDToInfo.keys();
    std::sort(all_keys_str.begin(), all_keys_str.end());
    for (auto key_iterator = all_keys_str.constBegin();
         key_iterator != all_keys_str.constEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(key,
                 CALL_SHOW(m_FileIDToInfo[key]));
    }

    // == Button lists
    qDebug().noquote() << "===== Button Lists";
    all_keys = m_ButtonListIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_ButtonListIDToInfo[key]));
    }

    // == Buttons
    qDebug().noquote() << "===== Buttons";
    all_keys = m_ButtonIDToInfo.keys();
    std::sort(all_keys.begin(), all_keys.end());
    for (auto key_iterator = all_keys.constBegin();
         key_iterator != all_keys.constEnd();
         key_iterator++)
    {
        const qint64 key = *key_iterator;
        qDebug().noquote() << QString("%1: %2")
            .arg(QString::number(key),
                 CALL_SHOW(m_ButtonIDToInfo[key]));
    }

    CALL_OUT("");
}
