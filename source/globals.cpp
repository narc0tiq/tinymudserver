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
#include <stdlib.h>
#include <stdio.h>

// global variables
bool	 bStopNow = false;			// when set, the MUD shuts down
time_t tLastMessage = 0;			// time we last sent a periodic message
int		iControl = NO_SOCKET;	// socket for accepting new connections

// list of all connected players
tPlayerList playerlist;
// map of all rooms
tRoomMap roommap;
// map of known commands(eg. look, quit, north etc.)
tCommandMap commandmap;
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
// positive responses
set<string, ciLess> posResp;
// negative responses
set<string, ciLess> negResp;

string screenLayout;

string TextFormatting( string sMessageText, class tPlayer *p )
{
	char tmp[1024];

	sMessageText = FindAndReplace(sMessageText, "\r", "\r\n");
	sMessageText = FindAndReplace(sMessageText, "[/color]",   "\e[0m");
	sMessageText = FindAndReplace(sMessageText, "[black]",    "\e[22;30m");
	sMessageText = FindAndReplace(sMessageText, "[red]",      "\e[22;31m");
	sMessageText = FindAndReplace(sMessageText, "[green]",    "\e[22;32m");
	sMessageText = FindAndReplace(sMessageText, "[yellow]",	  "\e[22;33m");
	sMessageText = FindAndReplace(sMessageText, "[blue]",     "\e[22;34m");
	sMessageText = FindAndReplace(sMessageText, "[magenta]",  "\e[22;35m");
	sMessageText = FindAndReplace(sMessageText, "[cyan]",     "\e[22;36m");
	sMessageText = FindAndReplace(sMessageText, "[white]",    "\e[22;37m");
	sMessageText = FindAndReplace(sMessageText, "[grey]",     "\e[1;30m");
	sMessageText = FindAndReplace(sMessageText, "[RED]",      "\e[1;31m");
	sMessageText = FindAndReplace(sMessageText, "[GREEN]",    "\e[1;32m");
	sMessageText = FindAndReplace(sMessageText, "[YELLOW]",   "\e[1;33m");
	sMessageText = FindAndReplace(sMessageText, "[BLUE]",     "\e[1;34m");
	sMessageText = FindAndReplace(sMessageText, "[MAGENTA]",  "\e[1;35m");
	sMessageText = FindAndReplace(sMessageText, "[CYAN]",     "\e[1;36m");
	sMessageText = FindAndReplace(sMessageText, "[WHITE]",    "\e[1;37m");
	sMessageText = FindAndReplace(sMessageText, "[bblack]",   "\e[40m");
	sMessageText = FindAndReplace(sMessageText, "[bred]",     "\e[41m");
	sMessageText = FindAndReplace(sMessageText, "[bgreen]",   "\e[42m");
	sMessageText = FindAndReplace(sMessageText, "[byellow]",  "\e[43m");
	sMessageText = FindAndReplace(sMessageText, "[bblue]",    "\e[44m");
	sMessageText = FindAndReplace(sMessageText, "[bmagenta]", "\e[45m");
	sMessageText = FindAndReplace(sMessageText, "[bcyan]",    "\e[46m");
	sMessageText = FindAndReplace(sMessageText, "[bwhite]",   "\e[47m");
	sMessageText = FindAndReplace(sMessageText, "[CLS]", "\e[2J\e[H");
	sMessageText = FindAndReplace(sMessageText, "[PlayerName]", p->playername);
	sMessageText = FindAndReplace(sMessageText, "[SCP]",  "\e[s");
	sMessageText = FindAndReplace(sMessageText, "[RCP]",  "\e[u");
	sMessageText = FindAndReplace(sMessageText, "[HOME]", "\e[H");
	sMessageText = FindAndReplace(sMessageText, "[EL]", "\e[2K");
	if( p->curhp < p->maxhp / 3 )
		sprintf( tmp, "\e[1;31m%d", p->curhp );
	else if( p->curhp < ( p->maxhp / 3 ) << 1 )
		sprintf( tmp, "\e[1;33m%d", p->curhp );
	else
		sprintf( tmp, "\e[1;32m%d", p->curhp );
	sMessageText = FindAndReplace(sMessageText, "[HP]", tmp );
	sprintf( tmp, "\e[1;37m%d", p->maxhp );
	sMessageText = FindAndReplace(sMessageText, "[MHP]", tmp );
	sMessageText = FindAndReplace(sMessageText, "[SKL]", "");
	
	return sMessageText;
}

int dieRoll( int sides )
{
	int retVal;

	retVal = ( rand() % sides ) + 1;

	return retVal;
}