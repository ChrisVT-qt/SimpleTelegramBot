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

// MainWindow.cpp
// Class definition

// Project includes
#include "CallTracer.h"
#include "Config.h"
#include "MainWindow.h"
#include "MessageLogger.h"
#include "StringHelper.h"
#include "TelegramComms.h"
#include "TelegramHelper.h"

// Qt includes
#include <QDir>
#include <QGridLayout>
#include <QPainter>
#include <QRegularExpressionMatch>
#include <QScrollBar>
#include <QThread>
#include <QTimer>


// Frequency of status updates
#define STATUS_REFRESH_DELAY 2*1000



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
MainWindow::MainWindow()
{
    CALL_IN("");

    // Not shutting down
    m_ShuttingDown = false;

    // Initialize bot
    TelegramHelper * th = TelegramHelper::Instance();
    connect (th, SIGNAL(MessageReceived(const qint64, const qint64)),
        this, SLOT(MessageReceived(const qint64, const qint64)));
    connect (th,
        SIGNAL(CommandReceived(qint64, qint64, qint64, QString, QString)),
        this,
        SLOT(CommandReceived(qint64, qint64, qint64, QString, QString)));
    connect (th, SIGNAL(StickerSetInfoReceived(QString)),
        this, SLOT(StickerSetInfoReceived(QString)));
    connect (th, SIGNAL(CommandSeparateMessageReceived(qint64, qint64)),
        this, SLOT(CommandSeparateMessageReceived(qint64, qint64)));
    connect (th, SIGNAL(StickerSetReceived(const QString &)),
        this, SLOT(StickerSetReceived(const QString &)));

    TelegramComms * tc = TelegramComms::Instance();
    connect (tc, SIGNAL(StickerSetInfoFailed(const QString &)),
        this, SLOT(StickerSetInfoFailed(const QString &)));

    // Initialize Widgets
    InitWidgets();

    // Start bot
    tc -> StartBot();

    // Register commands
    RegisterCommands();

    // Start status updates
    UpdateStatus();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
MainWindow::~MainWindow()
{
    CALL_IN("");

    // Nothing to do

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Instanciator
MainWindow * MainWindow::Instance()
{
    CALL_IN("");

    if (!m_Instance)
    {
        m_Instance = new MainWindow();
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
MainWindow * MainWindow::m_Instance = nullptr;



// ======================================================================== GUI



///////////////////////////////////////////////////////////////////////////////
// All GUI stuff
void MainWindow::InitWidgets()
{
    CALL_IN("");

    setWindowTitle(QString("SimpleTelegramBot - %1").arg(BOT_NAME));

    QWidget * central_widget = new QWidget();
    setCentralWidget(central_widget);
    QVBoxLayout * layout = new QVBoxLayout();
    central_widget -> setLayout(layout);
    
    // Log
    m_TabWidget = new QTabWidget();
    m_TabWidget -> setMinimumSize(800, 300);
    layout -> addWidget(m_TabWidget);

    // Bottom row
    QHBoxLayout * bottom_layout = new QHBoxLayout();
    layout -> addLayout(bottom_layout);

    m_Status = new QLabel();
    bottom_layout -> addWidget(m_Status);

    bottom_layout -> addStretch(1);

    QPushButton * pb_special = new QPushButton(tr("Shut Down"));
    connect (pb_special, SIGNAL(clicked()),
        this, SLOT(GracefullyShutDown()));
    bottom_layout -> addWidget(pb_special);

    layout -> setStretch(0, 1);
    layout -> setStretch(1, 0);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Update bot status
void MainWindow::UpdateStatus()
{
    CALL_IN("");

    // Get download queue size
    TelegramComms * tc = TelegramComms::Instance();
    const int queue_size = tc -> GetDownloadWorkListSize();
    m_Status -> setText(tr("Download queue: %1 files")
        .arg(QString::number(queue_size)));

    // See you again in 2s
    QTimer::singleShot(STATUS_REFRESH_DELAY,
        this, &MainWindow::UpdateStatus);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Gracefully shut down
void MainWindow::GracefullyShutDown()
{
    CALL_IN("");

    // No new comnands
    m_ShuttingDown = true;

    // Let all chats know
    const QString message =
        tr("Bot will shutdown after current acticities have been completed.");
    TelegramComms * tc = TelegramComms::Instance();
    tc -> SendBroadcastMessage(message);

    // Check if there is current work going on
    if (!CommandsBeingExecuted())
    {
        // We can quit
        close();
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Check if commands are being executed
bool MainWindow::CommandsBeingExecuted() const
{
    CALL_IN("");

    bool commands_being_executed = false;
    while (true)
    {
        // Downloads
        TelegramComms * tc = TelegramComms::Instance();
        const int queue_size = tc -> GetDownloadWorkListSize();
        if (queue_size > 0)
        {
            commands_being_executed = true;
            break;
        }

        // Nothing
        break;
    }

    CALL_OUT("");
    return commands_being_executed;
}



// ======================================================================== Bot



///////////////////////////////////////////////////////////////////////////////
// Register commands of this bot
void MainWindow::RegisterCommands()
{
    CALL_IN("");

    // Available commands
    QJsonObject set_my_commands;

    QJsonObject json_scope;
    json_scope["type"] = "all_private_chats";
    set_my_commands["scope"] = json_scope;

    QJsonArray json_all_commands;

    QJsonObject json_command;
    json_command["command"] = "contactsheets";
    json_command["description"] = tr("Creates contact sheets with samples "
        "of available sticker sets.");
    json_all_commands << json_command;

    json_command["command"] = "help";
    json_command["description"] = tr("Provides help on available commands");
    json_all_commands << json_command;

    json_command["command"] = "stickerset";
    json_command["description"] = tr("Downloaded a given sticker set");
    json_all_commands << json_command;

    json_command["command"] = "start";
    json_command["description"] =
        tr("Introduction to the capabilities of this bot");
    json_all_commands << json_command;

    set_my_commands["commands"] = json_all_commands;

    // Set them on the bot
    TelegramComms * tc = TelegramComms::Instance();
    tc -> SetMyCommands(set_my_commands);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Any update was received
void MainWindow::MessageReceived(const qint64 mcChatID, qint64 mcMessageID)
{
    CALL_IN(QString("mcChatID=%1, mcMessageID=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID)));

    // Abbreviation
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > message_info =
        tc -> GetMessageInfo(mcMessageID);

    // Check if we have this chat already
    if (!m_ChatIDToTextEdit.contains(mcChatID))
    {
        // Get details
        const QHash < QString, QString > chat_info =
            tc -> GetChatInfo(mcChatID);
        QString chat_name;
        if (chat_info["type"] == "supergroup")
        {
            // Group
            chat_name = chat_info["title"];
        }  else if (chat_info["type"] == "private")
        {
            // Personal chat
            chat_name = chat_info["username"];
        } else
        {
            // Unknown chat type
            const QString reason = tr("Chat ID %1: Unknown chat type \"%2\"")
                .arg(QString::number(mcChatID),
                     chat_info["type"]);
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return;
        }

        QTextEdit * text_edit = new QTextEdit;
        text_edit -> setReadOnly(true);
        m_ChatIDToTextEdit[mcChatID] = text_edit;
        m_ChatIDToLogText[mcChatID]
            << tr("<tr>"
                "<td colspan=\"3\" align=\"center\">"
                "<b>Opened chat \"%1\" on %2</b>"
                "</td>"
                "</tr>")
                .arg(chat_name,
                     message_info["date_time"]);
        m_TabWidget -> addTab(text_edit, chat_name);
    }

    if (!message_info.contains("date_time") ||
        !message_info.contains("from_id"))
    {
        // Can't determine who sent this message - weird
        const QString reason = tr("Message received does not contain "
            "required date_time and from_id information.");
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }
    const QString date_time = message_info["date_time"];
    const qint64 user_id = message_info["from_id"].toLongLong();
    const QHash < QString, QString > user_info =
        tc -> GetUserInfo(user_id);
    const QString user = user_info["first_name"];

    // Show message
    QString message;
    if (message_info.contains("text"))
    {
        // == Standard text message
        message = message_info["text"];
        message.replace("\n", "<br/>");
    } else if (message_info.contains("sticker_id"))
    {
        // == Forwarded sticker
        QString forward_user;
        if (message_info.contains("forward_from_id"))
        {
            // Original user info
            const qint64 forward_user_id =
                message_info["forward_from_id"].toLongLong();
            const QHash < QString, QString > forward_user_info =
                tc -> GetUserInfo(forward_user_id);
            forward_user = forward_user_info["first_name"];
        } else if (message_info.contains("forward_sender_name"))
        {
            forward_user = message_info["forward_sender_name"];
        } else
        {
            // Problem
            const QString reason =
                tr("Cannot determine sender of forwarded sticker message.");
            MessageLogger::Error(CALL_METHOD, reason);
            qDebug().noquote() << message_info;
            CALL_OUT("");
            return;
        }

        // Add line to log
        message = tr("Sticker forwarded in message from %1.")
            .arg(forward_user);
    } else if (message_info.contains("document_id"))
    {
        // == File uploaded
        const QString document_id = message_info["document_id"];
        const QHash < QString, QString > file_info =
            tc -> GetFileInfo(document_id);
        const QString file_name = file_info["file_name"];
        const qint64 file_size = file_info["file_size"].toLongLong();

        // Add line to log
        message = tr("Uploaded file \"%1\" (%2).")
            .arg(file_name,
                 StringHelper::ConvertFileSize(file_size));
    } else if (message_info.contains("new_chat_title"))
    {
        // Title of the chat was updated
        const QString new_chat_title = message_info["new_chat_title"];

        // Update tab title
        const qint64 chat_id = message_info["chat_id"].toLongLong();
        if (!m_ChatIDToTextEdit.contains(chat_id))
        {
            const QString reason =
                tr("Chat ID %1 did not appar to have its own chat window.")
                .arg(QString::number(chat_id));
            MessageLogger::Error(CALL_METHOD, reason);
            CALL_OUT(reason);
            return;
        }
        QTextEdit * text_edit = m_ChatIDToTextEdit[chat_id];
        const int index = m_TabWidget -> indexOf(text_edit);
        m_TabWidget -> setTabText(index, new_chat_title);

        // Add line to log
        message = tr("Set new chat title \"%1\".")
            .arg(new_chat_title);
    } else
    {
        // Some message we cannot show right now
        const QString reason = tr("Unhandled message format.");
        MessageLogger::Error(CALL_METHOD, reason);
        qDebug().noquote() << message_info;
    }

    if (!message.isEmpty())
    {
        QString html = QString("<tr>"
            "<td><b>[%1]</b></td>"
            "<td><b>%2</b></td>"
            "<td>%3</td>"
            "</tr>")
            .arg(date_time,
                 user,
                 message);
        m_ChatIDToLogText[mcChatID] << html;
    }


    // Update log
    QScrollBar * vertical =
        m_ChatIDToTextEdit[mcChatID] -> verticalScrollBar();
    int vertical_position = vertical -> value();
    const bool at_bottom = (vertical_position == vertical -> maximum());
    m_ChatIDToTextEdit[mcChatID] -> setHtml(
        "<table width=\"100%\">"
        + m_ChatIDToLogText[mcChatID].join("")
        + "</table>");
    if (at_bottom)
    {
        vertical_position = vertical -> maximum();
    }
    vertical -> setValue(vertical_position);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Command received
void MainWindow::CommandReceived(const qint64 mcUserID, const qint64 mcChatID,
    const qint64 mcMessageID, const QString & mcrCommand,
    const QString & mcrParameters)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, mcrCommand=%4, "
        "mcrParameters=%5")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrCommand),
             CALL_SHOW(mcrParameters)));

    // Check for imminent shutdown
    if (m_ShuttingDown)
    {
        TelegramComms * tc = TelegramComms::Instance();
        const QString message = tr("Bot is shutting down - command ignored.");
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }

    // Check which commands have been used
    while (true)
    {
        if (mcrCommand == "contactsheets")
        {
            Command_ContactSheets(mcUserID, mcChatID, mcMessageID,
                mcrParameters);
            break;
        }
        if (mcrCommand == "help")
        {
            Command_Help(mcUserID, mcChatID, mcMessageID, mcrParameters);
            break;
        }
        if (mcrCommand == "start")
        {
            Command_Start(mcUserID, mcChatID, mcMessageID, mcrParameters);
            break;
        }
        if (mcrCommand == "stickerset")
        {
            Command_StickerSet(mcUserID, mcChatID, mcMessageID, mcrParameters);
            break;
        }

        // Unknown command
        Command_UnknownCommand(mcUserID, mcChatID, mcMessageID, mcrCommand);
        break;
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Seaparate message for command
void MainWindow::CommandSeparateMessageReceived(const qint64 mcUserID,
    const qint64 mcForwardedMessageID)
{
    CALL_IN(QString("mcUserID=%1, mcForwardedMessageID=%2")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcForwardedMessageID)));

    // Check if we're actually waiting for this message
    if (!m_Separate_UserIDToExpectedMessageID.contains(mcUserID) ||
        m_Separate_UserIDToExpectedMessageID[mcUserID]
            != mcForwardedMessageID)
    {
        // Not interested
        CALL_OUT("");
        return;
    }

    // Perform command
    const QString command = m_Separate_UserIDToCommand[mcUserID];
    const qint64 chat_id = m_Separate_UserIDToChatID[mcUserID];
    m_Separate_UserIDToCommand.remove(mcUserID);
    m_Separate_UserIDToExpectedMessageID.remove(mcUserID);
    m_Separate_UserIDToChatID.remove(mcUserID);
    if (command == "stickerset")
    {
        SeparateCommand_StickerSet(mcUserID, chat_id, mcForwardedMessageID);
    } else
    {
        // Unhandled command
        const QString reason =
            tr("Unhandled command \"%1\" for separate message.")
                .arg(command);
        MessageLogger::Error(CALL_METHOD, reason);
        CALL_OUT(reason);
        return;
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Command: /help
void MainWindow::Command_Help(const qint64 mcUserID, const qint64 mcChatID,
    const qint64 mcMessageID, const QString & mcrParameters)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, "
        "mcrParameters=%4")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrParameters)));

    // Get details
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > user_info = tc -> GetUserInfo(mcUserID);

    // Check if general help is requested, or just for a particular subject
    QString message;
    if (mcrParameters.isEmpty())
    {
        message = tr("Hi, %1. I’m ShimaronBot. I understand the following "
            "commands:\n\n")
            .arg(user_info["first_name"]);
        message += tr("/contactsheets - Creates contact sheets with samples "
            "of available sticker sets.\n");
        message += tr("/help - Provides help on available commands.\n");
        message += tr("/start - Introduction to the capabilities of this "
            "bot.\n");
        message += tr("/stickerset - Downloaded a given sticker set.");
    } else if (mcrParameters == "help")
    {
        message = tr("Command:\n"
            "/help [command]\n"
            "Purpose:\n"
            "- To provide additonal help on the command [command].\n"
            "Parameters:\n"
            "- [command] is a valid command for this bot.\n"
            "Result:\n"
            "- Help on the command.");
    } else if (mcrParameters == "stickerset")
    {
        message = tr("Command:\n"
            "/stickerset\n"
            "Purpose:\n"
            "- To download an entire sticker set to your computer.\n"
            "Parameters:\n"
            "There are thre ways to call this command:\n"
            "(1) with a [URL] you can obtaint to share sticker sets, e.g. "
            "https://t.me/addstickers/something (you get this URL when "
            "clicking on any sticker and the using the \"share\" button at "
            "the top right corner.\n"
            "(2) with a [set name] that is just the name of the set\n"
            "(3) by forwarding a sticker message to the bot with the text "
            "/stickerset and no parameters or other text in the message.\n"
            "Result:\n"
            "- a ZIP file with all stickers in the set.");
    } else if (mcrParameters == "contactsheets")
    {
        message = tr("Command:\n"
            "/contactsheets\n"
            "Purpose:\n"
            "(1) To download an overview of all available sticker sets.\n"
            "(2) To download an overview of a particular sticker set.\n"
            "Parameters:\n"
            "(1) [width]x[height], the resolution of the target image.\n"
            "Default is 1920x1080.\n"
            "(2) [set_name], the sticker set name.\n"
            "Result:\n"
            "- One or several images for download.");
    } else if (mcrParameters == "start")
    {
        message = tr("Command:\n"
            "/start\n"
            "Purpose:\n"
            "- To introduce you to the features of this bot.\n"
            "Parameters:\n"
            "- None.\n"
            "Result:\n"
            "- Hopefully, a happy user.");
    } else
    {
        message = tr("Sorry, %1, I cannot provide you with any help on "
            "\"%2\".")
            .arg(user_info["first_name"],
                 mcrParameters);
    }

    // Send message
    tc -> SendMessage(mcChatID, message);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Command: /stickerset
void MainWindow::Command_StickerSet(const qint64 mcUserID,
    const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrParameters)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, "
        "mcrParameters=%4")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrParameters)));

    // Abbreviation
    TelegramHelper * th = TelegramHelper::Instance();

    // Obtain sticker set name
    QString sticker_set_name;
    while (true)
    {
        // ==== /stickerset in a forwarded message containing a sticker
        if (mcrParameters.isEmpty())
        {
            // Possibly a forwarded message (seperately)
            m_Separate_UserIDToCommand[mcUserID] = "stickerset";
            m_Separate_UserIDToExpectedMessageID[mcUserID] = mcMessageID + 1;
            m_Separate_UserIDToChatID[mcUserID] = mcChatID;
            CALL_OUT("");
            return;
        }

        // ==== /stickerset https://t.me/addstickers/name
        static const QRegularExpression format_by_link(
            "^https://t.me/addstickers/(.+)$");
        const QRegularExpressionMatch match_by_link =
            format_by_link.match(mcrParameters);
        if (match_by_link.hasMatch())
        {
            sticker_set_name = match_by_link.captured(1);
            break;
        }

        // === /stickerset name
        static const QRegularExpression format_by_name("^([a-zA-Z0-9_]+)$");
        const QRegularExpressionMatch match_by_name =
            format_by_name.match(mcrParameters);
        if (match_by_name.hasMatch())
        {
            sticker_set_name = match_by_name.captured(1);
            break;
        }

        // Error
        const QString message =
            tr("Could not identify the sticker set name from \"%1\".")
                .arg(mcrParameters);
        th -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }

    // Get sticker set
    DownloadNewStickerSet(mcChatID, sticker_set_name);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
void MainWindow::SeparateCommand_StickerSet(const qint64 mcUserID,
    const qint64 mcChatID, const qint64 mcForwardedMessageID)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcForwardedMessageID=%3")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcForwardedMessageID)));

    // Get message info
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > message_info =
        tc -> GetMessageInfo(mcForwardedMessageID);

    // Get sticker being forwarded
    if (!message_info.contains("sticker_id"))
    {
        QString message =
            tr("Could not find a sticker in the forwarded message.");
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }
    const QString sticker_id = message_info["sticker_id"];
    const QHash < QString, QString > sticker_info =
        tc -> GetFileInfo(sticker_id);

    // Get sticker set
    // (yes, stickers can be sent outside of a set)
    if (!sticker_info.contains("set_name"))
    {
        QString message =
            tr("Could not find a sticker set for the forwarded sticker.");
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }
    const QString sticker_set_name = sticker_info["set_name"];
    DownloadNewStickerSet(mcChatID, sticker_set_name);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Download a sticker set if we haven't done so yet
void MainWindow::DownloadNewStickerSet(const qint64 mcChatID,
    const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcChatID=%1, mcrStickerSetName=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrStickerSetName)));

    // Check if sticker set is already being downloaded
    TelegramHelper * th = TelegramHelper::Instance();
    if (th -> IsStickerSetBeingDownloaded(mcrStickerSetName))
    {
        TelegramComms * tc = TelegramComms::Instance();
        const QString message = tr("Sticker set %1 is already in the process "
            "of being downloaded.")
            .arg(mcrStickerSetName);
        tc -> SendMessage(mcChatID, message);
    } else
    {
        m_StickerSetNameToChatIDs[mcrStickerSetName] += mcChatID;
        th -> DownloadStickerSet(mcrStickerSetName);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Sticker set received
void MainWindow::StickerSetReceived(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Get sticker set infomation
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > set_info =
        tc -> GetStickerSetInfo(mcrStickerSetName);
    QString set_title = set_info["title"];
    set_title.replace("\n", " ");

    // Check if upload was suppressed
    if (m_SuppressStickerSetUpload.contains(mcrStickerSetName))
    {
        const QStringList set_stickers =
            tc -> GetStickerSetFileIDs(mcrStickerSetName);
        const QString message = tr("Upload of ZIP file suppressed for "
            "sticker set %1 (%2) with %3 stickers.")
            .arg(mcrStickerSetName,
                 set_title,
                 QString::number(set_stickers.size()));
        for (auto chat_iterator =
                m_StickerSetNameToChatIDs[mcrStickerSetName].constBegin();
            chat_iterator !=
                m_StickerSetNameToChatIDs[mcrStickerSetName].constEnd();
            chat_iterator++)
        {
            const qint64 chat_id = *chat_iterator;
            tc -> SendMessage(chat_id, message);
        }
    } else
    {
        // Filename
        TelegramHelper * th = TelegramHelper::Instance();
        const QString filename =
            th -> GetStickerSetZIPFilename(mcrStickerSetName);
        for (auto chat_iterator =
                m_StickerSetNameToChatIDs[mcrStickerSetName].constBegin();
            chat_iterator !=
                m_StickerSetNameToChatIDs[mcrStickerSetName].constEnd();
            chat_iterator++)
        {
            const qint64 chat_id = *chat_iterator;
            if (m_StickerSetNameHasBeenSentTOChatIDs[mcrStickerSetName]
                .contains(chat_id))
            {
                // Sticker set has already been uploaded to this chat
                QString message = tr("Sticker set %1 (%2) has already been "
                    "uploaded to this chat.")
                    .arg(mcrStickerSetName,
                         set_title);
                tc -> SendMessage(chat_id, message);
            } else
            {
                tc -> UploadFile(chat_id, filename);
                m_StickerSetNameHasBeenSentTOChatIDs[mcrStickerSetName] +=
                    chat_id;
            }
        }
    }

    // No need to keep this around
    m_StickerSetNameToChatIDs.remove(mcrStickerSetName);

    // Check if there is current work going on
    if (m_ShuttingDown &&
        !CommandsBeingExecuted())
    {
        // We can quit after waiting for the upload to finish
        QTimer::singleShot(5000, this, &MainWindow::close);
    }

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Sticker set info failed
void MainWindow::StickerSetInfoFailed(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    TelegramComms * tc = TelegramComms::Instance();
    const QString message = tr("Sticker set \"%1\" does not exist.")
        .arg(mcrStickerSetName);
    for (auto chat_iterator =
            m_StickerSetNameToChatIDs[mcrStickerSetName].constBegin();
        chat_iterator !=
            m_StickerSetNameToChatIDs[mcrStickerSetName].constEnd();
        chat_iterator++)
    {
        const qint64 chat_id = *chat_iterator;
        tc -> SendMessage(chat_id, message);
    }

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Command /contactsheets
void MainWindow::Command_ContactSheets(const qint64 mcUserID,
    const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrParameters)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, "
        "mcrParameters=%4")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrParameters)));

    // Abbreviation
    TelegramComms * tc = TelegramComms::Instance();

    // We'll need this
    static const QRegularExpression format_resolution("^([0-9]+)x([0-9]+)$");
    static const QRegularExpression format_stickerset("^([a-zA-Z0-9_]+)$");

    // Determine target resolution
    while (true)
    {
        // == All sets, default resolution
        if (mcrParameters.isEmpty())
        {
            const int sheet_width = 1920;
            const int sheet_height = 1080;
            Command_ContactSheets_AllSets(mcChatID, sheet_width, sheet_height);
            break;
        }

        // == All sets, custom resolution
        const QRegularExpressionMatch match_resolution =
            format_resolution.match(mcrParameters);
        if (match_resolution.hasMatch())
        {
            const int sheet_width = match_resolution.captured(1).toInt();
            const int sheet_height = match_resolution.captured(2).toInt();
            Command_ContactSheets_AllSets(mcChatID, sheet_width, sheet_height);
            break;
        }

        // == Single set
        const QRegularExpressionMatch match_stickerset =
            format_stickerset.match(mcrParameters);
        if (match_stickerset.hasMatch())
        {
            const QString set_name = match_stickerset.captured(1);
            Command_ContactSheets_SingleSet(mcChatID, set_name);
            break;
        }

        // == Error
        const QString message = tr("Parameter \"%1\" is neither a resolution "
            "not a sticker set name")
            .arg(mcrParameters);
        tc -> SendMessage(mcChatID, message);
        break;
    }

    CALL_OUT("");
    return;
}



///////////////////////////////////////////////////////////////////////////////
// Contact sheet: all sets
void MainWindow::Command_ContactSheets_AllSets(const qint64 mcChatID,
    const int mcWidth, const int mcHeight)
{
    CALL_IN(QString("mcChatID=%1, mcWidth=%2, mcHeight=%3")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcWidth),
             CALL_SHOW(mcHeight)));

    // Dimensions
    const int dim_sticker = 200;
    const int dim_frame_n = 20;
    const int dim_frame_e = 20;
    const int dim_frame_s = 20;
    const int dim_frame_w = 20;
    const int dim_vertical_spacing = 20;
    const int dim_horizontal_spacing = 20;
    const int dim_text_height = 20;

    const int min_sticker_x = 4;
    const int min_sticker_y = 2;

    const int min_width = dim_frame_w
        + min_sticker_x * dim_sticker
        + (min_sticker_x - 1) * dim_horizontal_spacing
        + dim_frame_e;
    const int min_height = dim_frame_n
        + min_sticker_y * (dim_sticker + dim_text_height)
        + (min_sticker_y - 1) * dim_vertical_spacing
        + dim_frame_s;

    // Abbreviation
    TelegramComms * tc = TelegramComms::Instance();

    // Acceptable resolution?
    if (mcWidth < min_width ||
        mcHeight < min_height)
    {
        const QString message = tr("Minimum resolution requirements are "
            "not met. Width must be at least %1, height at least %2.")
            .arg(QString::number(min_width),
                 QString::number(min_height));
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }
    if (mcWidth * mcHeight > 20000000)
    {
        const QString message = tr("Maximum resolution is limited to 20MP.");
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }

    // Number of stickers on each sheet
    int sheet_columns =
        (mcWidth - dim_frame_e - dim_frame_w + dim_horizontal_spacing) /
        (dim_sticker + dim_horizontal_spacing);
    int sheet_rows =
        (mcHeight - dim_frame_n - dim_frame_s + dim_vertical_spacing) /
        (dim_sticker + dim_text_height + dim_vertical_spacing);

    QString message = tr("Fitting %1x%2 stickers on the contact sheet.")
        .arg(QString::number(sheet_columns),
             QString::number(sheet_rows));
    tc -> SendMessage(mcChatID, message);

    // Calculate actual values
    const int slack_x = mcWidth -
        (dim_frame_w
        + sheet_columns * dim_sticker
        + (sheet_columns - 1) * dim_horizontal_spacing
        + dim_frame_e);
    const int slack_y = mcHeight -
        (dim_frame_n
        + sheet_rows * (dim_sticker + dim_text_height)
        + (sheet_rows - 1) * dim_vertical_spacing
        + dim_frame_s);
    const int act_frame_n = 20 + slack_y/2;
    const int act_frame_e = 20 + slack_x/2;

    // Loop all available sticker sets
    const QStringList all_set_names = tc -> GetAllStickerSetNames();
    int row = 0;
    int column = 0;
    int sheet_count = 1;
    int num_stickers = 0;
    int num_animated = 0;
    QPixmap sheet = QPixmap(mcWidth, mcHeight);
    QPainter * painter = new QPainter(&sheet);
    painter -> fillRect(0, 0, mcWidth, mcHeight, Qt::white);
    for (const QString & set_name : all_set_names)
    {
        if (!tc -> DoesStickerSetInfoExist(set_name))
        {
            continue;
        }
        const QStringList sticker_ids = tc -> GetStickerSetFileIDs(set_name);
        const QString first_file_id = sticker_ids.first();
        if (!tc -> HasFileBeenDownloaded(first_file_id))
        {
            continue;
        }

        // Read picture; this will fail if sticker is animated because it
        // then is in a file format we cannot read;
        const QByteArray sticker_data = tc -> GetFile(first_file_id);
        QImage image = QImage::fromData(sticker_data);
        if (image.isNull())
        {
            num_animated++;
            continue;
        }
        num_stickers++;

        image = image.scaled(dim_sticker, dim_sticker,
            Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Render
        const int x = act_frame_e +
            column * (dim_sticker + dim_horizontal_spacing)
            + (dim_sticker - image.width())/2;
        const int y = act_frame_n +
            row * (dim_sticker + dim_text_height + dim_vertical_spacing)
            + (dim_sticker - image.height())/2;
        painter -> drawImage(x, y, image);

        // Set name
        painter -> drawText(x, y + dim_sticker + 2, dim_sticker, 15,
            Qt::AlignCenter, set_name);

        column++;
        if (column == sheet_columns)
        {
            column = 0;
            row++;
            if (row == sheet_rows)
            {
                row = 0;
                const QString filename = USER_FILES + QString("Sheet %1.png")
                    .arg(QString::number(sheet_count));
                sheet.save(filename, "PNG");
                tc -> UploadFile(mcChatID, filename);

                // New sheet
                delete painter;
                sheet = QPixmap(mcWidth, mcHeight);
                painter = new QPainter(&sheet);
                painter -> fillRect(0, 0, mcWidth, mcHeight, Qt::white);
                sheet_count++;
            }
        }
    }

    // Save last contact sheet
    if (row !=0 ||
        column !=0 )
    {
        const QString filename = USER_FILES + QString("Sheet %1.png")
            .arg(QString::number(sheet_count));
        sheet.save(filename, "PNG");
        tc -> UploadFile(mcChatID, filename);
        delete painter;
    }

    message = tr("Created %1 contact %2 with a total of %3 sticker %4.")
        .arg(QString::number(sheet_count),
             sheet_count == 1 ? tr("sheet") : tr("sheets"),
             QString::number(num_stickers),
             num_stickers == 1 ? tr("set") : tr("sets"));
    if (num_animated > 0)
    {
        message += tr(" %1 sets with animated stickers were ignored.")
            .arg(QString::number(num_animated));
    }
    tc -> SendMessage(mcChatID, message);

    // Check if there is current work going on
    if (m_ShuttingDown &&
        !CommandsBeingExecuted())
    {
        // We can quit after allowing the upload to complete
        QTimer::singleShot(5000, this, &MainWindow::close);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Contact sheet: single set
void MainWindow::Command_ContactSheets_SingleSet(const qint64 mcChatID,
    const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcChatID=%1, mcrStickerSetName=%2")
        .arg(CALL_SHOW(mcChatID),
             CALL_SHOW(mcrStickerSetName)));

    // Check if the set exists
    TelegramComms * tc = TelegramComms::Instance();
    if (!tc -> DoesStickerSetInfoExist(mcrStickerSetName))
    {
        const QString message = tr("I don't know sticker set %1.")
            .arg(mcrStickerSetName);
        tc -> SendMessage(mcChatID, message);
        CALL_OUT("");
        return;
    }
    const QHash < QString, QString > set_info =
        tc -> GetStickerSetInfo(mcrStickerSetName);
    QString set_title = set_info["title"];
    set_title.replace("\n", " ");

    // Get sticker file IDs
    const QStringList all_file_ids =
        tc -> GetStickerSetFileIDs(mcrStickerSetName);
    for (const QString & file_id : all_file_ids)
    {
        const QHash < QString, QString > file_info =
            tc -> GetFileInfo(file_id);
        if (file_info["is_animated"] == "true")
        {
            const QString message = tr("Sticker set %1 contains animated "
                "stickers that I cannot handle.")
                .arg(mcrStickerSetName);
            tc -> SendMessage(mcChatID, message);
            CALL_OUT("");
            return;
        }
        if (!tc -> HasFileBeenDownloaded(file_id))
        {
            const QString message =
                tr("Not all stickers for sticker set %1 has been downloaded.")
                .arg(mcrStickerSetName);
            tc -> SendMessage(mcChatID, message);
            CALL_OUT("");
            return;
        }
    }

    // Default resolution
    const int sheet_width = 1920;
    const int sheet_height = 1080;

    // Dimensions
    const int dim_sticker = 200;
    const int dim_title_height = 100;
    const int dim_frame_n = 20;
    const int dim_frame_e = 20;
    const int dim_frame_s = 20;
    const int dim_frame_w = 20;
    const int dim_vertical_spacing = 20;
    const int dim_horizontal_spacing = 20;

    // Number of stickers on each sheet
    int sheet_columns =
        (sheet_width - dim_frame_e - dim_frame_w + dim_horizontal_spacing) /
        (dim_sticker + dim_horizontal_spacing);
    int sheet_rows =
        (sheet_height - dim_frame_n - dim_title_height - dim_frame_s +
            dim_vertical_spacing) /
        (dim_sticker + dim_vertical_spacing);

    QString message = tr("Fitting %1x%2 stickers on the contact sheet.")
        .arg(QString::number(sheet_columns),
             QString::number(sheet_rows));
    tc -> SendMessage(mcChatID, message);

    // Calculate actual values
    const int slack_x = sheet_width -
        (dim_frame_w
        + sheet_columns * dim_sticker
        + (sheet_columns - 1) * dim_horizontal_spacing
        + dim_frame_e);
    const int slack_y = sheet_height -
        (dim_frame_n
        + dim_title_height
        + sheet_rows * dim_sticker
        + (sheet_rows - 1) * dim_vertical_spacing
        + dim_frame_s);
    const int act_frame_n = dim_frame_n + slack_y/2;
    const int act_frame_e = dim_frame_e + slack_x/2;

    // Title font
    QFont title_font("Georgia", 70);

    // Loop all stickers in this set
    int row = 0;
    int column = 0;
    int sheet_count = 1;
    QPixmap sheet;
    QPainter * painter = nullptr;
    for (const QString & file_id : all_file_ids)
    {
        // Title
        if (row == 0 && column == 0)
        {
            // Generate new sheet
            if (painter)
            {
                delete painter;
            }
            sheet = QPixmap(sheet_width, sheet_height);
            painter = new QPainter(&sheet);
            painter -> fillRect(0, 0, sheet_width, sheet_height,
                Qt::white);

            // Set name
            painter -> setFont(title_font);
            painter -> drawText(0, act_frame_n, sheet_width, dim_title_height,
                Qt::AlignCenter, set_title);
        }

        const QByteArray sticker_data = tc -> GetFile(file_id);
        QImage image = QImage::fromData(sticker_data);
        image = image.scaled(dim_sticker, dim_sticker,
            Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Render
        const int x = act_frame_e +
            column * (dim_sticker + dim_horizontal_spacing)
            + (dim_sticker - image.width())/2;
        const int y = act_frame_n + dim_title_height +
            row * (dim_sticker + dim_vertical_spacing)
            + (dim_sticker - image.height())/2;
        painter -> drawImage(x, y, image);

        column++;
        if (column == sheet_columns)
        {
            column = 0;
            row++;
            if (row == sheet_rows)
            {
                row = 0;
                const QString filename = USER_FILES + QString("Sheet %1.png")
                    .arg(QString::number(sheet_count));
                sheet.save(filename, "PNG");
                tc -> UploadFile(mcChatID, filename);
                sheet_count++;
            }
        }
    }

    // Save last contact sheet
    if (row !=0 ||
        column !=0 )
    {
        const QString filename = USER_FILES + QString("Sheet %1.png")
            .arg(QString::number(sheet_count));
        sheet.save(filename, "PNG");
        tc -> UploadFile(mcChatID, filename);
        delete painter;
    }

    message = tr("Created %1 contact %2 for set \"%3\" with a total "
        "of %4 %5.")
        .arg(QString::number(sheet_count),
             sheet_count == 1 ? tr("sheet") : tr("sheets"),
             mcrStickerSetName,
             QString::number(all_file_ids.size()),
             all_file_ids.size() == 1 ? tr("sticker") : tr("stickers"));
    tc -> SendMessage(mcChatID, message);

    // Check if there is current work going on
    if (m_ShuttingDown &&
        !CommandsBeingExecuted())
    {
        // We can quit after allowing the upload to complete
        QTimer::singleShot(5000, this, &MainWindow::close);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Command /start
void MainWindow::Command_Start(const qint64 mcUserID,
    const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrParameters)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, "
        "mcrParameters=%4")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrParameters)));

    // !!!

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Unknown command
void MainWindow::Command_UnknownCommand(const qint64 mcUserID,
    const qint64 mcChatID, const qint64 mcMessageID,
    const QString & mcrCommand)
{
    CALL_IN(QString("mcUserID=%1, mcChatID=%2, mcMessageID=%3, "
        "mcrCommand=%4")
        .arg(CALL_SHOW(mcUserID),
             CALL_SHOW(mcChatID),
             CALL_SHOW(mcMessageID),
             CALL_SHOW(mcrCommand)));

    TelegramHelper * th = TelegramHelper::Instance();
    const QString message = tr("Unknown command /%1.\n"
        "Use /help to get a list of available commands.")
        .arg(mcrCommand);
    th -> SendMessage(mcChatID, message);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Sticker set info received
void MainWindow::StickerSetInfoReceived(const QString & mcrStickerSetName)
{
    CALL_IN(QString("mcrStickerSetName=%1")
        .arg(CALL_SHOW(mcrStickerSetName)));

    // Check if we are downloading this sticker set
    if (!m_StickerSetNameToChatIDs.contains(mcrStickerSetName))
    {
        // Nothing to do.
        CALL_OUT("");
        return;
    }

    // Get info
    TelegramComms * tc = TelegramComms::Instance();
    const QHash < QString, QString > info =
        tc -> GetStickerSetInfo(mcrStickerSetName);
    QString title = info["title"];
    title.replace("\n", " ");

    // Get files
    const QStringList sticker_ids =
        tc -> GetStickerSetFileIDs(mcrStickerSetName);

    const QString message = tr("Sticker set %1 has %2 stickers.")
        .arg(title,
             QString::number(sticker_ids.size()));
    for (auto chat_iterator =
            m_StickerSetNameToChatIDs[mcrStickerSetName].constBegin();
        chat_iterator !=
            m_StickerSetNameToChatIDs[mcrStickerSetName].constEnd();
        chat_iterator++)
    {
        const qint64 chat_id = *chat_iterator;
        tc -> SendMessage(chat_id, message);
    }

    CALL_OUT("");
}

