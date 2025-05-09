// TelegramComms.h
// Class definition

#ifndef TELEGRAMCOMMS_H
#define TELEGRAMCOMMS_H

// Qt includes
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>



// Class definition
class TelegramComms
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
private:
    // Constructor
    TelegramComms();

public:
    // Destructor
    virtual ~TelegramComms();

    // Instanciator
    static TelegramComms * Instance();

private:
    // Instance
    static TelegramComms * m_Instance;



    // =============================================================== Database
public:
    // Set database name
    void SetDatabaseFile(const QString & mcrFilename);
private:
    QString m_DatabaseFilename;

public:
    bool OpenDatabase();
private:
    bool m_DatabaseConnected;

    // Create database
    bool CreateDatabase();
    bool CreateDatabase_Table(const QString & mcrTableName,
        const QString & mcrIDType = "bigint");
    bool CreateDatabase_Table_StickerSet(const QString & mcrTableName);

    // Read database
    bool ReadDatabase();
    bool ReadDatabase_Table(const QString & mcrTableName,
        QHash < qint64, QHash < QString, QString > > & mrInfoData);
    bool ReadDatabase_Table(const QString & mcrTableName,
        QHash < QString, QHash < QString, QString > > & mrInfoData);
    bool ReadDatabase_Table_StickerSet(const QString & mcrTableName);

    // Save info data
    bool SaveInfoData(const QString & mcrTableName,
        const QHash < QString, QString > & mcrInfoData,
        const QString & mcrIDType = "bigint");
    bool SaveInfoData_StickerSet(const QString & mcrTableName,
        const QString & mcrStickerSetID);

public:
    // Update database
    void UpdateDatabase();



    // ================================================================== Setup
public:
    // Bot name
    bool SetBotName(const QString & mcrBotName);
private:
    QString m_BotName;

public:
    // Bot token
    bool SetToken(const QString & mcrToken);
private:
    QString m_Token;

public:
    // Set offset
    void SetOffset(const qint64 mcOffset);
private:
    bool m_OffsetSet;
    qint64 m_Offset;

public:
    // Start/stop bot
    void StartBot();
    void StopBot();
private:
    bool m_IsRunning;

public:
    QString GetStartDateTime() const;
    QString GetUptime() const;
private:
    QDateTime m_StartDateTime;



    // ================================================ Reading from the Server
private slots:
    // Periodically checking for updates
    void Periodic_WatchForUpdates();

private slots:
    // Check for updates
    void CheckForUpdates();
private slots:
    // Handle response
    bool HandleResponse(QNetworkReply * mpResponse);
private:
    QNetworkAccessManager * m_NetworkAccessManager;

private:
    // Original server response
    bool Parse_Response(const QJsonObject & mcrResponse);

private:
    // Updates
    bool Parse_UpdateArray(const QJsonArray & mcrUpdates);
    QHash < QString, QString > Parse_Update(const QJsonObject & mcrUpdate);
    QHash < qint64, QHash < QString, QString > > m_UpdateIDToInfo;
public:
    bool DoesUpdateInfoExist(const qint64 mcUpdateID) const;
    QHash < QString, QString > GetUpdateInfo(const qint64 mcUpdateID);
signals:
    void UpdateReceived(const qint64 mcrChatID, const qint64 mcrUpdateID);

private:
    // Message
    QHash < QString, QString > Parse_Message(const QJsonObject & mcrMessage);
    QHash < qint64, QHash < QString, QString > > m_MessageIDToInfo;
public:
    bool DoesMessageInfoExist(const qint64 mcMessageID) const;
    QHash < QString, QString > GetMessageInfo(const qint64 mcMessageID);
signals:
    void MessageReceived(const qint64 mcrChatID, const qint64 mcrMessageID);

private:
    // User
    QHash < QString, QString > Parse_User(const QJsonObject & mcrUser);
    QHash < qint64, QHash < QString, QString > > m_UserIDToInfo;
public:
    bool DoesUserInfoExist(const qint64 mcUserID) const;
    QHash < QString, QString > GetUserInfo(const qint64 mcUserID);

private:
    // Chat
    QHash < QString, QString > Parse_Chat(const QJsonObject & mcrChat);
    QHash < qint64, QHash < QString, QString > > m_ChatIDToInfo;
public:
    bool DoesChatInfoExist(const qint64 mcChatID) const;
    QHash < QString, QString > GetChatInfo(const qint64 mcChatID);

private:
    // MyChatMember
    QHash < QString, QString > Parse_MyChatMember(
        const QJsonObject & mcrMyChatMember);
    QHash < QString, QString > Parse_MyChatMember_OldChatMember(
        const QJsonObject & mcrOldChatMember);
    QHash < QString, QString > Parse_MyChatMember_NewChatMember(
        const QJsonObject & mcrNewChatMember);
    QHash < qint64, QHash < QString, QString > > m_MyChatMemberIDToInfo;
public:
    bool DoesMyChatMemberInfoExist(const qint64 mcMyChatMemberID) const;
    QHash < QString, QString > GetMyChatMemberInfo(
        const qint64 mcMyChatMemberID);

private:
    // File
    QHash < QString, QString > Parse_File(const QJsonObject & mcrFile);
    QHash < QString, QHash < QString, QString > > m_FileIDToInfo;
public:
    bool DoesFileInfoExist(const QString & mcrFileID) const;
    QHash < QString, QString > GetFileInfo(const QString & mcrFileID);

private:
    // Button List
    QHash < QString, QString > Parse_ButtonList(
        const QJsonObject & mcrButtonList);
    qint64 m_NextButtonListID;
    QHash < qint64, QHash < QString, QString > > m_ButtonListIDToInfo;
public:
    bool DoesButtonListInfoExist(const qint64 mcButtonListID) const;
    QHash < QString, QString > GetButtonList(const qint64 mcButtonListID);

private:
    // Button
    QHash < QString, QString > Parse_Button(const QJsonObject & mcrButton);
    qint64 m_NextButtonID;
    QHash < qint64, QHash < QString, QString > > m_ButtonIDToInfo;
public:
    bool DoesButtonInfoExist(const qint64 mcButtonID) const;
    QHash < QString, QString > GetButtonInfo(const qint64 mcButtonID);



    // =========================================================== Sticker Sets
private:
    QHash < QString, QString > Parse_StickerSet(
        const QJsonObject & mcrStickerSet);
    QHash < QString, QHash < QString, QString > > m_StickerSetNameToInfo;
    QHash < QString, QStringList > m_StickerSetNameToFileIDs;
public:
    QStringList GetAllStickerSetNames() const;
    bool DoesStickerSetInfoExist(const QString & mcrStickerSetName) const;
    QHash < QString, QString > GetStickerSetInfo(
        const QString & mcrStickerSetName) const;
    QStringList GetStickerSetFileIDs(const QString & mcrStickerSetName) const;

    void DownloadStickerSetInfo(const QString & mcrStickerSetName);
private slots:
    void Periodic_DownloadStickerSetInfo();
private:
    QStringList m_StickerSetInfo_DownloadQueue;
    QString m_StickerSetInfoBeingDownloaded;
    void Error_StickerSetInvalid();
signals:
    void StickerSetInfoFailed(const QString & mcrStickerSetName);
    void StickerSetInfoReceived(const QString & mcrStickerSetName);



private:
    // =========================================================== Channel Post
    QHash < QString, QString > Parse_ChannelPost(
        const QJsonObject & mcrChannelPost);
    QHash < qint64, QHash < QString, QString > > m_MessageIDToChannelPostInfo;
public:
    bool DoesChannelPostExist(const qint64 mcMessageID) const;
    QHash < QString, QString > GetChannelPostInfo(const qint64 mcMessgeID);
signals:
    void ChannelPostReceived(const qint64 mcrChatID,
        const qint64 mcrMessageID);



    // ========================================================= File Downloads
public:
    // Download file
    void DownloadFile(const QString & mcrFileID);

    // Download worklist size
    int GetDownloadWorkListSize() const;
private slots:
    void Periodic_DownloadFiles();
private:
    QStringList m_DownloadQueue;

    bool Download_FilePath(const QString & mcrFileID,
        const QString & mcrFilePath);
    bool SaveFile(const QString & mcrFilePath, const QByteArray & mcrData);
public:
    bool HasFileBeenDownloaded(const QString & mcrFileID);
    QByteArray GetFile(const QString & mcrFileID);
signals:
    void FileDownloaded(const QString & mcrFileID);

private:
    QHash < QString, QString > m_FilePathToFileID;



    // ================================================== Sending to the Server
public:
    // Setting available commands
    void SetMyCommands(const QJsonObject & mcrAvailableCommands);

    // Sending messages
    void SendMessage(const qint64 mcChatID, const QString & mcrMessage);

    // Sending messages to all active chats
    void SendBroadcastMessage(const QString & mcrMessage);
private:
    QSet < qint64 > m_ActiveChats;

public:
    // Sending reply
    void SendReply(const qint64 mcChatID, const qint64 mcMessageID,
        const QString & mcrMessage);

    // Upload a file to a chat
    bool UploadFile(const qint64 mcChatID, const QString & mcrFilename);



    // ================================================================== Debug
public:
    // Dump everything
    void Dump() const;
};

#endif
