// == Telegram communications

// Delays are introduced to avoid getting too much communication going to
// the server. If the server thinks you're polling, it will terminate your
// requests and put you on hold, so we want to avoid this.

// Delay for obtaining updates
#define POLL_DELAY 5*1000

// For file download, fetch a file every second
#define DOWNLOAD_DELAY 1*1000

// But configuration
#define BOT_NAME "your bot name"
#define BOT_TOKEN "your bot token"

// General bot directories
#define BOT_ROOT QString("your telegram bot roor directory")
#define BOT_DATABASE_DIR (BOT_ROOT + "Database/")
#define BOT_DATABASE_FILE (BOT_DATABASE_DIR + "TelegramBot.sql")

// This is where the bot puts files downloaded from the server
// (by file ID)
#define BOT_FILES (BOT_ROOT + "Files/")


// == User end configuration

// General base directory
#define USER_ROOT BOT_ROOT

// This is where we put our files for the user to get
// (human readable names)
#define USER_FILES (USER_ROOT + "Download/")

// This is where we put out sticker sets
// (human readable names)
#define USER_STICKERSETS (USER_FILES + "Sticker Sets/")
