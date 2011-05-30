/*

 tinymudserver - an example MUD server

 Author:	Nick Gammon
					http://www.gammon.com.au/

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.

*/

// standard library includes ...

using namespace std;

#include "globals.h"

// global variables
bool	 bStopNow = false;			// when set, the MUD shuts down
time_t tLastMessage = 0;			// time we last sent a periodic message
int		iControl = NO_SOCKET;	// socket for accepting new connections

// list of all connected players
tPlayerList playerlist;
// map of all rooms
tRoomMap roommap;
// map of known commands(eg. look, quit, north etc.)
map<string, tHandler> commandmap;
// map of things to do for various connection states
map<tConnectionStates, tHandler> statemap;
// messages
map<string, string, ciLess> messagemap;
// directions
set<string, ciLess> directionset;
// bad player names
set<string, ciLess> badnameset;
// blocked IP addresses
set<string> blockedIP;

string TextFormatting( string sMessageText )
{
	sMessageText = FindAndReplace(sMessageText, "%r", "\r\n");
	sMessageText = FindAndReplace(sMessageText, "[/color]", "\e[0m");
	sMessageText = FindAndReplace(sMessageText, "[red]", "\e[0;31m");
	sMessageText = FindAndReplace(sMessageText, "[green]", "\e[0;32m");
	sMessageText = FindAndReplace(sMessageText, "[yellow]",	"\e[0;33m");
	sMessageText = FindAndReplace(sMessageText, "[blue]", "\e[0;34m");
	sMessageText = FindAndReplace(sMessageText, "[magenta]", "\e[0;35m");
	sMessageText = FindAndReplace(sMessageText, "[cyan]", "\e[0;36m");
	sMessageText = FindAndReplace(sMessageText, "[white]", "\e[0;37m");
	sMessageText = FindAndReplace(sMessageText, "[grey]", "\e[1;30m");
	sMessageText = FindAndReplace(sMessageText, "[RED]", "\e[1;31m");
	sMessageText = FindAndReplace(sMessageText, "[GREEN]", "\e[1;32m");
	sMessageText = FindAndReplace(sMessageText, "[YELLOW]", "\e[1;33m");
	sMessageText = FindAndReplace(sMessageText, "[BLUE]", "\e[1;34m");
	sMessageText = FindAndReplace(sMessageText, "[MAGENTA]", "\e[1;35m");
	sMessageText = FindAndReplace(sMessageText, "[CYAN]", "\e[1;36m");
	sMessageText = FindAndReplace(sMessageText, "[WHITE]", "\e[1;37m");
	
	return sMessageText;
}