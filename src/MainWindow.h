// MainWindow.h
// Class definition

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QLabel>
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>

// Forward declaration
class TelegramBot;



// Class definition
class MainWindow
    : public QMainWindow
{
    Q_OBJECT
    
    
    
    // ============================================================== Lifecycle
private:
    // Constructor
    MainWindow();
    
public:
    // Destructor
    virtual ~MainWindow();
    
    // Instanciator
    static MainWindow * Instance();

private:
    // Instance
    static MainWindow * m_Instance;
    
    
    
    // ==================================================================== GUI
private:
    // Initialize Widgets
    void InitWidgets();

    QTabWidget * m_TabWidget;
    QHash < qint64, QTextEdit * > m_ChatIDToTextEdit;
    QHash < qint64, QStringList > m_ChatIDToLogText;
    QLabel * m_Status;

private slots:
    // Update status message
    void UpdateStatus();

    // Gracefully shut down
    void GracefullyShutDown();
private:
    bool m_ShuttingDown;

    // Check if commands are being executed
    bool CommandsBeingExecuted() const;



    // ==================================================================== Bot
private:
    // Register commands of this bot
    void RegisterCommands();

private slots:
    // Any message
    void MessageReceived(const qint64 mcChatID, const qint64 mcMessageID);

    // Command
    void CommandReceived(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrCommand,
        const QString & mcrParameters);

    // Seaparate message for command
    void CommandSeparateMessageReceived(const qint64 mcUserID,
        const qint64 mcForwardedMessageID);
private:
    QHash < qint64, QString > m_Separate_UserIDToCommand;
    QHash < qint64, qint64 > m_Separate_UserIDToExpectedMessageID;
    QHash < qint64, qint64 > m_Separate_UserIDToChatID;


private:
    // == Command: /help
    void Command_Help(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrParameters);


    // == Command: /stickerset
    void Command_StickerSet(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrParameters);
    void SeparateCommand_StickerSet(const qint64 mcUserID,
        const qint64 mcChatID, const qint64 mcForwardedMessageID);
    void DownloadNewStickerSet(const qint64 mcUserID, const qint64 mcChatID,
        const QString & mcrStickerSetName);

    QHash < QString, QList < qint64 > > m_StickerSetNameToChatIDs;
    QHash < QString, QList < qint64 > > m_StickerSetNameToUserIDs;
    QHash < QString, QSet < qint64 > > m_StickerSetNameHasBeenSentToUserIDs;
private slots:
    void StickerSetInfoFailed(const QString & mcrStickerSetName);
    void StickerSetReceived(const QString & mcrStickerSetName);


private:
    // == Command /contactsheets
    void Command_ContactSheets(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrParameters);
    void Command_ContactSheets_AllSets(const qint64 mcChatID,
        const int mcRows, const int mcColumns);
    void Command_ContactSheets_SingleSet(const qint64 mcChatID,
        const QString & mcrStickerSetName, const int mcRows,
        const int mcColumns);
    void Command_ContactSheets_Render(const qint64 mcChatID,
        const int mcRows, const int mcColumns,
        const QStringList & mcrStickerNames);


    // == Command /set
    void Command_Set(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrParameters);


    // == Command /start
    void Command_Start(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrParameters);


    // == Unknown command
    void Command_UnknownCommand(const qint64 mcUserID, const qint64 mcChatID,
        const qint64 mcMessageID, const QString & mcrCommand);


private slots:
    // Sticker set info received
    void StickerSetInfoReceived(const QString & mcrStickerSetName);
};

#endif

