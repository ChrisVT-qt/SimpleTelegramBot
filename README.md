# SimpleTelegramBot
Simple Telegram bot in C++ that relies on Qt only and no other libraries.

General goal
- I wanted to write a Telegram Bot from scratch and understand how 
communication between my application and the Telegram server works.
- I wanted to understand this on a low level, i.e. I was not shooting for 
efficiency and did not use any Telegram communications libraries. I like a
low dependecy level of my code to other code anyway.

Current status
- Bot seems to work on macOS 15.3 Sequoia and Qt 6.9.0. 
- Not tested on other platforms, but given it only uses Qt, I see no reason why 
it would not.
- Bot can receive and send messages, download and upload files, and receive
commands.
- A few sample commands for downloading and handling sticker sets have been
implemented, and addition of new commands should be rather straight forward.
- The bot uses Telegram REST API. 
- Good news: You don't need access to a web server.
- Bad news: I had to slow down requests because Telegram does not like bots 
polling too frequently.
- Therefore, the bot will poll for new commands only every 5s. This probably 
can be optimized.
- Download requests are queued up and are processed one every second; this
works for a single user but will be too slow in a multi-user environment.

Future Goals 
- Additional bot commands
- Support of a larger portion of the Telegram Bot API, but only for a purpose,
that is, a bot command that needs it. This is not intended to be another
heavy-weight implementation of the full Telegram API.
- I'd like to speed things up, but this seems difficult with the current API.


# Getting things started...
In order to get started, you will need to put a few things into the 
configuration file config.h
- Set your bot name in BOT_NAME and its token in BOT_TOKEN.
- Set a directory where stuff gets stored (BOT_ROOT)
- Pick a database name where all messages are stored (BOT_DATABASE_FILE)
- You can adjust the remaining setting as you want.
