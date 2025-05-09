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

// TelegramHelper.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "Config.h"
#include "MessageLogger.h"
#include "TelegramComms.h"
#include "TelegramHelper.h"

// Qt includes
#include <QDir>
#include <QProcess>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
TelegramHelper::TelegramHelper()
{
    CALL_IN("");

    // Create some directories
    QDir files("/");
    if (!files.exists(USER_FILES))
    {
        files.mkpath(USER_FILES);
    }
    if (!files.exists(USER_STICKERSETS))
    {
        files.mkpath(USER_STICKERSETS);
    }

    // Connect some signals
    TelegramComms * tc = TelegramComms::Instance();
    connect (tc, SIGNAL(MessageReceived(const qint64, const qint64)),
        this, SLOT(Server_MessageReceieved(const qint64, const qint64)));
    connect (tc, SIGNAL(FileDownloaded(const QString &)),
        this, SLOT(Server_FileDownloaded(const QString &)));
    connect (tc, SIGNAL(StickerSetInfoReceived(const QString &)),
        this, SLOT(Server_StickerSetInfoReceived(const QString &)));

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
TelegramHelper::~TelegramHelper()
{
    CALL_IN("");

    // Nothing to do

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Instanciator
TelegramHelper * TelegramHelper::Instance()
{
    CALL_IN("");

    // Check if we already have an instance
    if (!m_Instance)
    {
        // Nope. Create one.
        m_Instance = new TelegramHelper;
    }

    // Return instance
    CALL_OUT("");
    return m_Instance;
}



///////////////////////////////////////////////////////////////////////////////
// Instance
TelegramHelper * TelegramHelper::m_Instance = nullptr;



// =================================================================== Messages



///////////////////////////////////////////////////////////////////////////////
// A message has been received from the server
void TelegramHelper::Server_MessageReceieved(const qint64 mcChatID,
    const qint64 mcMessageID)
{
    CALL_IN(QString("mcChatID=%1, mcMessageID=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID)));

    // Pass on
    emit MessageReceived(mcChatID, mcMessageID);

    // Get message info
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > message_info =
        tc -> GetMessageInfo(mcMessageID);
    const qint64 user_id = message_info["from_id"].toLongLong();
    const QString text = message_info["text"];

    // Check if the message is from the bot
    const QHash < QString, QString > user_info = tc -> GetUserInfo(user_id);
    if (user_info["is_bot"] == "true")
    {
        // Nothing to do here.
        CALL_OUT("");
        return;
    }

    // Check if it's a command
    static const QRegularExpression format_command(
        "^/([a-zA-Z0-9_]+)(@([a-zA-Z0-9_]+))?( (.*))?$");
    // Could be multiple commands
    const QStringList split_lines = text.split("\n");
    for (const QString & line : split_lines)
    {
        QRegularExpressionMatch match_command = format_command.match(line);
        if (match_command.hasMatch())
        {
            const QString command = match_command.captured(1);
            const QString bot_name = match_command.captured(3);
            const QString parameters = match_command.captured(5).trimmed();

            // Check our bot is addressed
            if (bot_name.isEmpty() ||
                bot_name == BOT_NAME)
            {
                emit CommandReceived(user_id, mcChatID, mcMessageID, command,
                    parameters);

                if (parameters.isEmpty())
                {
                    // Assuming every command not taking an attribute has the
                    // forwarded message immediately after this command as
                    // a parameter
                    m_UserIDToSeparateMessageID[user_id] = mcMessageID + 1;
                }
            }
        }
    }

    // Check for "separate" command parameters
    // When a command is issued by forwarding a message, Telegram creates
    // two messages: One with the command, the other with the forwarded
    // message. We call this second message "separate parameter"
    if (message_info.contains("forward_date_time"))
    {
        // Check if we have been expecting this
        if (m_UserIDToSeparateMessageID.contains(user_id) &&
            m_UserIDToSeparateMessageID[user_id] == mcMessageID)
        {
            emit CommandSeparateMessageReceived(user_id, mcMessageID);
            m_UserIDToSeparateMessageID.remove(user_id);
        }
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Send message
bool TelegramHelper::SendMessage(const qint64 mcChatID,
    const QString & mcrMessage)
{
    CALL_IN(QString("mcChatID=%1, mcrMessage=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrMessage)));

    // Check if chat ID exists
    TelegramComms * tc = TelegramComms::Instance();
    if (!tc -> DoesChatInfoExist(mcChatID))
    {
        const QString reason = tr("Chat %1 does not exist.")
            .arg(QString::number(mcChatID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Chek is we actually have a message
    if (mcrMessage.isEmpty())
    {
        const QString reason = tr("Cannot send an empty essage.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Send message
    tc -> SendMessage(mcChatID, mcrMessage);

    CALL_OUT("");
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Send a reply
bool TelegramHelper::SendReply(const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrMessage)
{
    CALL_IN(QString("mcChatID=%1, mcrMessage=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrMessage)));

    // Check if chat ID exists
    TelegramComms * tc = TelegramComms::Instance();
    if (!tc -> DoesChatInfoExist(mcChatID))
    {
        const QString reason = tr("Chat %1 does not exist.")
            .arg(QString::number(mcChatID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    if (!tc -> DoesMessageInfoExist(mcMessageID))
    {
        const QString reason = tr("Forwarded Message ID %1 does not exist.")
            .arg(QString::number(mcMessageID));
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Chek is we actually have a message
    if (mcrMessage.isEmpty())
    {
        const QString reason = tr("Cannot send an empty essage.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return false;
    }

    // Send message
    tc -> SendMessage(mcChatID, mcrMessage);

    CALL_OUT("");
    return true;
}



// ====================================================================== Files



///////////////////////////////////////////////////////////////////////////////
// A file download has been completed
void TelegramHelper::Server_FileDownloaded(const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    // Let everybody know
    emit FileDownloaded(mcrFileID);

    // Check for sticker file processing
    if (m_FileIDToStickerSetNames.contains(mcrFileID))
    {
        StickerFileReceived(mcrFileID);
    }

    CALL_OUT("");
}



// =================================================================== Stickers



///////////////////////////////////////////////////////////////////////////////
// Check if we're already downloading a particular sticker set
bool TelegramHelper::IsStickerSetBeingDownloaded(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    const bool is_being_downloaded =
        m_StickerSetIsDownloading.contains(mcrStickerSetName);

    CALL_OUT("");
    return is_being_downloaded;
}



///////////////////////////////////////////////////////////////////////////////
// Download an entire sticker set
void TelegramHelper::DownloadStickerSet(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // (1) Add flag that we're downloading this sticker set, so the other
    //     methods receiving updates know to go here.
    //
    // (2) If no sticker set info is available for this sticker set, get it.
    //     This will also generate file infos for all stickers
    //
    // Loop sticker files:
    // (3) If file has not been downloaded, obtain it.
    //
    // (4) If the sticker set ZIP file does not exist, create it
    //
    // (5) Remove download flag & let outside world know sticker set zip
    //     file is now available.

    // (1) Add flag that we're downloading this sticker set, so the other
    //     methods receiving updates know to go here.
    if (!m_StickerSetIsDownloading.contains(mcrStickerSetName))
    {
        m_StickerSetIsDownloading += mcrStickerSetName;
    }

    // (2) If no sticker set info is available for this sticker set, get it.
    //     This will also generate file infos for all stickers
    TelegramComms * tc = TelegramComms::Instance();
    if (!tc -> DoesStickerSetInfoExist(mcrStickerSetName))
    {
        // Download detaches
        tc -> DownloadStickerSetInfo(mcrStickerSetName);
        CALL_OUT("");
        return;
    }

    // Loop sticker files:
    const QStringList sticker_file_ids =
        tc -> GetStickerSetFileIDs(mcrStickerSetName);
    if (!m_StickerSetToRemainingFileIDs.contains(mcrStickerSetName))
    {
        m_StickerSetToRemainingFileIDs[mcrStickerSetName] = QSet < QString >();
        for (const QString & sticker_file_id : sticker_file_ids)
        {
            // (3) If file has not been downloaded, obtain it
            // DownloadFile() will determine if file needs to be downloaded
            m_StickerSetToRemainingFileIDs[mcrStickerSetName]
                << sticker_file_id;
            m_FileIDToStickerSetNames[sticker_file_id]
                += mcrStickerSetName;

            // Download detaches
            tc -> DownloadFile(sticker_file_id);
        }
        if (!m_StickerSetToRemainingFileIDs[mcrStickerSetName].isEmpty())
        {
            CALL_OUT("");
            return;
        }
    }

    // (4) If the sticker set ZIP file does not exist, create it
    if (!DoesStickerSetZIPFileExist(mcrStickerSetName))
    {
        SaveStickerSetZIPFile(mcrStickerSetName);
    }

    // (5) Remove download flag & let outside world know sticker set zip
    //     file is now available.
    m_StickerSetIsDownloading -= mcrStickerSetName;
    emit StickerSetReceived(mcrStickerSetName);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Derver notified us it received the sticker set info
void TelegramHelper::Server_StickerSetInfoReceived(
    const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Let everybody know
    emit StickerSetInfoReceived(mcrStickerSetName);

    // Check if this is part of a sticker set download
    if (m_StickerSetIsDownloading.contains(mcrStickerSetName))
    {
        DownloadStickerSet(mcrStickerSetName);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Single sticker file has been received
void TelegramHelper::StickerFileReceived(const QString & mcrFileID)
{
    CALL_IN(QString("mcrFileID=%1")
        .arg(CALL_SHOW(mcrFileID)));

    // We received this file
    for (auto set_name_iterator =
            m_FileIDToStickerSetNames[mcrFileID].constBegin();
         set_name_iterator != m_FileIDToStickerSetNames[mcrFileID].constEnd();
         set_name_iterator++)
    {
        const QString & set_name = *set_name_iterator;
        m_StickerSetToRemainingFileIDs[set_name] -= mcrFileID;

        // Are there more file to be received?
        if (m_StickerSetToRemainingFileIDs[set_name].isEmpty())
        {
            // Everything's downloaded.
            DownloadStickerSet(set_name);
        }
    }

    m_FileIDToStickerSetNames.remove(mcrFileID);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save ZIP file with all stickers in a set
void TelegramHelper::SaveStickerSetZIPFile(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Make target directory
    QDir dir(USER_STICKERSETS);
    if (!dir.exists(mcrStickerSetName))
    {
        dir.mkdir(mcrStickerSetName);
    }

    // Get all files in the sticker set
    TelegramComms * tc = TelegramComms::Instance();
    const QStringList sticker_ids =
        tc -> GetStickerSetFileIDs(mcrStickerSetName);

    // Save them in order
    int sticker_count = 1;
    for (const QString & file_id : sticker_ids)
    {
        // Determine file extension
        const QHash < QString, QString > file_info =
            tc -> GetFileInfo(file_id);
        const QString extension =
            (file_info["is_animated"] == "false" ? "webp" : "tgs");

        // Filename
        const QString number =
            QString("00" + QString::number(sticker_count)).right(3);
        const QString filename = QString("%1/%2/Sticker_%3.%4")
            .arg(USER_STICKERSETS,
                 mcrStickerSetName,
                 number,
                 extension);

        // Save file
        const QByteArray data = tc -> GetFile(file_id);
        QFile out_file(filename);
        out_file.open(QFile::WriteOnly);
        out_file.write(data);
        out_file.close();

        sticker_count++;
    }

    // Create zip file
    QStringList arguments;
    const QString zip_filename = GetStickerSetZIPFilename(mcrStickerSetName);
    arguments
        << "-9"
        << "-r"
        << zip_filename
        << mcrStickerSetName;
    QProcess * process = new QProcess(this);
    process -> setWorkingDirectory(USER_STICKERSETS);
    process -> start("zip", arguments);
    process -> waitForFinished();
    const int exit_status = process -> exitStatus();
    const QString std_err(process -> readAllStandardError());
    delete process;

    if (exit_status != QProcess::NormalExit)
    {
        // An error occurred
        const QString reason = tr("An error occurred creating the sticker "
            "set zip file \"%1.zip\": %2")
            .arg(mcrStickerSetName,
                 std_err);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    // Let the world know
    emit StickerSetReceived(mcrStickerSetName);

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Check if sticker set ZIP file exists
bool TelegramHelper::DoesStickerSetZIPFileExist(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    const QString filename = GetStickerSetZIPFilename(mcrStickerSetName);
    const bool exists = QFile::exists(filename);

    CALL_OUT("");
    return exists;
}



///////////////////////////////////////////////////////////////////////////////
// Get (local) filename of the ZIP file with all stickers in that set
QString TelegramHelper::GetStickerSetZIPFilename(
    const QString & mcrStickerSetName) const
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    const QString zip_filename = USER_STICKERSETS + mcrStickerSetName + ".zip";

    CALL_OUT("");
    return zip_filename;
}
