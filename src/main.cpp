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

// main.cpp

// Project includes
#include "Application.h"
#include "Config.h"
#include "TelegramComms.h"
#include "MainWindow.h"

// System include
#include <signal.h>

// Terminate thread (Ctrl-C)
[[noreturn]] void terminated(int mStatus);



int main(int mNumParameters, char * mpParameter[])
{
    // Catch SIGTERM (Ctrl-C)
    signal(SIGINT, terminated);

    Application * app = Application::Instance(mNumParameters, mpParameter);

    // Open database
    TelegramComms * tc = TelegramComms::Instance();

    tc -> SetBotName(BOT_NAME);
    tc -> SetToken(BOT_TOKEN);

    tc -> SetDatabaseFile(BOT_DATABASE_FILE);
    tc -> OpenDatabase();

    MainWindow * window = MainWindow::Instance();
    window -> show();

    const int result = app -> exec();

    // Clean up
    delete window;
    delete app;

    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Terminate thread (Ctrl-C)
void terminated(int mStatus)
{
    exit(mStatus);
}
