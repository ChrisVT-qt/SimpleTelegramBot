// TelegramHelper.h
// Class definition

#ifndef TELEGRAMHELPER_H
#define TELEGRAMHELPER_H

// Qt includes
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>



// Class definition
class TelegramHelper
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
private:
    // Constructor
    TelegramHelper();

public:
    // Destructor
    virtual ~TelegramHelper();

    // Instanciator
    static TelegramHelper * Instance();

private:
    // Instance
    static TelegramHelper * m_Instance;


    // =============================================================== Messages
private slots:
    // A message has been received from the server
    void Server_MessageReceieved(const qint64 mcChatID,
        const qint64 mcMessageID);
signals:
    void MessageReceived(const qint64 mcChatID, const qint64 mcMessageID);
private:

public:
    // Send message
    bool SendMessage(const qint64 mcChatID, const QString & mcrMessage);
    bool SendReply(const qint64 mcChatID, const qint64 mcMessageID,
        const QString & mcrMessage);

signals:
    // A command has been received
    void CommandReceived(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcCommand,
        const QString & mcrParameters);
    void CommandSeparateMessageReceived(const qint64 mcUserID,
        const qint64 mcSeparateMessageID);
private:
    // Separate forwarded messages that belong to a previous command
    QHash < qint64, qint64 > m_UserIDToSeparateMessageID;



    // ================================================================== Files
private slots:
    void Server_FileDownloaded(const QString & mcrFileID);
signals:
    void FileDownloaded(const QString & mcrFileID);



    // =============================================================== Stickers
public:
    // Check if we're already downloading a particular sticker set
    bool IsStickerSetBeingDownloaded(const QString & mcrStickerSetName) const;

    // Download an entire sticker set
    void DownloadStickerSet(const QString & mcrStickerSetName,
        const bool mcForce = false);
private:
    QSet < QString > m_StickerSetIsDownloading;
private slots:
    void Server_StickerSetInfoReceived(const QString & mcrStickerSetName);
signals:
    void StickerSetInfoReceived(const QString & mcrStickerSetName);
private:
    void DownloadStickerFiles(const QString & mcrStickerSetName);
    void StickerFileReceived(const QString & mcrFileID);
    void SaveStickerSetZIPFile(const QString & mcrStickerSetName);
signals:
    void StickerSetReceived(const QString & mcrStickerSetName);
public:
    bool DoesStickerSetZIPFileExist(const QString & mcrStickerSetName) const;
    // Get (local) filename of the ZIP file with all stickers in that set
    QString GetStickerSetZIPFilename(const QString & mcrStickerSetName) const;

private:
    // File ID to sticker set name(s)
    QHash < QString, QSet < QString > > m_FileIDToStickerSetNames;

    // Remaining download queue for sticker set
    QHash < QString, QSet < QString > > m_StickerSetToRemainingFileIDs;
};

#endif
